#include "OpenGLWidget.h"
#include <QOpenGLContext>
#include <QMouseEvent>
#include <QColorDialog>
#include <sstream>
#include <string>
#include <iostream>
#include <glm/glm.hpp>

#include "MainWindow.h"
#include <neurolots/nlrender/Config.h>

OpenGLWidget::OpenGLWidget( QWidget* parent_,
                            Qt::WindowFlags windowsFlags_,
                            bool paintNeurons_,
                            const std::string&
#ifdef NEUROLOTS_USE_ZEQ
                            zeqUri
#endif
  )
  : QOpenGLWidget( parent_, windowsFlags_ )
  , _fpsLabel( this )
  , _showFps( false )
  , _wireframe( false )
  , _neuronsCollection( nullptr )
  , _paintNeurons( paintNeurons_ )
  , _frameCount( 0 )
  , _mouseX( 0 )
  , _mouseY( 0 )
  , _rotation( false )
  , _translation( false )
  , _idleUpdate( true )
  , _paint( false )
  , _currentClearColor( 20, 20, 20, 0 )
{
#ifdef NEUROLOTS_USE_ZEQ
  if ( zeqUri != "" )
  {
    std::cout << zeqUri << std::endl;
    _camera = new neurolots::Camera( zeqUri );
  }
  else
#endif
    _camera = new neurolots::Camera( );

  _fpsLabel.setStyleSheet(
    "QLabel { background-color : #333;"
    "color : white;"
    "padding: 3px;"
    "margin: 10px;"
    " border-radius: 10px;}" );

  // This is needed to get key evends
  this->setFocusPolicy( Qt::WheelFocus );

}


OpenGLWidget::~OpenGLWidget( void )
{
  delete _camera;
}

void OpenGLWidget::createNeuronsCollection( void )
{
  makeCurrent( );
  neurolots::nlrender::Config::init( );
  _neuronsCollection = new neurolots::NeuronsCollection( _camera );
}

void OpenGLWidget::createParticleSystem( void )
{
  makeCurrent( );
  prefr::Config::init( );

  unsigned int maxParticles = 10000;
  unsigned int maxEmitters = 1;

  _ps = new prefr::ParticleSystem(10, maxParticles, true);

  std::string prefrShadersPath;

  if ( getenv( "PREFR_SHADERS_PATH" ) == nullptr )
  {
    std::cerr << "Environment Variable PREFR_SHADERS_PATH not defined"
              << std::endl;
    exit(-1);
  }
  else
    prefrShadersPath = std::string( getenv( "PREFR_SHADERS_PATH" ));

  std::string vertPath, fragPath;
  fragPath = vertPath = std::string( prefrShadersPath );
  vertPath.append( "/shd/GL-vert.glsl" );
  fragPath.append( "/shd/GL-frag.glsl" );
  _particlesShader = new CShader( false, false,
                                 vertPath.c_str( ) ,
                                 fragPath.c_str( ));

  std::cout << "Loading shaders: " << std::endl;
  std::cout << "- Vertex: " << vertPath.c_str( ) << std::endl;
  std::cout << "- Fragments: " << fragPath.c_str( ) << std::endl;

  prefr::ParticlePrototype* prototype =
    new prefr::ParticlePrototype(
      3.0f, 5.0f,
      prefr::ParticleCollection( _ps->particles, 0, maxParticles));

  prototype->color.Insert( 0.0f, ( glm::vec4(0, 0, 1, 0.2 )));
  prototype->color.Insert( 0.65f, ( glm::vec4(1, 1, 0, 0.2 )));
  prototype->color.Insert( 0.35f, ( glm::vec4(0, 1, 0, 0.2 )));
  prototype->color.Insert( 1.0f, ( glm::vec4(0, 0.5, 0.5, 0 )));
  prototype->color.Remove( 3 );
  prototype->color.Insert( 1.0f, ( glm::vec4(0, 0.5, 0.5, 0 )));
  prototype->color.Insert( 1.0f, ( glm::vec4(0, 0.5, 0.5, .5 )));

  prototype->velocity.Insert( 0.0f, 3.0f );
  prototype->velocity.Insert( 1.0f, 5.0f );

  prototype->size.Insert( 0.0f, 1.0f );

  _ps->AddPrototype( prototype );

  prefr::PointEmissionNode* emissionNode;

  int particlesPerEmitter = maxParticles / maxEmitters;

  std::cout << "Creating " << maxEmitters << " emitters with "
            << particlesPerEmitter << std::endl;

  glm::vec3 origin ( _camera->Pivot( )[0],
                     _camera->Pivot( )[1],
                     _camera->Pivot( )[2]
                    );

  std::cout << "Using center: " << std::endl;
  std::cout << origin.x << ", "
            << origin.y << ", "
            << origin.z << std::endl;

  for ( unsigned int i = 0; i < maxEmitters; i++ )
  {
    std::cout << "Creating emission node " << i << " from "
              << i * particlesPerEmitter << " to "
              << i * particlesPerEmitter + particlesPerEmitter << std::endl;
    emissionNode =
        new prefr::PointEmissionNode(
          prefr::ParticleCollection(
            _ps->particles,
            i * particlesPerEmitter,
            i * particlesPerEmitter + particlesPerEmitter ),
          origin + glm::vec3(i * 10, 0, 0));
    _ps->AddEmissionNode(emissionNode);
  }

  prefr::ParticleEmitter* emitter =
    new prefr::ParticleEmitter( *_ps->particles, 0.3f, true );
  _ps->AddEmitter( emitter );

  std::cout << "Created emitter" << std::endl;
  prefr::ParticleUpdater* updater =
    new prefr::ParticleUpdater( *_ps->particles );
  std::cout << "Created updater" << std::endl;

  prefr::ParticleSorter* sorter;

  #if (PREFR_USE_CUDA)
  std::cout << "CUDA sorter" << std::endl;
  sorter = new prefr::ThrustParticleSorter( *_ps->particles );
  #else
  sorter = new prefr::ParticleSorter( *_ps->particles );
  #endif

  std::cout << "Created sorter" << std::endl;

  prefr::GLDefaultParticleRenderer* renderer =
    new prefr::GLDefaultParticleRenderer( *_ps->particles );

  std::cout << "Created systems" << std::endl;

  _ps->AddUpdater( updater );
  _ps->SetSorter( sorter );
  _ps->SetRenderer( renderer );

  _ps->Start();


}


void OpenGLWidget::loadData( const std::string& fileName,
                             const TDataFileType fileType,
                             const std::string& target )
{

  makeCurrent( );

  switch( fileType )
  {
  case TDataFileType::BlueConfig:
    _neuronsCollection->loadBlueConfig( fileName, target );
    break;

  case TDataFileType::SWC:
    _neuronsCollection->loadSwc( fileName );
    break;

  case TDataFileType::NsolScene:
    _neuronsCollection->loadScene( fileName );
    break;

  default:
    throw std::runtime_error( "Data file type not supported" );

  }

  createParticleSystem( );

  this->_paint = true;
  update( );

  return;
}


void OpenGLWidget::initializeGL( void )
{
  initializeOpenGLFunctions( );

  glEnable( GL_DEPTH_TEST );
  glClearColor( float( _currentClearColor.red( )) / 255.0f,
                float( _currentClearColor.green( )) / 255.0f,
                float( _currentClearColor.blue( )) / 255.0f,
                float( _currentClearColor.alpha( )) / 255.0f );
  glPolygonMode( GL_FRONT_AND_BACK , GL_FILL );
  glEnable( GL_CULL_FACE );

  glLineWidth( 1.5 );

  _then = std::chrono::system_clock::now( );


  QOpenGLWidget::initializeGL( );

}

void OpenGLWidget::paintParticles( void )
{
  glDepthMask(GL_FALSE);
  glEnable(GL_BLEND);

  glEnable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA);

  glFrontFace(GL_CCW);

  _particlesShader->activate();
      // unsigned int shader;
      // shader = _particlesShader->getID();
  unsigned int shader;
  shader = _particlesShader->getID();

  unsigned int uModelViewProjM, cameraUp, cameraRight;

  uModelViewProjM = glGetUniformLocation( shader, "modelViewProjM" );
  glUniformMatrix4fv( uModelViewProjM, 1, GL_FALSE,
                     _camera->ViewProjectionMatrix( ));

  cameraUp = glGetUniformLocation( shader, "cameraUp" );
  cameraRight = glGetUniformLocation( shader, "cameraRight" );

  float* viewM = _camera->ViewMatrix( );

  glUniform3f( cameraUp, viewM[1], viewM[5], viewM[9] );
  glUniform3f( cameraRight, viewM[0], viewM[4], viewM[8] );


  _ps->UpdateUnified( 0.1f );
  _ps->UpdateCameraDistances( glm::vec3( _camera->Position()[0],
                                         _camera->Position()[1],
                                         _camera->Position()[2]));
  _ps->UpdateRender( );

  _ps->Render( );

}


void OpenGLWidget::paintGL( void )
{

  _frameCount++;
  glDepthMask(GL_TRUE);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glDisable(GL_BLEND);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);

  if ( _paint )
  {
    _camera->Anim( );

    if ( _neuronsCollection && _paintNeurons )
      _neuronsCollection->Paint( );

    if ( _ps )
      paintParticles( );

    glUseProgram( 0 );
    glFlush( );

  }

  #define FRAMES_PAINTED_TO_MEASURE_FPS 10
  if ( _frameCount % FRAMES_PAINTED_TO_MEASURE_FPS  == 0 )
  {

    std::chrono::time_point< std::chrono::system_clock > now =
      std::chrono::system_clock::now( );

    auto duration =
      std::chrono::duration_cast<std::chrono::milliseconds>( now - _then );
    _then = now;

    MainWindow* mainWindow = dynamic_cast< MainWindow* >( parent( ));
    if ( mainWindow )
    {

      unsigned int ellapsedMiliseconds = duration.count( );

      unsigned int fps = roundf( 1000.0f *
                                 float( FRAMES_PAINTED_TO_MEASURE_FPS ) /
                                 float( ellapsedMiliseconds ));

      // mainWindow->showStatusBarMessage(
      //   QString::number( fps ) + QString( " FPS" ));
      if ( _showFps )
      {
        _fpsLabel.setVisible( true );
        _fpsLabel.setText( QString::number( fps ) + QString( " FPS" ));
        _fpsLabel.adjustSize( );
      }
      else
        _fpsLabel.setVisible( false );
    }

  }

  if ( _idleUpdate )
  {
    // std::cout << _frameCount << std::endl;
    update( );
  }
  else
  {
    _fpsLabel.setVisible( false );
  }

}

void OpenGLWidget::resizeGL( int w , int h )
{
  _camera->Ratio((( double ) w ) / h );
  glViewport( 0, 0, w, h );


}


void OpenGLWidget::mousePressEvent( QMouseEvent* event_ )
{

  if ( event_->button( ) == Qt::LeftButton )
  {
    _rotation = true;
    _mouseX = event_->x( );
    _mouseY = event_->y( );
  }

  if ( event_->button( ) ==  Qt::MidButton )
  {
    _translation = true;
    _mouseX = event_->x( );
    _mouseY = event_->y( );
  }

  update( );

}

void OpenGLWidget::mouseReleaseEvent( QMouseEvent* event_ )
{
  if ( event_->button( ) == Qt::LeftButton)
  {
    _rotation = false;
  }

  if ( event_->button( ) ==  Qt::MidButton )
  {
    _translation = false;
  }

  update( );

}

void OpenGLWidget::mouseMoveEvent( QMouseEvent* event_ )
{
  if( _rotation )
  {
    _camera->LocalRotation( -( _mouseX - event_->x( )) * 0.01,
                          ( _mouseY - event_->y( )) * 0.01 );
    _mouseX = event_->x( );
    _mouseY = event_->y( );
  }
  if( _translation )
  {
    _mouseX = event_->x( );
    _mouseY = event_->y( );
  }

  this->update( );
}


void OpenGLWidget::wheelEvent( QWheelEvent* event_ )
{

  int delta = event_->angleDelta( ).y( );

  if ( delta > 0 )
    _camera->Radius( _camera->Radius( ) / 1.1f );
  else
    _camera->Radius( _camera->Radius( ) * 1.1f );

  update( );

}



void OpenGLWidget::keyPressEvent( QKeyEvent* event_ )
{
  makeCurrent( );

  switch ( event_->key( ))
  {
  case Qt::Key_C:
    _camera->Pivot( Eigen::Vector3f( 0.0f, 0.0f, 0.0f ));
    _camera->Radius( 1000.0f );
    _camera->Rotation( 0.0f, 0.0f );
    update( );
    break;

  case Qt::Key_W:
    _neuronsCollection->AddLod( 1.0f );
    update( );
    break;

  case Qt::Key_S:
    _neuronsCollection->AddLod( -1.0f );
    update( );
    break;

  case Qt::Key_E:
    _neuronsCollection->AddTng( 0.1f );
    update( );
    break;

  case Qt::Key_D:
    _neuronsCollection->AddTng( -0.1f );
    update( );
    break;

  case Qt::Key_R:
    _neuronsCollection->AddMaxDist( 1 );
    update( );
    break;

  case Qt::Key_F:
    _neuronsCollection->AddMaxDist( -1 );
    update( );
    break;
  }
}


void OpenGLWidget::changeClearColor( void )
{
  QColor color =
    QColorDialog::getColor( _currentClearColor, parentWidget( ),
                            "Choose new background color",
                            QColorDialog::DontUseNativeDialog);

  if ( color.isValid( ))
  {
    _currentClearColor = color;

    makeCurrent( );
    glClearColor( float( _currentClearColor.red( )) / 255.0f,
                  float( _currentClearColor.green( )) / 255.0f,
                  float( _currentClearColor.blue( )) / 255.0f,
                  float( _currentClearColor.alpha( )) / 255.0f );
    update( );
  }
}


void OpenGLWidget::toggleUpdateOnIdle( void )
{
  _idleUpdate = !_idleUpdate;
  if ( _idleUpdate )
    update( );
}

void OpenGLWidget::toggleShowFPS( void )
{
  _showFps = !_showFps;
  if ( _idleUpdate )
    update( );
}

void OpenGLWidget::toggleWireframe( void )
{
  makeCurrent( );
  _wireframe = !_wireframe;

  if ( _wireframe )
  {
    glEnable( GL_POLYGON_OFFSET_LINE );
    glPolygonOffset( -1, -1 );
    glLineWidth( 1.5 );
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
  }
  else
  {
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    glDisable( GL_POLYGON_OFFSET_LINE );
  }

  update( );
}

void OpenGLWidget::togglePaintNeurons( void )
{
  _paintNeurons = !_paintNeurons;
  update( );
}
