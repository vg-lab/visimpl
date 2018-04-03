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
#include <QGraphicsOpacityEffect>

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

namespace visimpl
{

  static float invRGBInt = 1.0f / 255;

  OpenGLWidget::OpenGLWidget( QWidget* parent_,
                              Qt::WindowFlags windowsFlags_,
                              const std::string&
  #ifdef VISIMPL_USE_ZEROEQ
                              zeqUri
  #endif
    )
  : QOpenGLWidget( parent_, windowsFlags_ )
  #ifdef VISIMPL_USE_ZEROEQ
  , _zeqUri( zeqUri )
  #endif
  , _fpsLabel( nullptr )
  , _showFps( false )
  , _wireframe( false )
  , _camera( nullptr )
  , _lastCameraPosition( 0, 0, 0)
  , _focusOnSelection( true )
  , _pendingSelection( false )
  , _backtrace( false )
  , _playbackMode( TPlaybackMode::CONTINUOUS )
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
  , _maxLife( 0.0f )
  , _deltaTime( 0.0f )
  , _sbsTimePerStep( 5.0f )
  , _sbsBeginTime( 0 )
  , _sbsEndTime( 0 )
  , _sbsCurrentTime( 0 )
  , _sbsCurrentRenderDelta( 0 )
  , _sbsPlaying( false )
  , _sbsFirstStep( true )
  , _sbsNextStep( false )
  , _sbsPrevStep( false )
  , _simDeltaTime( 0.125f )
  , _timeStepsPerSecond( 2.0f )
  , _simTimePerSecond( 0.5f )
  , _firstFrame( true )
  , _prototype( nullptr )
  , _offPrototype( nullptr )
  , _renderSpeed( 0.0f )
  , _simPeriod( 0.0f )
  , _simPeriodMicroseconds( 0.0f )
  , _renderPeriod( 0.0f )
  , _renderPeriodMicroseconds( 0.0f )
  , _sliderUpdatePeriod( 0.25f )
  , _elapsedTimeRenderAcc( 0.0f )
  , _elapsedTimeSliderAcc( 0.0f )
  , _elapsedTimeSimAcc( 0.0f )
  , _alphaBlendingAccumulative( false )
  , _showSelection( false )
  , _resetParticles( false )
  , _updateSelection( false )
  , _showActiveEvents( true )
  , _subsetEvents( nullptr )
  , _deltaEvents( 0.125f )
  , _inputMux( nullptr )
  {
  #ifdef VISIMPL_USE_ZEROEQ
    if ( zeqUri != "" )
      _camera = new reto::Camera( zeqUri );
    else
  #endif
    _camera = new reto::Camera( );
    _camera->farPlane( 100000.f );
    _camera->animDuration( 0.5f );

    _lastCameraPosition = glm::vec3( 0, 0, 0 );

    _maxFPS = 60.0f;
    _renderPeriod = 1.0f / _maxFPS;
    _renderPeriodMicroseconds = _renderPeriod * 1000000;

    _sbsInvTimePerStep = 1.0 / _sbsTimePerStep;

    _sliderUpdatePeriodMicroseconds = _sliderUpdatePeriod * 1000000;

    _renderSpeed = 1.f;

    _fpsLabel = new QLabel( );
    _fpsLabel->setStyleSheet(
      "QLabel { background-color : #333;"
      "color : white;"
      "padding: 3px;"
      "margin: 10px;"
      " border-radius: 10px;}" );
    _fpsLabel->setVisible( _showFps );
    _fpsLabel->setMaximumSize( 100, 50 );


    _eventLabelsLayout = new QGridLayout( );
    _eventLabelsLayout->setAlignment( Qt::AlignTop );
    _eventLabelsLayout->setMargin( 0 );
    setLayout( _eventLabelsLayout );
    _eventLabelsLayout->addWidget( _fpsLabel, 0, 0, 1, 9 );

    _colorPalette =
        scoop::ColorPalette::colorBrewerQualitative(
            scoop::ColorPalette::ColorBrewerQualitative::Set1, 9 );

    // This is needed to get key events
    this->setFocusPolicy( Qt::WheelFocus );




  //  new QShortcut( QKeySequence( Qt::CTRL + Qt::Key_Minus ),
  //                 this, SLOT( reducePlaybackSpeed( ) ));
  //
  //  new QShortcut( QKeySequence( Qt::CTRL + Qt::Key_Plus ),
  //                   this, SLOT( increasePlaybackSpeed( ) ));
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

    _deltaTime = 0.5f;

    simil::SpikeData* spikeData = new simil::SpikeData( fileName, fileType, report );
    spikeData->reduceDataToGIDS( );

    simil::SpikesPlayer* spPlayer = new simil::SpikesPlayer( );
    spPlayer->LoadData( spikeData );
    _player = spPlayer;
    _player->deltaTime( _deltaTime );


    float scale = 1.0f;

    switch( fileType )
    {
      case simil::TBlueConfig:

        simulationDeltaTime( 0.5f );
        simulationStepsPerSecond( 20.0f );
        changeSimulationDecayValue( 20.0f );
        break;
      case simil::THDF5:
        simulationDeltaTime( 0.005f );
        simulationStepsPerSecond( 20.0f );
        changeSimulationDecayValue( 0.1f );
        scale = 500.f;
        break;
      default:
        break;
    }

    createParticleSystem( scale );

    _inputMux = new InputMultiplexer( _particleSystem, _player->gids( ) );
    _inputMux->models( _prototype, _offPrototype );

  #ifdef VISIMPL_USE_ZEROEQ
    _player->connectZeq( _zeqUri );
  #endif
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

  void OpenGLWidget::configureSimulationFrame( void )
  {
    if( !_player || !_player->isPlaying( ) || !_particleSystem->run( ))
      return;

    float prevTime = _player->currentTime( );

    if( _backtrace )
    {

      backtraceSimulation( );

      _backtrace = false;
    }

    _player->Frame( );

    float currentTime = _player->currentTime( );

    _inputMux->processInput( _player->spikesNow( ), prevTime,
                             currentTime, false );

  }

  void OpenGLWidget::configurePreviousStep( void )
  {
    if( !_player || !_particleSystem->run( ))
      return;
//TODO
    _sbsBeginTime = _sbsFirstStep ?
                    _player->currentTime( ):
                    _sbsPlaying ? _sbsBeginTime : _sbsEndTime;

    _sbsBeginTime = std::max( ( double ) _player->startTime( ),
                              _sbsBeginTime - _player->deltaTime( ));

    _sbsEndTime = _sbsBeginTime + _player->deltaTime( );

    _player->GoTo( _sbsBeginTime );

    backtraceSimulation( );

    _sbsStepSpikes = _player->spikesBetween( _sbsBeginTime, _sbsEndTime );

    _sbsCurrentTime = _sbsBeginTime;
    _sbsCurrentSpike = _sbsStepSpikes.first;

    _sbsFirstStep = false;
    _sbsPlaying = true;
  }

  void OpenGLWidget::configureStepByStep( void )
  {

    if( !_player || ! _player->isPlaying( ) || !_particleSystem->run( ))
      return;

    _sbsBeginTime = _sbsFirstStep ? _player->currentTime( ) : _sbsEndTime;

    _sbsEndTime = _sbsBeginTime + _player->deltaTime( );

    if( _sbsPlaying )
    {
      _player->GoTo( _sbsBeginTime );

      backtraceSimulation( );
    }

    _sbsStepSpikes = _player->spikesBetween( _sbsBeginTime, _sbsEndTime );

    _sbsCurrentTime = _sbsBeginTime;
    _sbsCurrentSpike = _sbsStepSpikes.first;

    _sbsFirstStep = false;
    _sbsPlaying = true;

  }

  void OpenGLWidget::configureStepByStepFrame( double elapsedRenderTime )
  {

    _sbsCurrentRenderDelta = 0;

    if( _sbsPlaying && _sbsCurrentTime >= _sbsEndTime )
    {
      _sbsPlaying = false;
      _player->GoTo( _sbsEndTime );
      _player->Pause( );
      emit stepCompleted( );
    }

    if( _sbsPlaying )
    {
      double diff = _sbsEndTime - _sbsCurrentTime;
      double renderDelta  = elapsedRenderTime * _sbsInvTimePerStep * _player->deltaTime( );
      _sbsCurrentRenderDelta = std::min( diff, renderDelta );

      double nextTime = _sbsCurrentTime + _sbsCurrentRenderDelta;

      auto spikeIt = _sbsCurrentSpike;
      while( spikeIt->first < nextTime  &&
             spikeIt - _sbsStepSpikes.first < _sbsStepSpikes.second - _sbsStepSpikes.first )
      {
        ++spikeIt;
      }

      if( spikeIt != _sbsCurrentSpike )
      {
        simil::SpikesCRange frameSpikes = std::make_pair( _sbsCurrentSpike, spikeIt );
        _inputMux->processInput( frameSpikes, _sbsCurrentTime, nextTime, false );
      }

      _sbsCurrentTime = nextTime;
      _sbsCurrentSpike = spikeIt;
    }

  }

  void OpenGLWidget::backtraceSimulation( void )
  {
    float endTime = _player->currentTime( );
    float startTime = std::max( 0.0f, endTime - _maxLife );
    simil::SpikesCRange context =
        _player->spikesBetween( startTime, endTime );

    if( context.first != context.second )
      _inputMux->processInput( context, startTime, endTime, true );
  }


  void expandBoundingBox( glm::vec3& minBounds,
                          glm::vec3& maxBounds,
                          const glm::vec3& position)
  {
    for( unsigned int i = 0; i < 3; ++i )
    {
      minBounds[ i ] = std::min( minBounds[ i ], position[ i ] );
      maxBounds[ i ] = std::max( maxBounds[ i ], position[ i ] );
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

    _particleSystem->parallel( true );

    const TPosVect& positions = _player->positions( );

    _particlesShader = new reto::ShaderProgram( );
    _particlesShader->loadVertexShaderFromText( prefr::prefrVertexShader );
    _particlesShader->loadFragmentShaderFromText( prefr::prefrFragmentShader );
    _particlesShader->create( );
    _particlesShader->link( );

    _offPrototype = new prefr::ColorOperationModel( _maxLife, _maxLife );

    _offPrototype->color.Insert( 0.0f, ( glm::vec4(0.1f, 0.1f, 0.1f, 0.2f)));

    _offPrototype->velocity.Insert( 0.0f, 0.0f );

    _offPrototype->size.Insert( 1.0f, 10.0f );

    _particleSystem->addModel( _offPrototype );


    _prototype = new prefr::ColorOperationModel( _maxLife, _maxLife );

    _particleSystem->addModel( _prototype );


    prefr::Updater* updater;

    switch( _simulationType )
    {
     case simil::TSimSpikes:

       updater = new prefr::ValuedUpdater( );
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
    TGIDSet::const_iterator gid = _player->gids( ).begin();


    _boundingBoxMin = glm::vec3( std::numeric_limits< float >::max( ),
                                 std::numeric_limits< float >::max( ),
                                 std::numeric_limits< float >::max( ));

    _boundingBoxMax = glm::vec3( std::numeric_limits< float >::min( ),
                                 std::numeric_limits< float >::min( ),
                                 std::numeric_limits< float >::min( ));

    for ( auto brionPos : positions )
    {
      cluster = new prefr::Cluster( );
      cluster->model( _prototype );
      cluster->updater( updater );
      cluster->active( true );


      glm::vec3 position( brionPos.x( ), brionPos.y( ), brionPos.z( ));

      if( scale != 1.0f )
        position *= scale;

      expandBoundingBox( _boundingBoxMin, _boundingBoxMax, position );

      start = i * partPerEmitter;

      switch( _simulationType )
      {
      case simil::TSimSpikes:
        source = new prefr::ValuedSource( -1.0f, position,
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

      cluster->inactiveKillParticles( false );
//      source->maxEmissionCycles( 1 );

      i++;
      gid++;

    }

    updateCameraBoundingBox( );

    prefr::Sorter* sorter = new prefr::Sorter( );

    std::cout << "Created sorter" << std::endl;

    prefr::GLRenderer* renderer = new prefr::GLRenderer( );

    std::cout << "Created systems" << std::endl;

    _particleSystem->addUpdater( updater );
    _particleSystem->sorter( sorter );
    _particleSystem->renderer( renderer );

    _particleSystem->start();


    _resetParticles = true;

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

      _particleSystem->updateCameraDistances( cameraPosition );

      _lastCameraPosition = cameraPosition;

      _particleSystem->updateRender( );
      _particleSystem->render( );

      _particlesShader->unuse( );

    }

    void OpenGLWidget::paintGL( void )
    {
      std::chrono::time_point< std::chrono::system_clock > now =
          std::chrono::system_clock::now( );

      unsigned int elapsedMicroseconds =
          std::chrono::duration_cast< std::chrono::microseconds >
            ( now - _lastFrame ).count( );

      _lastFrame = now;

      _deltaTime = elapsedMicroseconds * 0.000001;

      if( _player && _player->isPlaying( ))
      {
        _elapsedTimeSimAcc += elapsedMicroseconds;
        _elapsedTimeRenderAcc += elapsedMicroseconds;
        _elapsedTimeSliderAcc += elapsedMicroseconds;
      }
      _frameCount++;
      glDepthMask(GL_TRUE);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glDisable(GL_BLEND);
      glEnable(GL_DEPTH_TEST);
      glEnable(GL_CULL_FACE);

      if( _updateSelection )
        updateSelection( );

      if( _resetParticles )
        _inputMux->resetParticles( );

      _resetParticles = false;

      if ( _paint )
      {
        _camera->anim( );

        if ( _particleSystem )
        {
          if( _player && _player->isPlaying( ))
          {
            switch( _playbackMode )
            {
              // Continuous mode (Default)
              case TPlaybackMode::CONTINUOUS:
              {
                if( _elapsedTimeSimAcc >= _simPeriodMicroseconds )
                {
                  configureSimulationFrame( );
                  updateEventLabelsVisibility( );

                  _elapsedTimeSimAcc = 0.0f;
                }


                break;
              }
              // Step by step mode
              case TPlaybackMode::STEP_BY_STEP:
              {
                if( _sbsPrevStep )
                {
                  configurePreviousStep( );
                  _sbsPrevStep = false;
                }
                else if( _sbsNextStep )
                {
                  configureStepByStep( );
                  _sbsNextStep = false;
                }
                break;
              }
              default:
                break;
            }


            if( _elapsedTimeRenderAcc >= _renderPeriodMicroseconds )
            {
              double renderDelta = 0;

              switch( _playbackMode )
              {
              case TPlaybackMode::CONTINUOUS:
                renderDelta = _elapsedTimeRenderAcc * _simTimePerSecond * 0.000001;
                break;
              case  TPlaybackMode::STEP_BY_STEP:
                configureStepByStepFrame( _elapsedTimeRenderAcc * 0.000001 );
                renderDelta = _sbsCurrentRenderDelta;
                break;
              default:
                renderDelta = 0;
              }

              updateParticles( renderDelta );
              _elapsedTimeRenderAcc = 0.0f;
            }

          }

          paintParticles( );

        }
        glUseProgram( 0 );
        glFlush( );

      }

      if( _player && _elapsedTimeSliderAcc > _sliderUpdatePeriodMicroseconds )
      {

        _elapsedTimeSliderAcc = 0.0f;

    #ifdef VISIMPL_USE_ZEROEQ
        _player->sendCurrentTimestamp( );
    #endif

        emit updateSlider( _player->GetRelativeTime( ));
      }


      #define FRAMES_PAINTED_TO_MEASURE_FPS 10
      if( _showFps && _frameCount >= FRAMES_PAINTED_TO_MEASURE_FPS )
      {
        auto duration =
          std::chrono::duration_cast<std::chrono::milliseconds>( now - _then );
        _then = now;

        MainWindow* mainWindow = dynamic_cast< MainWindow* >( parent( ));
        if( mainWindow )
        {

          unsigned int ellapsedMiliseconds = duration.count( );

          unsigned int fps = roundf( 1000.0f *
                                     float( _frameCount ) /
                                     float( ellapsedMiliseconds ));

          if( _showFps )
          {
            _fpsLabel->setText( QString::number( fps ) + QString( " FPS" ));
            _fpsLabel->adjustSize( );
          }

        }

        _frameCount = 0;
      }

      if( _idleUpdate )
      {
        update( );
      }

    }


  void OpenGLWidget::setSelectedGIDs( const std::unordered_set< uint32_t >& gids )
  {
    if( gids.size( ) > 0 )
    {
      _selectedGIDs = gids;
      _pendingSelection = true;

      if( !_inputMux->showGroups( ))
        setUpdateSelection( );
      _inputMux->selection( gids );

    }
    std::cout << "Received " << _selectedGIDs.size( ) << " ids" << std::endl;

  }

  void OpenGLWidget::setUpdateSelection( void )
  {
    _updateSelection = true;
  }

  void OpenGLWidget::updateSelection( void )
  {
    if( _particleSystem /*&& _pendingSelection*/ )
    {
      _particleSystem->run( false );

//      bool baseOn = !_showSelection || _selectedGIDs.empty( );

      _boundingBoxMin = glm::vec3( std::numeric_limits< float >::max( ),
                                   std::numeric_limits< float >::max( ),
                                   std::numeric_limits< float >::max( ));

      _boundingBoxMax = glm::vec3( std::numeric_limits< float >::min( ),
                                   std::numeric_limits< float >::min( ),
                                   std::numeric_limits< float >::min( ));

      std::vector< prefr::Cluster* > active =
        _inputMux->activeClusters( );

      for( auto cluster : ( !active.empty( ) ? active :_particleSystem->clusters( ) ))
      {
        auto particleRange = cluster->particles( );
        for( prefr::tparticle particle = particleRange.begin( );
            particle != particleRange.end( ); ++particle )
        {
          expandBoundingBox( _boundingBoxMin,
                             _boundingBoxMax,
                             particle.position( ));
        }

      }

      updateCameraBoundingBox( );

      _particleSystem->run( true );
      _particleSystem->update( 0.0f );

      _updateSelection = false;
    }

  }

  void OpenGLWidget::showSelection( bool showSelection_ )
  {

    _showSelection = showSelection_;

    if( _inputMux )
      _inputMux->showGroups( !_showSelection );

    updateSelection( );

  }

  void OpenGLWidget::clearSelection( void )
  {
    _inputMux->clearSelection( );
    _selectedGIDs.clear( );
    _updateSelection = true;
  }

  void OpenGLWidget::updateCameraBoundingBox( void )
  {
    glm::vec3 center = ( _boundingBoxMax + _boundingBoxMin ) * 0.5f;
    float side = glm::length( _boundingBoxMax - center );
    float radius = side / std::tan( _camera->fov( ));

    _camera->targetPivotRadius( Eigen::Vector3f( center.x, center.y, center.z ),
                                radius );

  }

  void OpenGLWidget::showEventsActivityLabels( bool show )
  {
    for( auto container : _eventLabels )
    {
      container.upperWidget->setVisible( show );
      container.upperWidget->update( );
    }
  }


  void OpenGLWidget::updateParticles( float renderDelta )
  {
    if( _player->isPlaying( ) || _firstFrame )
    {

      _particleSystem->update( renderDelta );
      _firstFrame = false;
    }
  }



  void OpenGLWidget::updateEventLabelsVisibility( void )
  {
    std::vector< bool > visibility = activeEventsAt( _player->currentTime( ));

    unsigned int counter = 0;
    for( auto showLabel : visibility )
    {
      EventLabel& labelObjects = _eventLabels[ counter ];
//      QString style( "border: 1px solid " + showLabel ? "white;" : "black;" );
      QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect( );
      effect->setOpacity( showLabel ? 1.0 : 0.5 );
      labelObjects.upperWidget->setGraphicsEffect( effect );
      labelObjects.upperWidget->update( );
      ++counter;
    }



  }


  std::vector< bool > OpenGLWidget::activeEventsAt( float time )
  {
    std::vector< bool > result( _eventLabels.size( ), false);

    float totalTime = _player->endTime( ) - _player->startTime( );

    double perc = time / totalTime;
    unsigned int counter = 0;
    for( auto event : _eventsActivation )
    {
      unsigned int position = perc * event.size( );
      result[ counter ] = event[ position ];

      ++counter;
    }

    return result;
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
      _camera->radius( _camera->radius( ) / 1.3f );
    else
      _camera->radius( _camera->radius( ) * 1.3f );

  //  std::cout << "Camera radius " << _camera->radius( ) << std::endl;

    update( );

  }



  void OpenGLWidget::keyPressEvent( QKeyEvent* event_ )
  {
    switch ( event_->key( ))
    {
    case Qt::Key_C:
        _updateSelection = true;
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

    _fpsLabel->setVisible( _showFps );

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

  void OpenGLWidget::idleUpdate( bool idleUpdate_ )
  {
    _idleUpdate = idleUpdate_;
  }

  TPlaybackMode OpenGLWidget::playbackMode( void )
  {
    return _playbackMode;
  }

  void OpenGLWidget::playbackMode( TPlaybackMode mode )
  {
    if( mode != TPlaybackMode::AB_REPEAT )
      _playbackMode = mode;
  }

  bool OpenGLWidget::completedStep( void )
  {
    return !_sbsPlaying;
  }

  simil::SimulationPlayer* OpenGLWidget::player( )
  {
    return _player;
  }

  float OpenGLWidget::currentTime( void )
  {
    switch( _playbackMode )
    {
    case TPlaybackMode::STEP_BY_STEP:
      return _sbsCurrentTime;
    default:
      return _player->currentTime( );
    }
  }

  InputMultiplexer* OpenGLWidget::inputMultiplexer( void )
  {
    return _inputMux;
  }

  void OpenGLWidget::subsetEventsManager( simil::SubsetEventManager* manager )
  {
    _subsetEvents = manager;

    createEventLabels( );

    update( );
  }

  const scoop::ColorPalette& OpenGLWidget::colorPalette( void )
  {
    return _colorPalette;
  }

  void OpenGLWidget::createEventLabels( void )
  {
    const auto& eventNames = _subsetEvents->eventNames( );

    if( eventNames.empty( ))
      return;


    for( auto& label : _eventLabels )
    {
      _eventLabelsLayout->removeWidget( label.upperWidget );

      delete( label.frame );
      delete( label.label );
      delete( label.upperWidget );

    }

    _eventLabels.clear( );
    _eventsActivation.clear( );


    unsigned int row = 0;

    float totalTime = _player->endTime( ) - _player->startTime( );

    unsigned int activitySize =
        std::ceil( totalTime / _deltaEvents );

    scoop::ColorPalette::Colors colors = _colorPalette.colors( );

    for( auto name : eventNames )
    {

      EventLabel labelObjects;

      QFrame* frame = new QFrame( );
//      std::cout << "Creating color: " << .toStdString( ) << std::endl;
      frame->setStyleSheet( "background-color: " + colors[ row ].name( ) );
      frame->setMinimumSize( 20, 20 );
      frame->setMaximumSize( 20, 20 );

      QLabel* label = new QLabel( );
//      label->setVisible( false );
      label->setMaximumSize( 100, 50 );
      label->setTextFormat( Qt::RichText );
      label->setStyleSheet(
        "QLabel { background-color : #333;"
        "color : white;"
        "padding: 3px;"
        "margin: 10px;"
        " border-radius: 10px;}" );

      label->setText( "" + QString( name.c_str( )));

      QWidget* container = new QWidget( );
      QHBoxLayout* labelLayout = new QHBoxLayout( );

      labelLayout->addWidget( frame );
      labelLayout->addWidget( label );
      container->setLayout( labelLayout );

      labelObjects.frame = frame;
      labelObjects.label = label;
      labelObjects.upperWidget = container;

      _eventLabels.push_back( labelObjects );

//      _eventLabels.push_back( label );

      _eventLabelsLayout->addWidget( container, row, 10, 2, 1 );

      std::vector< bool > activity( activitySize );

      _eventsActivation.push_back(
          _subsetEvents->eventActivity( name, _deltaEvents, totalTime ));

      ++row;
    }
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
      _resetParticles = true;

      _backtrace = true;

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

  void OpenGLWidget::PreviousStep( void )
  {
    if( _playbackMode != TPlaybackMode::STEP_BY_STEP )
    {
      _playbackMode = TPlaybackMode::STEP_BY_STEP;
      _sbsFirstStep = true;
    }

    _sbsPrevStep = true;
    _sbsNextStep = false;
  }

  void OpenGLWidget::NextStep( void )
  {
    if( _playbackMode != TPlaybackMode::STEP_BY_STEP )
    {
      _playbackMode = TPlaybackMode::STEP_BY_STEP;
      _sbsFirstStep = true;
    }

    _sbsPrevStep = false;
    _sbsNextStep = true;
  }

  void OpenGLWidget::SetAlphaBlendingAccumulative( bool accumulative )
  {
    _alphaBlendingAccumulative = accumulative;
  }

  void OpenGLWidget::changeSimulationColorMapping( const TTransferFunction& colors )
  {

    prefr::vectortvec4 gcolors;

    for( auto c : colors )
    {
      glm::vec4 gColor( c.second.red( ) * invRGBInt,
                       c.second.green( ) * invRGBInt,
                       c.second.blue( ) * invRGBInt,
                       c.second.alpha( ) * invRGBInt );

      gcolors.Insert( c.first, gColor );
    }

    _prototype->color = gcolors;

  }

  TTransferFunction OpenGLWidget::getSimulationColorMapping( void )
  {
    TTransferFunction result;

    prefr::vectortvec4 colors = _prototype->color;

    auto timeValue = _prototype->color.times.begin( );
    for( auto c : _prototype->color.values )
    {
      QColor color( c.r * 255, c.g * 255, c.b * 255, c.a * 255 );
      result.push_back( std::make_pair( *timeValue, color ));

      ++timeValue;
    }


    return result;
  }

  void OpenGLWidget::changeSimulationSizeFunction( const TSizeFunction& sizes )
  {

    utils::InterpolationSet< float > newSize;
    for( auto s : sizes )
    {
      newSize.Insert( s.first, s.second );
    }
    _prototype->size = newSize;

  }

  TSizeFunction OpenGLWidget::getSimulationSizeFunction( void )
  {
    TSizeFunction result;
    auto sizeValue = _prototype->size.values.begin( );
    for( auto s : _prototype->size.times)
    {
      result.push_back( std::make_pair( s, *sizeValue ));
      ++sizeValue;
    }

    return result;
  }


  void OpenGLWidget::simulationDeltaTime( float value )
  {
    std::cout << "Delta time: " << value << std::endl;
    _player->deltaTime( value );

    _simDeltaTime = value;

    _simTimePerSecond = ( _simDeltaTime * _timeStepsPerSecond );
  }

  float OpenGLWidget::simulationDeltaTime( void )
  {
    return _player->deltaTime( );
  }

  void OpenGLWidget::simulationStepsPerSecond( float value )
  {
    _timeStepsPerSecond = value;

    _simPeriod = 1.0f / ( _timeStepsPerSecond );
    _simPeriodMicroseconds = _simPeriod * 1000000;
    _simTimePerSecond = ( _simDeltaTime * _timeStepsPerSecond );
  }

  float OpenGLWidget::simulationStepsPerSecond( void )
  {
    return _timeStepsPerSecond;
  }

  void OpenGLWidget::simulationStepByStepDuration( float value )
  {
    _sbsTimePerStep = value;
    _sbsInvTimePerStep = 1.0 / _sbsTimePerStep;
  }

  float OpenGLWidget::simulationStepByStepDuration( void )
  {
    return _sbsTimePerStep;
  }

  void OpenGLWidget::changeSimulationDecayValue( float value )
  {
    _maxLife = value;

    if( _inputMux )
      _inputMux->decay( value );

    if( _prototype)
      _prototype->setLife( value, value );
  }

  float OpenGLWidget::getSimulationDecayValue( void )
  {
    return _prototype->maxLife( );
  }

} // namespace visimpl
