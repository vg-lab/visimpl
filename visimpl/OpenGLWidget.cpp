/*
 * Copyright (c) 2015-2020 VG-Lab/URJC.
 *
 * Authors: Sergio E. Galindo <sergio.galindo@urjc.es>
 *
 * This file is part of ViSimpl <https://github.com/vg-lab/visimpl>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3.0 as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

// ViSimpl
#include "OpenGLWidget.h"
#include "MainWindow.h"
#include "DomainManager.h"

// Qt
#include <QWidget>
#include <QOpenGLContext>
#include <QMouseEvent>
#include <QColorDialog>
#include <QShortcut>
#include <QGraphicsOpacityEffect>
#include <QLabel>
#include <QDir>
#include <QOpenGLDebugLogger>

// C++
#include <string>
#include <iostream>
#include <map>
// GLM
#include <glm/glm.hpp>

#ifdef SIMIL_USE_BRION

#include <brain/brain.h>

#endif

#ifdef VISIMPL_USE_ZEROEQ

#include <zeroeq/zeroeq.h>

#endif

#include "visimpl/particlelab/ParticleLabShaders.h"
#include "GlewInitializer.h"

constexpr float ZOOM_FACTOR = 1.3f;
constexpr float TRANSLATION_FACTOR = 0.001f;
constexpr float ROTATION_FACTOR = 0.01f;
constexpr float DEFAULT_DELTA_TIME = 0.5f;
const QString INITIAL_CAMERA_POSITION = "0,0,0;1;1,0,0,0,1,0,0,0,1";

namespace visimpl
{
  const InitialConfig _initialConfigSimBlueConfig =
    std::make_tuple( 0.5f , 20.0f , 20.0f , 1.0f );
  const InitialConfig _initialConfigSimH5 =
    std::make_tuple( 0.1f , 20.0f , 1.5f , 5.0f );
  const InitialConfig _initialConfigSimCSV =
    //std::make_tuple( 0.005f, 20.0f, 0.1f, 50000.0f );
    std::make_tuple( 0.2f , 2.0f , 1.5f , 5.0f );

  const InitialConfig _initialConfigSimREST =
    std::make_tuple( 0.005f , 20.0f , 0.1f , 500.0f );

  constexpr float invRGBInt = 1.0f / 255;

  OpenGLWidget::OpenGLWidget( QWidget* parent_ ,
                              Qt::WindowFlags windowsFlags_ ,
                              const std::string&
#ifdef VISIMPL_USE_ZEROEQ
    zeqUri
#endif
                            )
    : QOpenGLWidget( parent_ , windowsFlags_ )
#ifdef VISIMPL_USE_ZEROEQ
    , _zeqUri( zeqUri )
#endif
    , _gl( )
    , _fpsLabel( nullptr )
    , _labelCurrentTime( nullptr )
    , _showFps( false )
    , _showCurrentTime( true )
    , _wireframe( false )
    , _camera( nullptr )
    , _lastCameraPosition( 0 , 0 , 0 )
    , _scaleFactor( 1.0f , 1.0f , 1.0f )
    , _scaleFactorExternal( false )
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
    , _currentClearColor( 20 , 20 , 20 , 255 )
    , _particleSystemInitialized( false )
    , _shaderClippingPlanes( nullptr )
    , _player( nullptr )
    , _clippingPlaneLeft( std::make_shared< reto::ClippingPlane >( ))
    , _clippingPlaneRight( std::make_shared< reto::ClippingPlane >( ))
    , _planeHeight( 1 )
    , _planeWidth( 1 )
    , _planeDistance( 20 )
    , _rotationPlanes( false )
    , _translationPlanes( false )
    , _paintClippingPlanes( true )
    , _planesColor( 1.0 , 1.0 , 1.0 , 1.0 )
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
    , _flagNewData( false )
    , _flagUpdateSelection( false )
    , _flagUpdateAttributes( false )
    , _newMode( VisualMode::Selection )
    , _flagAttribChange( false )
    , _newAttrib( T_TYPE_UNDEFINED )
    , _currentAttrib( T_TYPE_MORPHO )
    , _showActiveEvents( true )
    , _subsetEvents( nullptr )
    , _deltaEvents( 0.125f )
    , _domainManager( )
    , _screenPlaneShader( nullptr )
    , _quadVAO( 0 )
    , _weightFrameBuffer( 0 )
    , _accumulationTexture( 0 )
    , _revealTexture( 0 )
  {
    _lastCameraPosition = glm::vec3( 0 , 0 , 0 );

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
    _fpsLabel->setMaximumSize( 100 , 50 );

    _labelCurrentTime = new QLabel( );
    _labelCurrentTime->setStyleSheet(
      "QLabel { background-color : #333;"
      "color : white;"
      "padding: 3px;"
      "margin: 10px;"
      " border-radius: 10px;}" );
    _labelCurrentTime->setVisible( _showCurrentTime );
    _labelCurrentTime->setMaximumSize( 100 , 50 );

    _eventLabelsLayout = new QGridLayout( );
    _eventLabelsLayout->setAlignment( Qt::AlignTop );
    _eventLabelsLayout->setMargin( 0 );
    setLayout( _eventLabelsLayout );
    _eventLabelsLayout->addWidget( _labelCurrentTime , 0 , 0 , 1 , 9 );
    _eventLabelsLayout->addWidget( _fpsLabel , 1 , 0 , 1 , 9 );

    _colorPalette =
      scoop::ColorPalette::colorBrewerQualitative(
        scoop::ColorPalette::ColorBrewerQualitative::Set1 , 9 );

    // This is needed to get key events
    this->setFocusPolicy( Qt::WheelFocus );

#ifdef VISIMPL_USE_ZEROEQ
    if ( !_zeqUri.empty( ))
    {
      bool failed = false;
      try
      {
        auto& instance = ZeroEQConfig::instance( );
        if ( !instance.isConnected( ))
        {
          instance.connect( _zeqUri );
        }

        _camera = std::make_shared< Camera >( _zeqUri , instance.subscriber( ));
      }
      catch ( std::exception& e )
      {
        std::cerr << e.what( ) << " " << __FILE__ << ":" << __LINE__
                  << std::endl;
        failed = true;
      }
      catch ( ... )
      {
        std::cerr << "Unknown exception catched when initializing camera. "
                  << __FILE__ << ":" << __LINE__ << std::endl;
        failed = true;
      }

      if ( failed )
      {
        std::cerr << "Unable to connect to ZeroEQ." << std::endl;
        _camera = std::make_shared< Camera >( );
        _zeqUri.clear( );
      }
    }
#else
    _camera = std::make_shared< Camera >( );
#endif
    Q_ASSERT( _camera );

    setAutoFillBackground( true );
    setPalette( QPalette( QPalette::Window , Qt::black ));
  }

  OpenGLWidget::~OpenGLWidget( void )
  {
    delete _player;
  }

  void OpenGLWidget::initializeGL( void )
  {
    _gl.initializeOpenGLFunctions( );

    auto* logger = new QOpenGLDebugLogger( this );
    logger->initialize( );

    connect(
      logger , &QOpenGLDebugLogger::messageLogged ,
      [ ]( const QOpenGLDebugMessage& message )
      {
        if ( message.severity( ) <= QOpenGLDebugMessage::MediumSeverity )
          qDebug( ) << message;
      }
    );

    logger->startLogging( );

    glEnable( GL_DEPTH_TEST );
    glClearColor( float( _currentClearColor.red( )) / 255.0f ,
                  float( _currentClearColor.green( )) / 255.0f ,
                  float( _currentClearColor.blue( )) / 255.0f ,
                  float( _currentClearColor.alpha( )) / 255.0f );
    glPolygonMode( GL_FRONT_AND_BACK , GL_FILL );
    glEnable( GL_CULL_FACE );

    _then = std::chrono::system_clock::now( );
    _lastFrame = std::chrono::system_clock::now( );

    QOpenGLWidget::initializeGL( );
    visimpl::GlewInitializer::init( );

    const GLubyte* vendor = glGetString( GL_VENDOR ); // Returns the vendor
    const GLubyte* renderer = glGetString(
      GL_RENDERER ); // Returns a hint to the model
    const GLubyte* version = glGetString( GL_VERSION );
    const GLubyte* shadingVer = glGetString( GL_SHADING_LANGUAGE_VERSION );

    _screenPlaneShader = new reto::ShaderProgram( );
    _screenPlaneShader->loadVertexShaderFromText( SHADER_SCREEN_VERTEX );
    _screenPlaneShader->loadFragmentShaderFromText( SHADER_SCREEN_FRAGMENT );
    _screenPlaneShader->create( );
    _screenPlaneShader->link( );
    _screenPlaneShader->autocatching( true );
    _screenPlaneShader->use( );
    _screenPlaneShader->sendUniformi( "accumulation" , 0 );
    _screenPlaneShader->sendUniformi( "reveal" , 1 );


    std::cout << "OpenGL Hardware: " << vendor << " (" << renderer << ")"
              << std::endl;
    std::cout << "OpenGL Version: " << version << " (shading ver. "
              << shadingVer << ")" << std::endl;
  }

  void OpenGLWidget::_initRenderToTexture( void )
  {

    // Generate the OIR framebuffer

    _gl.glGenFramebuffers( 1 , &_weightFrameBuffer );
    _gl.glBindFramebuffer( GL_FRAMEBUFFER , _weightFrameBuffer );

    glGenTextures( 1 , &_accumulationTexture );
    glBindTexture( GL_TEXTURE_2D , _accumulationTexture );
    glTexImage2D( GL_TEXTURE_2D , 0 , GL_RGBA32F , width( ) , height( ) , 0 ,
                  GL_RGBA , GL_FLOAT , nullptr );
    glTexParameteri( GL_TEXTURE_2D , GL_TEXTURE_MIN_FILTER , GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D , GL_TEXTURE_MAG_FILTER , GL_LINEAR );

    glGenTextures( 1 , &_revealTexture );
    glBindTexture( GL_TEXTURE_2D , _revealTexture );
    glTexImage2D( GL_TEXTURE_2D , 0 , GL_R8 , width( ) , height( ) , 0 ,
                  GL_RED , GL_FLOAT , nullptr );
    glTexParameteri( GL_TEXTURE_2D , GL_TEXTURE_MIN_FILTER , GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D , GL_TEXTURE_MAG_FILTER , GL_LINEAR );

    _gl.glFramebufferTexture2D( GL_FRAMEBUFFER , GL_COLOR_ATTACHMENT0 ,
                                GL_TEXTURE_2D , _accumulationTexture , 0 );
    _gl.glFramebufferTexture2D( GL_FRAMEBUFFER , GL_COLOR_ATTACHMENT1 ,
                                GL_TEXTURE_2D , _revealTexture , 0 );

    const GLenum transparentDrawBuffers[] = { GL_COLOR_ATTACHMENT0 ,
                                              GL_COLOR_ATTACHMENT1 };
    _gl.glDrawBuffers( 2 , transparentDrawBuffers );

    if ( _gl.glCheckFramebufferStatus( GL_FRAMEBUFFER ) !=
         GL_FRAMEBUFFER_COMPLETE )
      std::cerr << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!"
                << std::endl;

    _gl.glBindFramebuffer( GL_FRAMEBUFFER , defaultFramebufferObject( ));

    float quadVertices[] = {
      // positions
      -1.0f , -1.0f , 0.0f ,
      1.0f , -1.0f , 0.0f ,
      1.0f , 1.0f , 0.0f ,

      1.0f , 1.0f , 0.0f ,
      -1.0f , 1.0f , 0.0f ,
      -1.0f , -1.0f , 0.0f
    };

    // quad VAO
    unsigned int quadVBO;
    _gl.glGenVertexArrays( 1 , &_quadVAO );
    _gl.glGenBuffers( 1 , &quadVBO );
    _gl.glBindVertexArray( _quadVAO );
    _gl.glBindBuffer( GL_ARRAY_BUFFER , quadVBO );
    _gl.glBufferData( GL_ARRAY_BUFFER , sizeof( quadVertices ) , quadVertices ,
                      GL_STATIC_DRAW );
    _gl.glEnableVertexAttribArray( 0 );
    _gl.glVertexAttribPointer( 0 , 3 , GL_FLOAT , GL_FALSE ,
                               3 * sizeof( float ) ,
                               ( void* ) 0 );
    _gl.glBindVertexArray( 0 );
  }

  void OpenGLWidget::_configureSimulationFrame( void )
  {
    if ( !_player || !_player->isPlaying( ))
      return;

    if ( _backtrace )
    {
      _backtraceSimulation( );
      _backtrace = false;
    }

    _player->Frame( );

    _domainManager.processInput( _player->spikesNow( ) , false );
  }

  void OpenGLWidget::_configurePreviousStep( void )
  {
    if ( !_player )
      return;

    _sbsBeginTime = _sbsFirstStep ?
                    _player->currentTime( ) :
                    _sbsPlaying ? _sbsBeginTime : _sbsEndTime;

    _sbsBeginTime = std::max( static_cast<double>(_player->startTime( )) ,
                              _sbsBeginTime -
                              static_cast<double>(_player->deltaTime( )));

    _sbsEndTime = _sbsBeginTime + _player->deltaTime( );

    if ( _sbsBeginTime < _sbsEndTime )
    {
      _player->GoTo( _sbsBeginTime );

      _backtraceSimulation( );

      _sbsStepSpikes = _player->spikesBetween( _sbsBeginTime , _sbsEndTime );

      _sbsCurrentTime = _sbsBeginTime;
      _sbsCurrentSpike = _sbsStepSpikes.first;

      _sbsFirstStep = false;
      _sbsPlaying = true;
    }
  }

  void OpenGLWidget::_configureStepByStep( void )
  {
    if ( !_player || !_player->isPlaying( ))
      return;

    _sbsBeginTime = _sbsFirstStep ? _player->currentTime( ) : _sbsEndTime;

    _sbsEndTime = _sbsBeginTime + _player->deltaTime( );

    if ( _sbsPlaying )
    {
      _player->GoTo( _sbsBeginTime );

      _backtraceSimulation( );
    }

    _sbsStepSpikes = _player->spikesBetween( _sbsBeginTime , _sbsEndTime );

    _sbsCurrentTime = _sbsBeginTime;
    _sbsCurrentSpike = _sbsStepSpikes.first;

    _sbsFirstStep = false;
    _sbsPlaying = true;
  }

  void OpenGLWidget::_configureStepByStepFrame( double elapsedRenderTime )
  {
    _sbsCurrentRenderDelta = 0;

    if ( _sbsPlaying && _sbsCurrentTime >= _sbsEndTime )
    {
      _sbsPlaying = false;
      _player->GoTo( _sbsEndTime );
      _player->Pause( );
      emit stepCompleted( );
    }

    if ( _sbsPlaying )
    {
      double diff = _sbsEndTime - _sbsCurrentTime;
      double renderDelta =
        elapsedRenderTime * _sbsInvTimePerStep * _player->deltaTime( );
      _sbsCurrentRenderDelta = std::min( diff , renderDelta );

      double nextTime = _sbsCurrentTime + _sbsCurrentRenderDelta;

      auto spikeIt = _sbsCurrentSpike;
      while ( spikeIt->first < nextTime &&
              spikeIt - _sbsStepSpikes.first <
              _sbsStepSpikes.second - _sbsStepSpikes.first )
      {
        ++spikeIt;
      }

      if ( spikeIt != _sbsCurrentSpike )
      {
        simil::SpikesCRange frameSpikes = std::make_pair( _sbsCurrentSpike ,
                                                          spikeIt );
        _domainManager.processInput( frameSpikes , false );
      }

      _sbsCurrentTime = nextTime;
      _sbsCurrentSpike = spikeIt;
    }
  }

  void OpenGLWidget::_backtraceSimulation( void )
  {
    if ( !_player ) return;

    const float endTime = _player->currentTime( );
    const float startTime = std::max( 0.0f ,
                                      endTime - _domainManager.getDecay( ));
    if ( startTime < endTime )
    {
      const auto context = _player->spikesBetween( startTime , endTime );

      _domainManager.setTime( endTime );
      if ( context.first != context.second )
        _domainManager.processInput( context , true );
    }
  }

  void OpenGLWidget::changeShader( int shaderIndex )
  {
    if ( shaderIndex == 0 )
      _domainManager.applyDefaultShader( );
    else if ( shaderIndex == 1 )
      _domainManager.applySolidShader( );
  }

  void OpenGLWidget::createParticleSystem( )
  {
    makeCurrent( );
    std::cout << "Loaded default shaders." << std::endl;
    _updateData( );

    if ( !_particleSystemInitialized )
    {
      _shaderClippingPlanes = new reto::ShaderProgram( );
      _shaderClippingPlanes->loadVertexShaderFromText(
        visimpl::SHADER_PLANE_VERTEX );
      _shaderClippingPlanes->loadFragmentShaderFromText(
        visimpl::SHADER_PLANE_FRAGMENT );
      _shaderClippingPlanes->compileAndLink( );
      _shaderClippingPlanes->autocatching( );

      _domainManager.initRenderers(
        _clippingPlaneLeft , _clippingPlaneRight , _camera );
    }

    if ( _player )
    {
      _domainManager.setSelection( _player->gids( ) , _gidPositions );

#ifdef SIMIL_USE_BRION

      _domainManager.initAttributeData( _player->gids( ) ,
                                        _player
                                        ? _player->data( )->blueConfig( )
                                        : nullptr );
      _domainManager.selectAttribute( _colorPalette.colors( ) , _gidPositions ,
                                      tNeuronAttributes::T_TYPE_MORPHO );
#endif

    }

    _domainManager.setMode( VisualMode::Selection );

    updateCameraBoundingBox( true );

    _initClippingPlanes( );

    _particleSystemInitialized = true;
  }

  void OpenGLWidget::_paintParticles( void )
  {
    glm::vec3 cameraPosition( _camera->position( )[ 0 ] ,
                              _camera->position( )[ 1 ] ,
                              _camera->position( )[ 2 ] );

    if ( _lastCameraPosition != cameraPosition )
    {
      _lastCameraPosition = cameraPosition;
    }

    _domainManager.draw( );
  }

  void OpenGLWidget::_paintPlanes( void )
  {
    if ( _clipping && _paintClippingPlanes )
    {
      _planeLeft.render( _shaderClippingPlanes );
      _planeRight.render( _shaderClippingPlanes );
    }
  }

  void OpenGLWidget::_resolveFlagsOperations( void )
  {
    if ( _flagNewData )
      _updateNewData( );

    if ( _flagUpdateSelection )
      _updateSelection( );

    if ( _flagAttribChange )
      _attributeChange( );

    if ( _flagUpdateAttributes )
      _updateAttributes( );
  }

  void OpenGLWidget::paintGL( void )
  {
    std::chrono::time_point< std::chrono::system_clock > now =
      std::chrono::system_clock::now( );

    const unsigned int elapsedMicroseconds =
      std::chrono::duration_cast< std::chrono::microseconds >
        ( now - _lastFrame ).count( );

    _lastFrame = now;

    _deltaTime = elapsedMicroseconds * 0.000001;

    if ( _player && _player->isPlaying( ))
    {
      _elapsedTimeSimAcc += elapsedMicroseconds;
      _elapsedTimeRenderAcc += elapsedMicroseconds;
      _elapsedTimeSliderAcc += elapsedMicroseconds;
    }
    _frameCount++;
    glDepthMask( GL_TRUE );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glDisable( GL_BLEND );
    glEnable( GL_DEPTH_TEST );
    glEnable( GL_CULL_FACE );

    _resolveFlagsOperations( );

    if ( _paint )
    {
      _camera->anim( );

      if ( _player && _player->isPlaying( ))
      {
        switch ( _playbackMode )
        {
          // Continuous mode (Default)
          case TPlaybackMode::CONTINUOUS:
            if ( _elapsedTimeSimAcc >= _simPeriodMicroseconds )
            {
              _configureSimulationFrame( );
              _updateEventLabelsVisibility( );

              _elapsedTimeSimAcc = 0.0f;
            }
            break;
            // Step by step mode
          case TPlaybackMode::STEP_BY_STEP:
            if ( _sbsPrevStep )
            {
              _configurePreviousStep( );
              _sbsPrevStep = false;
            }
            else if ( _sbsNextStep )
            {
              _configureStepByStep( );
              _sbsNextStep = false;
            }
            break;
          default:
            break;
        }

        if ( _elapsedTimeRenderAcc >= _renderPeriodMicroseconds )
        {
          double renderDelta = 0;

          switch ( _playbackMode )
          {
            case TPlaybackMode::CONTINUOUS:
              renderDelta =
                _elapsedTimeRenderAcc * _simTimePerSecond * 0.000001;
              break;
            case TPlaybackMode::STEP_BY_STEP:
              _configureStepByStepFrame( _elapsedTimeRenderAcc * 0.000001 );
              renderDelta = _sbsCurrentRenderDelta;
              break;
            default:
              renderDelta = 0;
          }

          if ( std::isnan( renderDelta )) renderDelta = 0;

          _updateParticles( renderDelta );
          _elapsedTimeRenderAcc = 0.0f;
        } // elapsed > render period

      } // if player && player->isPlaying

      glViewport( 0 , 0 , width( ) , height( ));

      if ( _domainManager.isAccumulativeModeEnabled( ))
      {
        _gl.glBindFramebuffer( GL_FRAMEBUFFER , defaultFramebufferObject( ));
        _gl.glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        _gl.glDisable( GL_DEPTH_TEST );
        _gl.glDepthMask( GL_FALSE );
        _gl.glEnable( GL_BLEND );
        _gl.glBlendFunc( GL_SRC_ALPHA , GL_ONE_MINUS_CONSTANT_ALPHA );
        _gl.glBlendEquation( GL_FUNC_ADD );
        _paintPlanes( );
        _paintParticles( );
      }
      else
      {
        _gl.glBindFramebuffer( GL_FRAMEBUFFER , _weightFrameBuffer );
        _gl.glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        _gl.glEnable( GL_DEPTH_TEST );


        _gl.glDepthMask( GL_FALSE );
        _gl.glEnable( GL_BLEND );
        _gl.glBlendFunci( 0 , GL_ONE , GL_ONE );
        _gl.glBlendFunci( 1 , GL_ZERO , GL_ONE_MINUS_SRC_COLOR );
        _gl.glBlendEquation( GL_FUNC_ADD );

        glm::vec4 zeroFillerVec( 0.0f );
        glm::vec4 oneFillerVec( 1.0f );
        _gl.glClearBufferfv( GL_COLOR , 0 , &zeroFillerVec[ 0 ] );
        _gl.glClearBufferfv( GL_COLOR , 1 , &oneFillerVec[ 0 ] );

        _paintPlanes( );
        _paintParticles( );

        // Perform blend

        _gl.glDepthFunc( GL_ALWAYS );
        _gl.glEnable( GL_BLEND );
        _gl.glBlendFunc( GL_SRC_ALPHA , GL_ONE_MINUS_SRC_ALPHA );

        _gl.glBindFramebuffer( GL_FRAMEBUFFER , defaultFramebufferObject( ));
        _gl.glClear(
          GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );

        _screenPlaneShader->use( );
        _gl.glActiveTexture( GL_TEXTURE0 );
        _gl.glBindTexture( GL_TEXTURE_2D , _accumulationTexture );
        _gl.glActiveTexture( GL_TEXTURE1 );
        _gl.glBindTexture( GL_TEXTURE_2D , _revealTexture );
        _gl.glBindVertexArray( _quadVAO );
        _gl.glDrawArrays( GL_TRIANGLES , 0 , 6 );
      }

    }

    if ( _player && _elapsedTimeSliderAcc > _sliderUpdatePeriodMicroseconds )
    {
      _elapsedTimeSliderAcc = 0.0f;

#ifdef VISIMPL_USE_ZEROEQ
      _player->sendCurrentTimestamp( );
#endif

      emit updateSlider( _player->GetRelativeTime( ));

      if ( _showCurrentTime )
      {
        _labelCurrentTime->setText(
          tr( "t=" ) + QString::number( _player->currentTime( )));
      }
    }

#define FRAMES_PAINTED_TO_MEASURE_FPS 10
    if ( _showFps && _frameCount >= FRAMES_PAINTED_TO_MEASURE_FPS )
    {
      auto duration = std::chrono::duration_cast< std::chrono::milliseconds >(
        now - _then );
      _then = now;

      MainWindow* mainWindow = dynamic_cast< MainWindow* >( parent( ));
      if ( mainWindow )
      {
        const unsigned int ellapsedMiliseconds = duration.count( );
        const auto ratioMS =
          static_cast<float>(_frameCount) / ellapsedMiliseconds;
        if ( !std::isnan( ratioMS ))
        {
          const unsigned int fps = roundf( 1000.0f * ratioMS );

          if ( _showFps )
          {
            _fpsLabel->setText( QString::number( fps ) + QString( " FPS" ));
          }
        }
      }

      _frameCount = 0;
    }

    if ( _idleUpdate && _player )
     update( );
  }

  void
  OpenGLWidget::setSelectedGIDs( const std::unordered_set< uint32_t >& gids )
  {
    if ( gids.size( ) > 0 )
    {
      _selectedGIDs = gids;
      _pendingSelection = true;

      setUpdateSelection( );
    }
  }

  void OpenGLWidget::setUpdateSelection( void )
  {
    _flagUpdateSelection = true;
  }

  void OpenGLWidget::setUpdateAttributes( void )
  {
    _flagAttribChange = true;
  }

  void OpenGLWidget::selectAttrib( int newAttrib )
  {
    if (( newAttrib < 0 || newAttrib >= static_cast<int>(T_TYPE_UNDEFINED) ||
          _domainManager.getMode( ) != VisualMode::Attribute ))
      return;

    _newAttrib = static_cast<tNeuronAttributes>(newAttrib);
    _flagAttribChange = true;
  }

  void OpenGLWidget::_modeChange( void )
  {
    _domainManager.setMode( _newMode );

    if ( _domainManager.getMode( ) == VisualMode::Attribute )
      emit attributeStatsComputed( );
  }

  void OpenGLWidget::_attributeChange( void )
  {
    _domainManager.selectAttribute(
      _colorPalette.colors( ) , _gidPositions , _newAttrib );

    _currentAttrib = _newAttrib;

    _flagAttribChange = false;

    emit attributeStatsComputed( );
  }

  void OpenGLWidget::_updateSelection( void )
  {
    if ( !_player ) return;

    updateCameraBoundingBox( );

    _flagUpdateSelection = false;
  }

  void OpenGLWidget::_updateAttributes( void )
  {
    updateCameraBoundingBox( );
    _flagUpdateAttributes = false;
  }

  void OpenGLWidget::updateData( )
  {
    _flagNewData = true;
  }

  bool OpenGLWidget::_updateData( bool force )
  {
    if ( !_player )
    {
      _gidPositions.clear( );
      _boundingBoxHome = tBoundingBox{ vec3{ 0 , 0 , 0 } , vec3{ 0 , 0 , 0 }};
      return false;
    }

    const auto& positions = _player->positions( );

    // assumed positions doesn't change so if equal the network didn't change.
    if ( !force && positions.size( ) == _gidPositions.size( )) return false;

    _gidPositions.clear( );
    _gidPositions.reserve( positions.size( ));

    vec3 bbmin = vec3( );
    vec3 bbmax = vec3( );
    auto gidit = _player->gids( ).cbegin( );
    auto insertElement = [ & ]( const vmml::Vector3f& v )
    {
      const vec3 position( v.x( ) * _scaleFactor.x ,
                           v.y( ) * _scaleFactor.y ,
                           v.z( ) * _scaleFactor.z );

      _gidPositions.insert( std::make_pair( *gidit , position ));
      if ( gidit == _player->gids( ).cbegin( ))
      {
        bbmin = bbmax = position;
      }
      else
      {
        auto x = std::min( bbmin.x , position.x );
        auto y = std::min( bbmin.y , position.y );
        auto z = std::min( bbmin.z , position.z );
        bbmin = vec3{ x , y , z };

        x = std::max( bbmax.x , position.x );
        y = std::max( bbmax.y , position.y );
        z = std::max( bbmax.z , position.z );
        bbmax = vec3{ x , y , z };
      }
      ++gidit;
    };
    std::for_each( positions.cbegin( ) , positions.cend( ) , insertElement );

    _boundingBoxHome = tBoundingBox{ bbmin , bbmax };

    return true;
  }

  void OpenGLWidget::_updateNewData( void )
  {
    _flagNewData = false;
    if ( !_updateData( )) return;
    _domainManager.setSelection( _player->gids( ) , _gidPositions );
  }

  void OpenGLWidget::setMode( int mode )
  {
    if ( mode < 0 || ( mode >= static_cast<int>(VisualMode::Undefined)))
      return;

    _newMode = static_cast<VisualMode>(mode);
    _modeChange( );
  }

  void OpenGLWidget::showInactive( bool /*state*/ )
  {
    // TODO _domainManager.showInactive( state );
  }

  void OpenGLWidget::clearSelection( void )
  {
    _selectedGIDs.clear( );
    _flagUpdateSelection = true;
  }

  void OpenGLWidget::home( void )
  {
    if ( _homePosition.isEmpty( ))
    {
      _focusOn( _domainManager.getBoundingBox() );
      _homePosition = cameraPosition( ).toString( );
    }

    setCameraPosition( _homePosition );
  }

  void OpenGLWidget::updateCameraBoundingBox( bool setBoundingBox )
  {
    if ( _gidPositions.empty( )) return;

    const auto boundingBox = _domainManager.getBoundingBox( );

    const glm::vec3 MAX{ std::numeric_limits< float >::max( ) ,
                         std::numeric_limits< float >::max( ) ,
                         std::numeric_limits< float >::max( ) };
    const glm::vec3 MIN{ std::numeric_limits< float >::min( ) ,
                         std::numeric_limits< float >::min( ) ,
                         std::numeric_limits< float >::min( ) };

    if ( boundingBox.first != MAX && boundingBox.second != MIN )
    {
      if ( setBoundingBox )
        _boundingBoxHome = boundingBox;

      _focusOn( boundingBox );
    }
  }

  void OpenGLWidget::_focusOn( const tBoundingBox& boundingBox )
  {
    const glm::vec3 center = ( boundingBox.first + boundingBox.second ) * 0.5f;
    const float side = glm::length( boundingBox.second - center ) / 1.75;
    const float radius = side / std::tan( _camera->camera( )->fieldOfView( ));

    _camera->position( Eigen::Vector3f( center.x , center.y , center.z ));
    _camera->radius( radius );
  }

  void OpenGLWidget::showEventsActivityLabels( bool show )
  {
    auto updateWidget = [ show ]( visimpl::OpenGLWidget::EventLabel& container )
    {
      container.upperWidget->setVisible( show );
      container.upperWidget->update( );
    };
    std::for_each( _eventLabels.begin( ) , _eventLabels.end( ) , updateWidget );
  }

  void OpenGLWidget::showCurrentTimeLabel( bool show )
  {
    _labelCurrentTime->setVisible( show );
    _labelCurrentTime->update( );
  }

  void OpenGLWidget::circuitScaleFactor( vec3 scale_ , bool update )
  {
    _scaleFactor = scale_;

    _scaleFactorExternal = true;

    _homePosition.clear(); // reset home position to force re-computation.

    if ( update && _player )
    {
      _updateData( true );
      _domainManager.setSelection( _player->gids( ) , _gidPositions );
      _focusOn( _domainManager.getBoundingBox( ));
    }
  }

  vec3 OpenGLWidget::circuitScaleFactor( void ) const
  {
    return _scaleFactor;
  }

  void OpenGLWidget::_updateParticles( float renderDelta )
  {
    if ( _player && ( _player->isPlaying( ) || _firstFrame ))
    {
      _domainManager.addTime( renderDelta , _player->endTime( ));
      _firstFrame = false;
    }
  }

  void OpenGLWidget::_updateEventLabelsVisibility( void )
  {
    if ( !_player ) return;

    std::vector< bool > visibility = _activeEventsAt( _player->currentTime( ));

    unsigned int counter = 0;
    for ( auto showLabel: visibility )
    {
      EventLabel& labelObjects = _eventLabels[ counter ];

      auto effect = new QGraphicsOpacityEffect( );
      effect->setOpacity( showLabel ? 1.0 : 0.5 );
      labelObjects.upperWidget->setGraphicsEffect( effect );
      labelObjects.upperWidget->update( );
      ++counter;
    }
  }

  std::vector< bool > OpenGLWidget::_activeEventsAt( float time )
  {
    std::vector< bool > result( _eventLabels.size( ) , false );

    if ( _player )
    {
      const float totalTime = _player->endTime( ) - _player->startTime( );
      const double perc = time / totalTime;

      unsigned int counter = 0;
      for ( auto event: _eventsActivation )
      {
        unsigned int position = perc * event.size( );
        result[ counter ] = event[ position ];

        ++counter;
      }
    }

    return result;
  }

  void OpenGLWidget::resizeGL( int w , int h )
  {
    _camera->windowSize( w , h );
    glViewport( 0 , 0 , w , h );

    //if ( _pickRenderer )
    //{
    //  _pickRenderer->setWindowSize( w , h );
    //}

    if ( _accumulationTexture != 0 )
    {
      // Resize MSAA buffers
      _gl.glBindTexture( GL_TEXTURE_2D , _accumulationTexture );
      _gl.glTexImage2D( GL_TEXTURE_2D , 0 , GL_RGBA32F , width( ) , height( ) ,
                        0 ,
                        GL_RGBA , GL_FLOAT , nullptr );

      _gl.glBindTexture( GL_TEXTURE_2D , _revealTexture );
      _gl.glTexImage2D( GL_TEXTURE_2D , 0 , GL_R8 , width( ) , height( ) , 0 ,
                        GL_RED , GL_FLOAT , nullptr );

      _gl.glBindTexture( GL_TEXTURE_2D_MULTISAMPLE , 0 );
      _gl.glBindTexture( GL_TEXTURE_2D , 0 );
      //_gl.glBindRenderbuffer( GL_RENDERBUFFER , defaultFramebufferObject( ));
    }
  }

  void OpenGLWidget::_initClippingPlanes( void )
  {
    _planePosLeft.resize( 4 , Eigen::Vector3f::Zero( ));
    _planePosRight.resize( 4 , Eigen::Vector3f::Zero( ));

    _planeRotation = Eigen::Matrix4f::Identity( );

    _planeLeft.init( _camera->camera( ));
    _planeRight.init( _camera->camera( ));

    _genPlanesFromBoundingBox( );
  }

  void OpenGLWidget::_genPlanesFromBoundingBox( void )
  {
    auto currentBoundingBox = _domainManager.getBoundingBox( );

    _planesCenter =
      glmToEigen( currentBoundingBox.first + currentBoundingBox.second ) * 0.5f;

    _planeDistance =
      std::abs( currentBoundingBox.second.x - currentBoundingBox.first.x ) + 1;
    _planeHeight = std::abs(
      currentBoundingBox.second.y - currentBoundingBox.first.y );
    _planeWidth = std::abs(
      currentBoundingBox.second.z - currentBoundingBox.first.z );

    _genPlanesFromParameters( );
  }

  void OpenGLWidget::_genPlanesFromParameters( void )
  {
    glm::vec3 offset =
      glm::vec3( _planeDistance , _planeHeight , _planeWidth ) * 0.5f;

    evec3 centerLeft = _planesCenter;
    evec3 centerRight = _planesCenter;
    centerLeft.x( ) -= offset.x;
    centerRight.x( ) += offset.x;

    _planePosLeft[ 0 ] = _planePosLeft[ 1 ] =
    _planePosLeft[ 2 ] = _planePosLeft[ 3 ] = centerLeft;

    _planePosRight[ 0 ] = _planePosRight[ 1 ] =
    _planePosRight[ 2 ] = _planePosRight[ 3 ] = centerRight;

    _planePosLeft[ 0 ] += Eigen::Vector3f( 0 , offset.y , -offset.z );
    _planePosLeft[ 1 ] += Eigen::Vector3f( 0 , -offset.y , -offset.z );
    _planePosLeft[ 2 ] += Eigen::Vector3f( 0 , -offset.y , offset.z );
    _planePosLeft[ 3 ] += Eigen::Vector3f( 0 , offset.y , offset.z );

    _planePosRight[ 0 ] += Eigen::Vector3f( 0 , offset.y , -offset.z );
    _planePosRight[ 1 ] += Eigen::Vector3f( 0 , -offset.y , -offset.z );
    _planePosRight[ 2 ] += Eigen::Vector3f( 0 , -offset.y , offset.z );
    _planePosRight[ 3 ] += Eigen::Vector3f( 0 , offset.y , offset.z );

    _updatePlanes( );
  }

  static Eigen::Vector3f transform( const Eigen::Vector3f& position ,
                                    const Eigen::Vector3f& displacement ,
                                    const Eigen::Matrix4f& rotation )
  {
    auto disp = vec3ToVec4( displacement );
    return vec4ToVec3(( rotation * ( vec3ToVec4( position ) - disp )) + disp );
  }

  void OpenGLWidget::_updatePlanes( void )
  {
    evec3 center;
    evec3 centerLeft;
    evec3 centerRight;
    center = centerLeft = centerRight = _planesCenter;

    unsigned int pointsNumber = 4;

    std::vector< Eigen::Vector3f > transformedPoints( pointsNumber * 2 );

    for ( unsigned int i = 0; i < pointsNumber; ++i )
    {
      transformedPoints[ i ] =
        transform( _planePosLeft[ i ] , center , _planeRotation );

      transformedPoints[ i + pointsNumber ] =
        transform( _planePosRight[ i ] , center , _planeRotation );
    }

    _planeLeft.points( transformedPoints[ 0 ] , transformedPoints[ 1 ] ,
                       transformedPoints[ 2 ] , transformedPoints[ 3 ] );

    _planeRight.points( transformedPoints[ 4 ] , transformedPoints[ 5 ] ,
                        transformedPoints[ 6 ] , transformedPoints[ 7 ] );


    float offsetX = _planeDistance * 0.5;
    centerLeft.x( ) -= offsetX;
    centerRight.x( ) += offsetX;

    center = transform( center , center , _planeRotation );
    centerLeft = transform( centerLeft , center , _planeRotation );
    centerRight = transform( centerRight , center , _planeRotation );

    _planeNormalLeft = ( center - centerLeft ).normalized( );
    _planeNormalRight = ( center - centerRight ).normalized( );

    _clippingPlaneLeft->setEquationByPointAndNormal(
      centerLeft , _planeNormalLeft );
    _clippingPlaneRight->setEquationByPointAndNormal(
      centerRight , _planeNormalRight );
  }

  void OpenGLWidget::clippingPlanes( bool active )
  {
    _clipping = active;
    _domainManager.enableClipping( active );
    if ( _clipping ) _updatePlanes( );
  }

  void OpenGLWidget::paintClippingPlanes( int paint_ )
  {
    _paintClippingPlanes = paint_;
  }

  void OpenGLWidget::toggleClippingPlanes( void )
  {
    clippingPlanes( !_clipping );
  }

  void OpenGLWidget::clippingPlanesReset( void )
  {
    _planeRotation = emat4::Identity( );

    _genPlanesFromBoundingBox( );
  }

  void OpenGLWidget::clippingPlanesHeight( float height_ )
  {
    _planeHeight = height_;

    _genPlanesFromParameters( );
  }

  float OpenGLWidget::clippingPlanesHeight( void )
  {
    return _planeHeight;
  }

  void OpenGLWidget::clippingPlanesWidth( float width_ )
  {
    _planeWidth = width_;

    _genPlanesFromParameters( );
  }

  float OpenGLWidget::clippingPlanesWidth( void )
  {
    return _planeWidth;
  }

  void OpenGLWidget::clippingPlanesDistance( float distance_ )
  {
    _planeDistance = distance_;

    _genPlanesFromParameters( );
  }

  float OpenGLWidget::clippingPlanesDistance( void )
  {
    return _planeDistance;
  }

  void OpenGLWidget::clippingPlanesColor( const QColor& color_ )
  {
    _planesColor = evec4( color_.red( ) * invRGBInt ,
                          color_.green( ) * invRGBInt ,
                          color_.blue( ) * invRGBInt ,
                          1.0 );

    _planeLeft.color( _planesColor );
    _planeRight.color( _planesColor );

    emit planesColorChanged( color_ );
  }

  QColor OpenGLWidget::clippingPlanesColor( void )
  {
    return QColor( _planesColor.x( ) * 255 ,
                   _planesColor.y( ) * 255 ,
                   _planesColor.z( ) * 255 ,
                   _planesColor.w( ) * 255 );
  }

  void OpenGLWidget::_rotatePlanes( float yaw_ , float pitch_ )
  {
    const float sinYaw = sin( yaw_ );
    const float cosYaw = cos( yaw_ );
    const float sinPitch = sin( pitch_ );
    const float cosPitch = cos( pitch_ );

    Eigen::Matrix4f rYaw;
    rYaw << cosYaw , 0.0f , sinYaw , 0.0f ,
      0.0f , 1.0f , 0.0f , 0.0f ,
      -sinYaw , 0.0f , cosYaw , 0.0f ,
      0.0f , 0.0f , 0.0f , 1.0f;

    Eigen::Matrix4f rPitch;
    rPitch << 1.0f , 0.0f , 0.0f , 0.0f ,
      0.0f , cosPitch , -sinPitch , 0.0f ,
      0.0f , sinPitch , cosPitch , 0.0f ,
      0.0f , 0.0f , 0.0f , 1.0f;

    Eigen::Matrix4f rot;
    rot = rPitch * rYaw;

    _planeRotation = rot * _planeRotation;

    _updatePlanes( );
  }

  GIDVec OpenGLWidget::getPlanesContainedElements( void ) const
  {
    GIDVec result;

    // Project elements
    evec3 normal = -_planeNormalLeft;
    normal.normalize( );

    auto positions = _gidPositions;

    result.reserve( positions.size( ));

    float distance = 0.0f;

    for ( auto neuronPos: positions )
    {
      distance = normal.dot( _planeLeft.points( )[ 0 ] ) -
                 normal.dot( glmToEigen( neuronPos.second ));

      if ( distance > 0.0f && distance <= _planeDistance )
      {
        result.emplace_back( neuronPos.first );
      }
    }

    result.shrink_to_fit( );

    return result;
  }

  void OpenGLWidget::mousePressEvent( QMouseEvent* event_ )
  {
    if ( event_->button( ) == Qt::LeftButton )
    {
      if ( event_->modifiers( ) == ( Qt::SHIFT | Qt::CTRL ) && _clipping )
      {
        _translationPlanes = true;
        _mouseX = event_->x( );
        _mouseY = event_->y( );
      }
      else if ( event_->modifiers( ) == Qt::CTRL )
      {
        _pickingPosition = event_->pos( );

        {
          _translation = true;
          _mouseX = event_->x( );
          _mouseY = event_->y( );
        }
      }
      else if ( event_->modifiers( ) == Qt::SHIFT && _clipping )
      {
        _rotationPlanes = true;
        _mouseX = event_->x( );
        _mouseY = event_->y( );
      }
      else
      {
        _rotation = true;
        _mouseX = event_->x( );
        _mouseY = event_->y( );
      }
    }

    update( );
  }

  void OpenGLWidget::mouseReleaseEvent( QMouseEvent* event_ )
  {
    if ( event_->modifiers( ) == Qt::CTRL )
    {
      if ( _pickingPosition == event_->pos( ))
      {
        _pickingPosition.setY( height( ) - _pickingPosition.y( ));
      }

      _translation = false;
      _translationPlanes = false;
    }

    if ( event_->button( ) == Qt::LeftButton )
    {
      _rotation = false;
      _rotationPlanes = false;
    }

    update( );
  }

  void OpenGLWidget::mouseMoveEvent( QMouseEvent* event_ )
  {
    const float diffX = event_->x( ) - _mouseX;
    const float diffY = event_->y( ) - _mouseY;

    auto updateLastEventCoords = [ this ]( const QMouseEvent* e )
    {
      _mouseX = e->x( );
      _mouseY = e->y( );
    };

    if ( _rotation )
    {
      _camera->rotate(
        Eigen::Vector3f( diffX * ROTATION_FACTOR , diffY * ROTATION_FACTOR ,
                         0.0f ));
      updateLastEventCoords( event_ );
    }

    if ( _rotationPlanes && _clipping )
    {
      _rotatePlanes( diffX * ROTATION_FACTOR , diffY * ROTATION_FACTOR );
      updateLastEventCoords( event_ );
    }

    if ( _translation )
    {
      const float xDis = diffX * TRANSLATION_FACTOR * _camera->radius( );
      const float yDis = diffY * TRANSLATION_FACTOR * _camera->radius( );

      _camera->localTranslate( Eigen::Vector3f( -xDis , yDis , 0.0f ));
      updateLastEventCoords( event_ );
    }

    if ( _translationPlanes )
    {
      const float xDis = diffX * TRANSLATION_FACTOR * _camera->radius( );
      const float yDis = diffY * TRANSLATION_FACTOR * _camera->radius( );
      const evec3 displacement( xDis , -yDis , 0 );

      _planesCenter += _camera->rotation( ).transpose( ) * displacement;

      updateLastEventCoords( event_ );
      _genPlanesFromParameters( );
    }

    update( );
  }

  void OpenGLWidget::wheelEvent( QWheelEvent* event_ )
  {
    int delta = event_->angleDelta( ).y( );


    if ( delta > 0 )
      _camera->radius( _camera->radius( ) / ZOOM_FACTOR );
    else
      _camera->radius( _camera->radius( ) * ZOOM_FACTOR );

    update( );
  }

  void OpenGLWidget::setPlayer( simil::SpikesPlayer* p ,
                                const simil::TDataType type )
  {
    // Resets camera position
    setCameraPosition( CameraPosition( INITIAL_CAMERA_POSITION ));
    _homePosition.clear( );

    if ( _player )
    {
      // TODO: clear data?
    }

    _deltaTime = DEFAULT_DELTA_TIME;
    _player = p;

    InitialConfig config;
    switch ( type )
    {
      case simil::TBlueConfig:
        config = _initialConfigSimBlueConfig;
        break;
      case simil::THDF5:
        config = _initialConfigSimH5;
        break;
      case simil::TCSV:
        config = _initialConfigSimCSV;
        break;
      case simil::TREST:
        config = _initialConfigSimREST;
        break;
      default:
        break;
    }

    if ( !_scaleFactorExternal )
    {
      const auto scale = std::get< T_SCALE >( config );
      _scaleFactor = vec3( scale , scale , scale );
    }

    std::cout << "Using scale factor of " << _scaleFactor.x
              << ", " << _scaleFactor.y
              << ", " << _scaleFactor.z
              << std::endl;

    createParticleSystem( );
    _initRenderToTexture( );

    simulationDeltaTime( std::get< T_DELTATIME >( config ));
    simulationStepsPerSecond( std::get< T_STEPS_PER_SEC >( config ));
    changeSimulationDecayValue( std::get< T_DECAY >( config ));

#ifdef VISIMPL_USE_ZEROEQ
    if ( !_zeqUri.empty( ))
    {
      try
      {
        auto& instance = ZeroEQConfig::instance( );
        if ( !instance.isConnected( ))
        {
          instance.connect( _zeqUri );
        }

        _player->connectZeq( instance.subscriber( ) , instance.publisher( ));
        instance.startReceiveLoop( );
        _zeqUri = std::string( ); // clear to avoid re-connect.
      }
      catch ( std::exception& e )
      {
        std::cerr << "Exception when initializing ZeroEQ. ";
        std::cerr << e.what( ) << __FILE__ << ":" << __LINE__ << std::endl;
      }
      catch ( ... )
      {
        std::cerr << "Unknown exception when initializing ZeroEQ. " << __FILE__
                  << ":" << __LINE__ << std::endl;
      }
    }
#endif

    this->_paint = ( _player != nullptr );
    home( ); // stores initial 'home' position.
    update( );
  }

  void OpenGLWidget::keyPressEvent( QKeyEvent* event_ )
  {
    switch ( event_->key( ))
    {
      case Qt::Key_C:
        _flagUpdateSelection = true;
        break;
      default:
        break;
    }
  }

  void OpenGLWidget::changeClearColor( void )
  {
    const auto color = QColorDialog::getColor( _currentClearColor ,
                                               parentWidget( ) ,
                                               "Choose new background color" ,
                                               QColorDialog::DontUseNativeDialog );

    if ( color.isValid( ))
    {
      _currentClearColor = color;

      makeCurrent( );
      glClearColor( float( _currentClearColor.red( )) / 255.0f ,
                    float( _currentClearColor.green( )) / 255.0f ,
                    float( _currentClearColor.blue( )) / 255.0f ,
                    float( _currentClearColor.alpha( )) / 255.0f );

      const QColor inverseColor{ 255 - _currentClearColor.red( ) ,
                                 255 - _currentClearColor.green( ) ,
                                 255 - _currentClearColor.blue( ) ,
                                 255 };

      clippingPlanesColor( inverseColor );

      update( );
    }
  }

  void OpenGLWidget::toggleUpdateOnIdle( void )
  {
    _idleUpdate = !_idleUpdate;

    if ( _idleUpdate && _player )
      update( );
  }

  void OpenGLWidget::toggleShowFPS( void )
  {
    _showFps = !_showFps;

    _fpsLabel->setVisible( _showFps );

    if ( _idleUpdate && _player )
      update( );
  }

  void OpenGLWidget::toggleWireframe( void )
  {
    makeCurrent( );
    _wireframe = !_wireframe;

    if ( _wireframe )
    {
      glEnable( GL_POLYGON_OFFSET_LINE );
      glPolygonOffset( -1 , -1 );
      glLineWidth( 1.5 );
      glPolygonMode( GL_FRONT_AND_BACK , GL_LINE );
    }
    else
    {
      glPolygonMode( GL_FRONT_AND_BACK , GL_FILL );
      glDisable( GL_POLYGON_OFFSET_LINE );
    }

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
    if ( mode != TPlaybackMode::AB_REPEAT )
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
    if ( !_player ) return 0.;

    switch ( _playbackMode )
    {
      case TPlaybackMode::STEP_BY_STEP:
        return _sbsCurrentTime;
      default:
        return _player->currentTime( );
    }
  }

  DomainManager* OpenGLWidget::domainManager( void )
  {
    return &_domainManager;
  }

  void OpenGLWidget::subsetEventsManager( simil::SubsetEventManager* manager )
  {
    _subsetEvents = manager;

    if ( manager ) _createEventLabels( );

    update( );
  }

  simil::SubsetEventManager* OpenGLWidget::subsetEventsManager( void )
  {
    return _subsetEvents;
  }

  const scoop::ColorPalette& OpenGLWidget::colorPalette( void )
  {
    return _colorPalette;
  }

  void OpenGLWidget::_createEventLabels( void )
  {
    const auto& eventNames = _subsetEvents->eventNames( );

    if ( eventNames.empty( ))
      return;

    for ( auto& label: _eventLabels )
    {
      _eventLabelsLayout->removeWidget( label.upperWidget );

      delete ( label.colorLabel );
      delete ( label.label );
      delete ( label.upperWidget );
    }

    _eventLabels.clear( );
    _eventsActivation.clear( );

    const float totalTime = _player->endTime( ) - _player->startTime( );
    const auto& colors = _colorPalette.colors( );

    unsigned int row = 0;
    auto insertEvents = [ &row , &totalTime , &colors , this ](
      const std::string& eventName )
    {
      QPixmap pixmap{ 20 , 20 };
      pixmap.fill( colors[ row ].name( ));
      auto colorLabel = new QLabel( );
      colorLabel->setPixmap( pixmap );

      QLabel* label = new QLabel( );
      label->setMaximumSize( 100 , 50 );
      label->setTextFormat( Qt::RichText );
      label->setStyleSheet(
        "QLabel { background-color : #333;"
        "color : white;"
        "padding: 3px;"
        "margin: 10px;"
        " border-radius: 10px;}" );

      label->setText( QString::fromStdString( eventName ));

      auto container = new QWidget( );
      auto labelLayout = new QHBoxLayout( );

      labelLayout->addWidget( colorLabel );
      labelLayout->addWidget( label );
      container->setLayout( labelLayout );

      EventLabel eventObj;
      eventObj.colorLabel = colorLabel;
      eventObj.label = label;
      eventObj.upperWidget = container;

      _eventLabels.push_back( eventObj );

      _eventLabelsLayout->addWidget( container , row , 10 , 2 , 1 );

      const auto activity = _subsetEvents->eventActivity( eventName ,
                                                          _deltaEvents ,
                                                          totalTime );
      _eventsActivation.push_back( activity );

      ++row;
    };
    std::for_each( eventNames.cbegin( ) , eventNames.cend( ) , insertEvents );
  }

  void OpenGLWidget::Play( void )
  {
    if ( _player )
    {
      _player->Play( );
    }
  }

  void OpenGLWidget::Pause( void )
  {
    if ( _player )
    {
      _player->Pause( );
    }
  }

  void OpenGLWidget::PlayPause( void )
  {

    if ( _player )
    {
      if ( !_player->isPlaying( ))
        _player->Play( );
      else
        _player->Pause( );
    }
  }

  void OpenGLWidget::Stop( void )
  {
    if ( _player )
    {
      _player->Stop( );
      _domainManager.setTime( 0.0f );
      _firstFrame = true;
    }
  }

  void OpenGLWidget::Repeat( bool repeat )
  {
    if ( _player )
    {
      _player->loop( repeat );
    }
  }

  void OpenGLWidget::PlayAt( float timePos )
  {
    if ( _player )
    {
      _backtrace = true;
      std::cout << "Play at " << timePos << std::endl;
      _player->PlayAtTime( timePos );
      _domainManager.setTime( timePos );

    }
  }

  void OpenGLWidget::Restart( void )
  {
    if ( _player )
    {
      const bool playing = _player->isPlaying( );
      _player->Stop( );
      if ( playing )
        _player->Play( );

      _firstFrame = true;
    }
  }

  void OpenGLWidget::PreviousStep( void )
  {
    if ( _playbackMode != TPlaybackMode::STEP_BY_STEP )
    {
      _playbackMode = TPlaybackMode::STEP_BY_STEP;
      _sbsFirstStep = true;
    }

    _sbsPrevStep = true;
    _sbsNextStep = false;
  }

  void OpenGLWidget::NextStep( void )
  {
    if ( _playbackMode != TPlaybackMode::STEP_BY_STEP )
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
    _domainManager.enableAccumulativeMode( accumulative );
  }

  void
  OpenGLWidget::changeSimulationColorMapping( const TTransferFunction& colors )
  {

    visimpl::TColorVec gcolors;

    for ( const auto& c: colors )
    {
      glm::vec4 gColor( c.second.red( ) * invRGBInt ,
                        c.second.green( ) * invRGBInt ,
                        c.second.blue( ) * invRGBInt ,
                        c.second.alpha( ) * invRGBInt );
      gcolors.emplace_back( c.first , gColor );
    }

    _domainManager.getSelectionModel( )->setGradient( gcolors );
  }

  TTransferFunction OpenGLWidget::getSimulationColorMapping( void )
  {
    TTransferFunction result;

    const auto model = _domainManager.getSelectionModel( );
    const auto colors = model->getGradient( );

    for ( const auto& item: colors )
    {
      auto col = item.second;
      const QColor color( col.r * 255 , col.g * 255 , col.b * 255 ,
                          col.a * 255 );
      result.push_back( std::make_pair( item.first , color ));
    }

    return result;
  }

  void OpenGLWidget::changeSimulationSizeFunction( const TSizeFunction& sizes )
  {
    _domainManager.getSelectionModel( )->setParticleSize( sizes );
  }

  TSizeFunction OpenGLWidget::getSimulationSizeFunction( void )
  {
    return _domainManager.getSelectionModel( )->getParticleSize( );
  }

  void OpenGLWidget::simulationDeltaTime( float value )
  {
    if ( _player ) _player->deltaTime( value );

    _simDeltaTime = value;

    _simTimePerSecond = ( _simDeltaTime * _timeStepsPerSecond );
  }

  float OpenGLWidget::simulationDeltaTime( void )
  {
    return _player->deltaTime( );
  }

  void OpenGLWidget::simulationStepsPerSecond( float value )
  {
    if ( std::abs( value ) < std::numeric_limits< float >::epsilon( )) return;

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
    _domainManager.setDecay( value );
  }

  float OpenGLWidget::getSimulationDecayValue( void )
  {
    return _domainManager.getDecay( );
  }

  CameraPosition OpenGLWidget::cameraPosition( ) const
  {
    CameraPosition pos;
    pos.position = _camera->position( );
    pos.radius = _camera->radius( );
    pos.rotation = _camera->rotation( );

    return pos;
  }

  void OpenGLWidget::setCameraPosition( const CameraPosition& pos )
  {
    if ( _camera )
    {
      _camera->position( pos.position );
      _camera->radius( pos.radius );
      _camera->rotation( pos.rotation );
      update( );
    }
  }

  void OpenGLWidget::resetParticles( )
  {
    _firstFrame = true;
    update( );
  }

  const tGidPosMap& OpenGLWidget::getGidPositions( ) const
  {
    return _gidPositions;
  }

} // namespace visimpl
