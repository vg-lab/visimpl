/*
 * @file  OpenGLWidget.cpp
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */

#include "OpenGLWidget.h"
#include <QOpenGLContext>
#include <QMouseEvent>
#include <QColorDialog>
#include <QShortcut>

#include <sstream>
#include <string>
#include <iostream>
#include <glm/glm.hpp>

#include <map>

#include "MainWindow.h"

#include "prefr/PrefrShaders.h"
#include "prefr/ColorSource.h"

#include "prefr/CompositeColorUpdater.h"

#include "prefr/ValuedSource.h"
#include "prefr/ValuedUpdater.h"

using namespace visimpl;

OpenGLWidget::OpenGLWidget( QWidget* parent_,
                            Qt::WindowFlags windowsFlags_,
                            bool paintNeurons_,
                            const std::string&
#ifdef VISIMPL_USE_ZEROEQ
                            zeqUri
#endif
  )
: QOpenGLWidget( parent_, windowsFlags_ )
#ifdef VISIMPL_USE_ZEROEQ
, _zeqUri( zeqUri )
#endif
, _fpsLabel( this )
, _showFps( false )
, _wireframe( false )
, _camera( nullptr )
, _lastCameraPosition( 0, 0, 0)
, _focusOnSelection( paintNeurons_ )
, _pendingSelection( false )
, _frameCount( 0 )
, _mouseX( 0 )
, _mouseY( 0 )
, _rotation( false )
, _translation( false )
, _idleUpdate( true )
, _paint( false )
, _currentClearColor( 20, 20, 20, 0 )
, _particlesShader( nullptr )
, _particleSystem( nullptr )
, _simulationType( simil::TSimulationType::TSimNetwork )
, _player( nullptr )
, _firstFrame( true )
, _prototype( nullptr )
, _offPrototype( nullptr )
, _elapsedTimeRenderAcc( 0.0f )
, _elapsedTimeSliderAcc( 0.0f )
, _elapsedTimeSimAcc( 0.0f )
, _alphaBlendingAccumulative( false )
, _showSelection( false )
, _resetParticles( false )
{
#ifdef VISIMPL_USE_ZEROEQ
  if ( zeqUri != "" )
    _camera = new reto::Camera( zeqUri );
  else
#endif
  _camera = new reto::Camera( );

  _lastCameraPosition = glm::vec3( 0, 0, 0 );

  _fpsLabel.setStyleSheet(
    "QLabel { background-color : #333;"
    "color : white;"
    "padding: 3px;"
    "margin: 10px;"
    " border-radius: 10px;}" );

  // This is needed to get key evends
  this->setFocusPolicy( Qt::WheelFocus );

  _maxFPS = 60.0f;
  _renderPeriod = 1.0f / _maxFPS;

//  _playbackSpeed = 5.0f;
  _playbackSpeed = 1.f;
  _renderSpeed = 1.f;

  new QShortcut( QKeySequence( Qt::CTRL + Qt::Key_Minus ),
                 this, SLOT( reducePlaybackSpeed( ) ));

  new QShortcut( QKeySequence( Qt::CTRL + Qt::Key_Plus ),
                   this, SLOT( increasePlaybackSpeed( ) ));
}


OpenGLWidget::~OpenGLWidget( void )
{
  delete _camera;

  if( _particlesShader )
    delete _particlesShader;

  if( _particleSystem )
    delete _particleSystem;

  if( _player )
    delete _player;

}




void OpenGLWidget::loadData( const std::string& fileName,
                             const simil::TDataType fileType,
                             simil::TSimulationType simulationType,
                             const std::string& report)
{

  makeCurrent( );

  _simulationType = simulationType;

  switch( _simulationType )
  {
    case simil::TSimSpikes:
    {
      _deltaTime = 0.5f;
//        _player = new SpikesPlayer( fileName, true );
      simil::SpikesPlayer* spPlayer = new simil::SpikesPlayer( );
      spPlayer->LoadData( fileType, fileName, report );
      _player = spPlayer;
      _player->deltaTime( _deltaTime );

      break;
    }
    case simil::TSimVoltages:
    {
      _player = new simil::VoltagesPlayer( fileName, report, true);
      _deltaTime = _player->deltaTime( );
      break;
    }
    default:
      VISIMPL_THROW("Cannot load an undefined simulation type.");

  }

  float scale = 1.0f;
  if( fileType == simil::THDF5 )
  {
    scale = 500.f;
  }

  switch( fileType )
  {
    case simil::TBlueConfig:
      _playbackSpeed = 5.0f;
      changeSimulationDecayValue( 5.0f );
      break;
    case simil::THDF5:
      _playbackSpeed = 0.05f;
      changeSimulationDecayValue( 1.0f );
      break;

    default:
      break;
  }

  createParticleSystem( scale );
  _player->connectZeq( _zeqUri );

  this->_paint = true;
  update( );

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
  _lastFrame = std::chrono::system_clock::now( );


  QOpenGLWidget::initializeGL( );

}

void OpenGLWidget::configureSimulation( void )
{
  if( !_player || !_player->isPlaying( ) || !_particleSystem->run( ))
    return;

  _player->Frame( );

  switch( _simulationType )
  {
    case simil::TSimSpikes:
    {
      simil::SpikesCRange spikes =
          dynamic_cast< simil::SpikesPlayer* >( _player )->spikesNow( );

      unsigned int count = 0;
      unsigned int triggeredSpikes = 0;
      for( simil::SpikesCIter spike = spikes.first;
           spike != spikes.second; ++spike)
      {
        if( _selectedGIDs.find( ( *spike ).second ) != _selectedGIDs.end( )
            || _selectedGIDs.size( ) == 0 || !_showSelection )
        {
          auto res = gidNodesMap.find( (*spike).second );
          if( res != gidNodesMap.end( ))
          {
            dynamic_cast< prefr::Cluster* >(( *res ).second )->killParticles( );
            ++triggeredSpikes;
          }
          count += 1;
        }
      }
      break;
    }
    case simil::TSimVoltages:
    {
      simil::VoltagesPlayer* vplayer =
          dynamic_cast< simil::VoltagesPlayer* >(_player);

      float normFactor = vplayer->getNormVoltageFactor( );

      unsigned int counter = 0;
      for( auto gid : vplayer->gids( ))
      {
        auto res = gidNodesMap.find( gid );
        if( res == gidNodesMap.end( ))
        {
          std::cerr << "Err: Node not found " << gid << std::endl;
          continue;
        }

        float volt = vplayer->getVoltage( gid );
        float relVal = volt + std::abs( vplayer->minVoltage( ));
        relVal *= normFactor;

        dynamic_cast< prefr::ValuedSource* >( res->second )->
            particlesLife( relVal );

        counter++;
      }

      break;
    }
    default:
      break;
  }

}

void OpenGLWidget::createParticleSystem( float scale )
{
  makeCurrent( );
  prefr::Config::init( );

  unsigned int maxParticles = _player->gids( ).size( );
  unsigned int maxEmitters = maxParticles;

  _particleSystem = new prefr::ParticleSystem( maxParticles );
  _particleSystem->renderDeadParticles( true );

  //TODO
  _particleSystem->parallel( false );

  const brion::Vector3fs& positions = _player->positions( );

  _particlesShader = new reto::ShaderProgram( );
  _particlesShader->loadVertexShaderFromText( prefr::prefrVertexShader );
  _particlesShader->loadFragmentShaderFromText( prefr::prefrFragmentShader );
  _particlesShader->create( );
  _particlesShader->link( );

  _offPrototype = new prefr::ColorOperationModel( _maxLife, _maxLife );

  _offPrototype->color.Insert( 0.0f, ( glm::vec4(0.1f, 0.1f, 0.1f, 0.5f)));

  _offPrototype->velocity.Insert( 0.0f, 0.0f );

  _offPrototype->size.Insert( 1.0f, 10.0f );

  _particleSystem->addModel( _offPrototype );


  _prototype = new prefr::ColorOperationModel( _maxLife, _maxLife );

  _particleSystem->addModel( _prototype );


  prefr::Updater* updater;

  switch( _simulationType )
  {
   case simil::TSimSpikes:

     updater = new prefr::CompositeColorUpdater( );
     std::cout << "Created Spikes Updater" << std::endl;

     _prototype->color.Insert( 0.0f, ( glm::vec4(0.f, 1.f, 0.f, 0.05)));
     _prototype->color.Insert( 0.35f, ( glm::vec4(1, 0, 0, 0.2 )));
     _prototype->color.Insert( 0.7f, ( glm::vec4(1.f, 1.f, 0, 0.2 )));
     _prototype->color.Insert( 1.0f, ( glm::vec4(0, 0, 1.f, 0.2 )));

     _prototype->velocity.Insert( 0.0f, 0.0f );

     _prototype->size.Insert( 0.0f, 20.0f );
     _prototype->size.Insert( 1.0f, 10.0f );

     break;
   case simil::TSimVoltages:

     updater = new prefr::ValuedUpdater( );
     std::cout << "Created Voltages Updater" << std::endl;

     _prototype->color.Insert( 0.0f, ( glm::vec4(0.1, 0.1, 0.3, 0.05)));
     _prototype->color.Insert( 0.25f, ( glm::vec4(0.2, 0.2, 0.3, 0.07 )));
     _prototype->color.Insert( 0.5f, ( glm::vec4(0, 0.5, 0, 0.10 )));
     _prototype->color.Insert( 0.75f, ( glm::vec4(0.3, 0.3, 0.1, 0.15 )));
     _prototype->color.Insert( 1.0f, ( glm::vec4(1, 0.1, 0.1, 0.2 )));

     _prototype->velocity.Insert( 0.0f, 0.0f );

     _prototype->size.Insert( 0.0f, 5.0f );
     _prototype->size.Insert( 1.0f, 20.0f );


     break;
   default:
     VISIMPL_THROW("Simulation type undefined.");
     break;
  }


  prefr::Cluster* cluster;
  prefr::Source* source;
  prefr::PointSampler* sampler = new prefr::PointSampler( );

  int partPerEmitter = 1;

  std::cout << "Creating " << maxEmitters << " emitters with "
            << partPerEmitter << std::endl;

  unsigned int start;

  unsigned int i = 0;
  glm::vec3 cameraPivot;
  brion::GIDSetCIter gid = _player->gids( ).begin();

  ;
  for ( auto brionPos : positions )
  {
    cluster = new prefr::Cluster( );
    cluster->model( _prototype );
    cluster->updater( updater );
    cluster->active( true );


    glm::vec3 position( brionPos.x( ), brionPos.y( ), brionPos.z( ));

    if( scale != 1.0f )
      position *= scale;

    cameraPivot += position;

    start = i * partPerEmitter;

    switch( _simulationType )
    {
    case simil::TSimSpikes:
      source = new prefr::ColorSource( -1.f, position,
                                       glm::vec4( 0, 0, 0, 0 ),
                                       true );
      break;
    case simil::TSimVoltages:
      source = new prefr::ValuedSource( -1.0f, position,
                                        glm::vec4( 0, 0, 0, 0 ),
                                        true );
      break;
    default:
      source = nullptr;
      VISIMPL_THROW("Node type undefined");
      break;
    }

    source->sampler( sampler );
    cluster->source( source );

    _particleSystem->addSource( source );
    _particleSystem->addCluster( cluster, start, partPerEmitter );

    cluster->inactiveKillParticles( true );
    source->maxEmissionCycles( 1 );

    gidNodesMap.insert( std::make_pair(( *gid ), cluster ));
    nodesGIDMap.insert( std::make_pair( cluster, ( *gid )));

    i++;
    gid++;

//    if( i > 50 ) break;
  }

  cameraPivot /= i;

  _camera->pivot( Eigen::Vector3f( cameraPivot.x,
                                   cameraPivot.y,
                                   cameraPivot.z ));

  prefr::Sorter* sorter = new prefr::Sorter( );

  std::cout << "Created sorter" << std::endl;

  prefr::GLRenderer* renderer = new prefr::GLRenderer( );

  std::cout << "Created systems" << std::endl;

  _particleSystem->addUpdater( updater );
  _particleSystem->sorter( sorter );
  _particleSystem->renderer( renderer );

  _particleSystem->start();


  _resetParticles = true;
//  resetParticles( );

}

void OpenGLWidget::resetParticles( void )
{
  _particleSystem->run( false );

  for( auto cluster : _particleSystem->clusters( ))
  {
    switch( _simulationType )
    {
      case simil::TSimSpikes:
        cluster->killParticles( false );
      break;
      case simil::TSimVoltages:
        cluster->killParticles( false );
      break;
      default:
      break;
    }
    cluster->source( )->restart( );
  }

  _particleSystem->run( true );

  _particleSystem->update( 0.0f );
}

void OpenGLWidget::SetAlphaBlendingAccumulative( bool accumulative )
{
  _alphaBlendingAccumulative = accumulative;
}

void OpenGLWidget::changeSimulationColorMapping( const TTransferFunction& colors )
{

  if( _particleSystem )
  {

    prefr::vectortvec4 gcolors;

    for( auto c : colors )
    {
      glm::vec4 gColor( c.second.red( ) / 255.0f,
                        c.second.green( ) / 255.0f,
                        c.second.blue( ) / 255.0f,
                        c.second.alpha( ) / 255.0f );
      gcolors.Insert( c.first, gColor );
    }

    _prototype->color = gcolors;

    _particleSystem->update( 0.0f );
  }
}

TTransferFunction OpenGLWidget::getSimulationColorMapping( void )
{
  TTransferFunction result;

  if( _particleSystem )
  {
    prefr::vectortvec4 colors = _prototype->color;

    for( unsigned int i = 0; i < colors.size; i++ )
    {
      glm::vec4 c = colors.values[ i ];
      QColor color( c.r * 255, c.g * 255, c.b * 255, c.a * 255 );
      result.push_back( std::make_pair( colors.times[ i ], color ));
    }
  }

  return result;
}

void OpenGLWidget::changeSimulationSizeFunction( const TSizeFunction& sizes )
{
  if( _particleSystem )
  {
    utils::InterpolationSet< float > newSize;
    for( auto s : sizes )
    {
      std::cout << s.first << " " << s.second << std::endl;
      newSize.Insert( s.first, s.second );
    }
    _prototype->size = newSize;

    _particleSystem->update( 0.0f );
  }
}

TSizeFunction OpenGLWidget::getSimulationSizeFunction( void )
{
  TSizeFunction result;
  if( _particleSystem )
  {
    auto v = _prototype->size.values.begin( );
    for( auto s : _prototype->size.times)
    {
      result.push_back( std::make_pair( s, *v ));
      v++;
    }
  }
  return result;
}

void OpenGLWidget::changeSimulationDecayValue( float value )
{
  _maxLife = value;

  if( _prototype)
    _prototype->setLife( value, value );
}

float OpenGLWidget::getSimulationDecayValue( void )
{
  return _prototype->maxLife( );
}

#ifdef VISIMPL_USE_ZEROEQ

void OpenGLWidget::setSelectedGIDs( const std::unordered_set< uint32_t >& gids )
{
  if( gids.size( ) > 0 )
  {
    _selectedGIDs = gids;
    _pendingSelection = true;
  }
  std::cout << "Received " << _selectedGIDs.size( ) << " ids" << std::endl;

}


void ExpandBoundingBox( Eigen::Vector3f& minBounds,
                        Eigen::Vector3f& maxBounds,
                        const glm::vec3& position)
{
  for( unsigned int i = 0; i < 3; ++i )
  {
    minBounds( i ) = std::min( minBounds( i ), position[ i ] );
    maxBounds( i ) = std::max( maxBounds( i ), position[ i ] );
  }
}

void OpenGLWidget::updateSelection( void )
{
  if( _particleSystem /*&& _pendingSelection*/ )
  {
    _particleSystem->run( false );

    bool baseOn = !_showSelection || _selectedGIDs.size( ) == 0;

    Eigen::Vector3f boundingBoxMin( std::numeric_limits< float >::max( ),
                                    std::numeric_limits< float >::max( ),
                                    std::numeric_limits< float >::max( ));
    Eigen::Vector3f boundingBoxMax( std::numeric_limits< float >::min( ),
                                    std::numeric_limits< float >::min( ),
                                    std::numeric_limits< float >::min( ));

    for( auto cluster : _particleSystem->clusters( ))
    {
      auto it = nodesGIDMap.find( cluster );
      unsigned int id = it->second;

//      auto res = _selectedGIDs.find( id );

      bool on =  baseOn || _selectedGIDs.find( id ) != _selectedGIDs.end( );

      cluster->model( on ? _prototype : _offPrototype );

      if( ( _focusOnSelection && on ) || !_focusOnSelection )
      for( prefr::tparticle particle = cluster->particles( ).begin( );
           particle != cluster->particles( ).end( ); ++particle)
      {
        ExpandBoundingBox( boundingBoxMin,
                           boundingBoxMax,
                           particle.position( ));
      }
    }

    Eigen::Vector3f center = ( boundingBoxMax + boundingBoxMin ) * 0.5f;
    float radius = ( boundingBoxMax - center ).norm( );

    _camera->targetPivotRadius( center, radius );

    _particleSystem->run( true );
    _particleSystem->update( 0.0f );

//    _pendingSelection = false;

  }
}

void OpenGLWidget::showSelection( bool showSelection_ )
{

  _showSelection = showSelection_;

  updateSelection( );

}

void OpenGLWidget::ClearSelection( void )
{
  _selectedGIDs.clear( );
  updateSelection( );
}

#endif


void OpenGLWidget::updateSimulation( void )
{
  if( _player->isPlaying( ) || _firstFrame )
  {

    _particleSystem->update( _elapsedTimeRenderAcc );
    _firstFrame = false;
  }
}

void OpenGLWidget::paintParticles( void )
{
  if( !_particleSystem )
    return;

  glDepthMask(GL_FALSE);
  glEnable(GL_BLEND);

  glEnable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);

  if( _alphaBlendingAccumulative )
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA);
  else
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


  glFrontFace(GL_CCW);

  _particlesShader->use( );
      // unsigned int shader;
      // shader = _particlesShader->getID();
  unsigned int shader;
  shader = _particlesShader->program( );

  unsigned int uModelViewProjM, cameraUp, cameraRight;

  uModelViewProjM = glGetUniformLocation( shader, "modelViewProjM" );
  glUniformMatrix4fv( uModelViewProjM, 1, GL_FALSE,
                     _camera->viewProjectionMatrix( ));

  cameraUp = glGetUniformLocation( shader, "cameraUp" );
  cameraRight = glGetUniformLocation( shader, "cameraRight" );

  float* viewM = _camera->viewMatrix( );

  glUniform3f( cameraUp, viewM[1], viewM[5], viewM[9] );
  glUniform3f( cameraRight, viewM[0], viewM[4], viewM[8] );

  glm::vec3 cameraPosition ( _camera->position( )[ 0 ],
                             _camera->position( )[ 1 ],
                             _camera->position( )[ 2 ] );

//  bool cameraMoved = ( _lastCameraPosition == cameraPosition );

//  if( cameraMoved )
  {

//    std::cout << cameraPosition.x << ", "
//                << cameraPosition.y << ", "
//                << cameraPosition.z << std::endl;
//    std::cout << "Camera moved..." << std::endl;
    _particleSystem->updateCameraDistances( cameraPosition );
    

    _lastCameraPosition = cameraPosition;
  }

  _particleSystem->updateRender( );
  _particleSystem->render( );

  _particlesShader->unuse( );

}

void OpenGLWidget::paintGL( void )
{
  std::chrono::time_point< std::chrono::system_clock > now =
      std::chrono::system_clock::now( );

  unsigned int elapsedMilliseconds =
      std::chrono::duration_cast< std::chrono::milliseconds >
        ( now - _lastFrame ).count( );

  _lastFrame = now;

  _deltaTime = elapsedMilliseconds * 0.001f;

  _elapsedTimeRenderAcc += _deltaTime;
  _elapsedTimeSliderAcc += _deltaTime;

  _frameCount++;
  glDepthMask(GL_TRUE);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glDisable(GL_BLEND);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);

  updateSelection( );

  if( _resetParticles )
    resetParticles( );

  _resetParticles = false;

  if ( _paint )
  {
    _camera->anim( );

    if ( _particleSystem )
    {

      if( _elapsedTimeRenderAcc >= _renderPeriod )
      {
        _elapsedTimeSimAcc = _elapsedTimeRenderAcc * _playbackSpeed;

        _player->deltaTime( _elapsedTimeSimAcc );

        configureSimulation( );
        updateSimulation( );
        _elapsedTimeRenderAcc = 0.0f;
      }

      paintParticles( );

    }
    glUseProgram( 0 );
    glFlush( );

  }

   if( _player && _elapsedTimeSliderAcc > SIM_SLIDER_UPDATE_PERIOD )
   {
     _elapsedTimeSliderAcc = 0.0f;

#ifdef VISIMPL_USE_ZEROEQ
       _player->sendCurrentTimestamp( );
#endif

     emit updateSlider( _player->GetRelativeTime( ));
   }


  #define FRAMES_PAINTED_TO_MEASURE_FPS 10
  if ( _frameCount == FRAMES_PAINTED_TO_MEASURE_FPS )
  {

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
    update( );
  }
  else
  {
    _fpsLabel.setVisible( false );
  }

}

void OpenGLWidget::resizeGL( int w , int h )
{
  _camera->ratio((( double ) w ) / h );
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
    _camera->localRotation( -( _mouseX - event_->x( )) * 0.01,
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
    _camera->radius( _camera->radius( ) / 1.1f );
  else
    _camera->radius( _camera->radius( ) * 1.1f );

  update( );

}



void OpenGLWidget::keyPressEvent( QKeyEvent* event_ )
{
  makeCurrent( );

  switch ( event_->key( ))
  {
  case Qt::Key_C:
    _camera->pivot( Eigen::Vector3f( 0.0f, 0.0f, 0.0f ));
    _camera->radius( 1000.0f );
    _camera->rotation( 0.0f, 0.0f );
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
  _focusOnSelection = !_focusOnSelection;
  update( );
}


simil::SimulationPlayer* OpenGLWidget::player( )
{
  return _player;
}

void OpenGLWidget::Play( void )
{
  if( _player )
  {
    _player->Play( );
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
      _player->Play( );
    else
      _player->Pause( );
  }
}

void OpenGLWidget::Stop( void )
{
  if( _player )
  {
    _player->Stop( );
    _resetParticles = true;
//    resetParticles( );
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

void OpenGLWidget::PlayAt( float percentage )
{
  if( _player )
  {
    _particleSystem->run( false );

//    resetParticles( );
    _resetParticles = true;

    std::cout << "Play at " << percentage << std::endl;
    _player->PlayAt( percentage );
    _particleSystem->run( true );
  }
}

void OpenGLWidget::Restart( void )
{
  if( _player )
  {
    bool playing = _player->isPlaying( );
    _player->Stop( );
    if( playing )
      _player->Play( );
//    resetParticles( );
    _resetParticles = true;
    _firstFrame = true;
  }
}

void OpenGLWidget::GoToEnd( void )
{

}


void OpenGLWidget::reducePlaybackSpeed( )
{
  _playbackSpeed -= 0.01f;
  if( _playbackSpeed < 0.01f )
    _playbackSpeed = 0.01f;

  std::cout << "Playback speed: x" << _playbackSpeed << std::endl;
}

void OpenGLWidget::increasePlaybackSpeed( )
{

  _playbackSpeed += 0.01f;

  std::cout << "Playback speed: x" << _playbackSpeed << std::endl;
}
