#include "OpenGLWidget.h"
#include <QOpenGLContext>
#include <QMouseEvent>
#include <QColorDialog>
#include <sstream>
#include <string>
#include <iostream>
#include <glm/glm.hpp>

#include <map>

#include "MainWindow.h"
#include <neurolots/nlrender/Config.h>

#include "prefr/ColorOperationPrototype.h"
#include "prefr/ColorEmissionNode.h"
#include "prefr/CompositeColorEmitter.h"
#include "prefr/CompositeColorUpdater.h"

using namespace visimpl;

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
  , _paintNeurons( paintNeurons_ )
  , _frameCount( 0 )
  , _mouseX( 0 )
  , _mouseY( 0 )
  , _rotation( false )
  , _translation( false )
  , _idleUpdate( true )
  , _paint( false )
  , _currentClearColor( 20, 20, 20, 0 )
  , _simulationType( TSimulationType::TUndefined )
  , _player( nullptr )
  , _firstFrame( true )
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




void OpenGLWidget::loadData( const std::string& fileName,
                             const TDataFileType fileType,
                             const std::string& /*target*/,
                             const std::string& /*report*/)
{

  makeCurrent( );

  switch( fileType )
  {
  case TDataFileType::tBlueConfig:

//    _blueConfig = new brion::BlueConfig( fileName );
////    std::cout << _blueConfig->getCircuitSource( ).getPath( ) << std::endl;
//    _circuit = new brain::Circuit( *_blueConfig );
//    std::cout << _blueConfig->getSpikeSource( ).getPath( ) << std::endl;
//    _spikeReport = new brion::SpikeReport( _blueConfig->getSpikeSource( ),
//                                            brion::AccessMode::MODE_READ );

    _deltaTime = 0.5f;
    _player = new SpikesPlayer( fileName, true );
    _player->deltaTime( _deltaTime );
    _simulationType = TSimulationType::TSpikes;
//    _player->loop( true );
//    _player->Play( _deltaTime );

    createParticleSystem( );

    break;

  default:
    throw std::runtime_error( "Data file type not supported" );

  }

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

void OpenGLWidget::configureSimulation( void )
{
  if( !_player || !_player->isPlaying( ))
    return;

  _player->Frame( );

  if( _simulationType == TSimulationType::TSpikes )
  {
    SpikesCRange spikes =
        dynamic_cast< visimpl::SpikesPlayer* >( _player )->spikesNow( );

    for( SpikesCIter spike = spikes.first; spike != spikes.second; spike++)
    {
      auto res = gidNodesMap.find( (*spike).second );
      if( res != gidNodesMap.end( ))
      {
        ( *res ).second->killParticles( );
      }
    }
  }

}

void OpenGLWidget::createNeuronsCollection( void )
{
  makeCurrent( );
  neurolots::nlrender::Config::init( );
//  _neuronsCollection = new neurolots::NeuronsCollection( _camera );
}

void OpenGLWidget::createParticleSystem( void )
{
  makeCurrent( );
  prefr::Config::init( );

  unsigned int maxParticles = _player->gids( ).size( );
  unsigned int maxEmitters = maxParticles;

  _ps = new prefr::ParticleSystem( maxParticles, maxParticles, true );
  _ps->renderDeadParticles = true;

  const brion::Vector3fs& positions = _player->positions( );

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
  vertPath.append( "/GL-vert.glsl" );
  fragPath.append( "/GL-frag.glsl" );
  _particlesShader = new CShader( false, false,
                                 vertPath.c_str( ) ,
                                 fragPath.c_str( ));

  std::cout << "Loading shaders: " << std::endl;
  std::cout << "- Vertex: " << vertPath.c_str( ) << std::endl;
  std::cout << "- Fragments: " << fragPath.c_str( ) << std::endl;

  _maxLife = 20.0f;

  prefr::ColorOperationPrototype* prototype =
    new prefr::ColorOperationPrototype(
      _maxLife, _maxLife,
      prefr::ParticleCollection( _ps->particles, 0, maxParticles));

//  prototype->color.Insert( 0.0f, ( glm::vec4(0.2, 0.2, 0.2, 0.05)));
//  prototype->color.Insert( 0.1f, ( glm::vec4(0, 1, 1, 0.5 )));
//  prototype->color.Insert( 1.0f, ( glm::vec4(0.2, 0.2, 0.2, 0.05 )));

  prototype->color.Insert( 0.0f, ( glm::vec4(0.1, 0.1, 0.3, 0.05)));
  prototype->color.Insert( 0.1f, ( glm::vec4(1, 0, 0, 0.2 )));
  prototype->color.Insert( 0.7f, ( glm::vec4(1, 0.5, 0, 0.2 )));
  prototype->color.Insert( 1.0f, ( glm::vec4(0, 0, 0.3, 0.05 )));

  prototype->velocity.Insert( 0.0f, 0.0f );

  prototype->size.Insert( 0.0f, 30.0f );
  prototype->size.Insert( 0.0f, 10.0f );

  _ps->AddPrototype( prototype );

  prefr::ColorEmissionNode* emissionNode;

  int partPerEmitter = 1;

  std::cout << "Creating " << maxEmitters << " emitters with "
            << partPerEmitter << std::endl;

  unsigned int i = 0;
  glm::vec3 cameraPivot;
  brion::GIDSetCIter gid = _player->gids( ).begin();
  for ( auto brionPos : positions )
  {

//    nsol::Matrix4_4f m = it.second->transform( );
//    vmml::Vector3f t = it.second->morphology( )->soma( )->center( );
//    glm::vec3 position ( t.x( ), t.y( ), t.z( ));
//    glm::vec3 position ( m.at( 0, 3 ), m.at( 1, 3 ), m.at( 2, 3 ));

//    std::cout << position.x << ", "
//              << position.y << ", "
//              << position.z << std::endl;



    glm::vec3 position( brionPos.x( ), brionPos.y( ), brionPos.z( ));

    cameraPivot += position;

    emissionNode =
        new prefr::ColorEmissionNode( prefr::ParticleCollection(
                                      _ps->particles,
                                      i * partPerEmitter,
                                      i * partPerEmitter + partPerEmitter ),
                                      position,
                                      glm::vec4( 0, 0, 0, 0 ),
                                      true );

    _ps->AddEmissionNode( emissionNode );
//    emissionNode->active = false;
//    emissionNode->killParticlesIfInactive = true;
    emissionNode->maxEmissionCycles = 1;

    gidNodesMap.insert( std::pair< uint32_t,
                        prefr::ColorEmissionNode* >(( *gid ), emissionNode ));

    i++;
    gid++;
  }

  cameraPivot /= i;

  _camera->Pivot( Eigen::Vector3f( cameraPivot.x,
                                         cameraPivot.y,
                                         cameraPivot.z ));

  prefr::CompositeColorEmitter* emitter =
    new prefr::CompositeColorEmitter( *_ps->particles, 1.f, true );
  _ps->AddEmitter( emitter );

  std::cout << "Created emitter" << std::endl;
  prefr::CompositeColorUpdater* updater =
    new prefr::CompositeColorUpdater( *_ps->particles );
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

  resetParticles( );

}

void OpenGLWidget::resetParticles( void )
{
  for( auto node = _ps->emissionNodes->begin( );
         node != _ps->emissionNodes->end( );
         node++ )
  {
    dynamic_cast< prefr::ColorEmissionNode* >( *node )->killParticles( false );
    (*node)->Restart( );
  }
//  _ps->UpdateUnified( _deltaTime );
}

void OpenGLWidget::paintParticles( void )
{
  if( !_ps )
    return;

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


  if( _player->isPlaying( ) || _firstFrame )
  {

    _ps->UpdateUnified( _deltaTime );
    _firstFrame = false;
  }

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

//    if ( _neuronsCollection && _paintNeurons )
//      _neuronsCollection->Paint( );

    if ( _ps )
    {
      configureSimulation( );
      paintParticles( );
    }
    glUseProgram( 0 );
    glFlush( );

  }

  #define FRAMES_PAINTED_TO_MEASURE_FPS 10
  if ( _frameCount == FRAMES_PAINTED_TO_MEASURE_FPS )
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

    _frameCount = 0;
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

//  case Qt::Key_W:
//    _neuronsCollection->AddLod( 1.0f );
//    update( );
//    break;
//
//  case Qt::Key_S:
//    _neuronsCollection->AddLod( -1.0f );
//    update( );
//    break;
//
//  case Qt::Key_E:
//    _neuronsCollection->AddTng( 0.1f );
//    update( );
//    break;
//
//  case Qt::Key_D:
//    _neuronsCollection->AddTng( -0.1f );
//    update( );
//    break;
//
//  case Qt::Key_R:
//    _neuronsCollection->AddMaxDist( 1 );
//    update( );
//    break;
//
//  case Qt::Key_F:
//    _neuronsCollection->AddMaxDist( -1 );
//    update( );
//    break;
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


visimpl::SimulationPlayer* OpenGLWidget::player( )
{
  return _player;
}

void OpenGLWidget::Play( void )
{
  if( _player )
  {
    _player->Play( _player->deltaTime( ));
  }
}

void OpenGLWidget::Pause( void )
{
  if( _player )
  {
    _player->Pause( );
  }
}

void OpenGLWidget::PlayPause( void )
{

  if( _player )
  {
    if( !_player->isPlaying( ))
      _player->Play( _player->deltaTime( ));
    else
      _player->Pause( );
  }
}

void OpenGLWidget::Stop( void )
{
  if( _player )
  {
    _player->Stop( );
    resetParticles( );
    _firstFrame = true;
  }
}

void OpenGLWidget::Repeat( bool repeat )
{
  if( _player )
  {
    _player->loop( repeat );
  }
}

void OpenGLWidget::PlayAt( unsigned int )
{

}

void OpenGLWidget::Restart( void )
{
  if( _player )
  {
    bool playing = _player->isPlaying( );
    _player->Stop( );
    if( playing )
      _player->Play( _player->deltaTime( ));
    resetParticles( );
    _firstFrame = true;
  }
}

void OpenGLWidget::GoToEnd( void )
{

}

