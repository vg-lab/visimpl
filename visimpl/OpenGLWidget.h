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

#ifndef __VISIMPL__OPENGLWIDGET__
#define __VISIMPL__OPENGLWIDGET__

#define PLAB_SKIP_GLEW_INCLUDE 1
#define NEUROLOTS_SKIP_GLEW_INCLUDE 1

#if defined(VISIMPL_USE_ZEROEQ) && defined(WIN32)
#include <winsock2.h>
#endif

#include <QOpenGLFunctions>
#include <QOpenGLFunctions_3_3_Core>

// Qt
#include <QOpenGLWidget>
#include <QString>

// C++
#include <chrono>
#include <unordered_set>
#include <queue>
#include <locale>
#include <iostream>

#include <reto/reto.h>
#include <sumrice/sumrice.h>
#include <scoop/scoop.h>
#include <plab/plab.h>

#include <sstream>
#include <QOpenGLFunctions_4_0_Core>

#include "types.h"
#include "render/Plane.h"
#include "DomainManager.h"

class QLabel;

struct streamDotSeparator : std::numpunct< char >
{
  char do_decimal_point( ) const
  { return '.'; }
};

namespace visimpl
{
  /** \class CameraPosition
   * \brief Implements a structure to serialize and store camera positions.
   *
   */
  class CameraPosition
  {
  public:
    Eigen::Vector3f position; /** position point.  */
    Eigen::Matrix3f rotation; /** rotation matrix. */
    float radius;             /** aperture.        */

    /** \brief CameraPosition class constructor.
     *
     */
    CameraPosition( )
      : position{ Eigen::Vector3f( ) }
      , rotation{ Eigen::Matrix3f::Zero( ) }
      , radius{ 0 }
    { };

    /** \brief CameraPosition class constructor.
     * \param[in] data Camera position serialized data.
     *
     */
    CameraPosition( const QString& data )
    {
      const auto separator = std::use_facet< std::numpunct< char>>(
        std::cout.getloc( )).decimal_point( );
      const bool needSubst = ( separator == ',' );

      auto parts = data.split( ";" );
      Q_ASSERT( parts.size( ) == 3 );
      const auto posData = parts.first( );
      const auto rotData = parts.last( );
      auto radiusData = parts.at( 1 );

      auto posParts = posData.split( "," );
      Q_ASSERT( posParts.size( ) == 3 );
      auto rotParts = rotData.split( "," );
      Q_ASSERT( rotParts.size( ) == 9 );

      if ( needSubst )
      {
        for ( auto& part: posParts ) part.replace( '.' , ',' );
        for ( auto& part: rotParts ) part.replace( '.' , ',' );
        radiusData.replace( '.' , ',' );
      }

      position = Eigen::Vector3f( posParts[ 0 ].toFloat( ) ,
                                  posParts[ 1 ].toFloat( ) ,
                                  posParts[ 2 ].toFloat( ));
      radius = radiusData.toFloat( );
      rotation.block< 1 , 3 >( 0 , 0 ) = Eigen::Vector3f{
        rotParts[ 0 ].toFloat( ) , rotParts[ 1 ].toFloat( ) ,
        rotParts[ 2 ].toFloat( ) };
      rotation.block< 1 , 3 >( 1 , 0 ) = Eigen::Vector3f{
        rotParts[ 3 ].toFloat( ) , rotParts[ 4 ].toFloat( ) ,
        rotParts[ 5 ].toFloat( ) };
      rotation.block< 1 , 3 >( 2 , 0 ) = Eigen::Vector3f{
        rotParts[ 6 ].toFloat( ) , rotParts[ 7 ].toFloat( ) ,
        rotParts[ 8 ].toFloat( ) };
    }

    /** \brief Returns the serialized camera position.
     *
     */
    QString toString( ) const
    {
      std::stringstream stream;
      stream.imbue( std::locale( stream.getloc( ) , new streamDotSeparator( )));
      stream << position << ";" << radius << ";"
             << rotation( 0 , 0 ) << "," << rotation( 0 , 1 ) << ","
             << rotation( 0 , 2 ) << ","
             << rotation( 1 , 0 ) << "," << rotation( 1 , 1 ) << ","
             << rotation( 1 , 2 ) << ","
             << rotation( 2 , 0 ) << "," << rotation( 2 , 1 ) << ","
             << rotation( 2 , 2 );

      auto serialization = QString::fromStdString( stream.str( ));
      serialization.replace( '\n' , ',' ).remove( ' ' );

      return serialization;
    }
  };

  typedef enum
  {
    CONTINUOUS = 0 ,
    STEP_BY_STEP ,
    AB_REPEAT
  } TPlaybackMode;

  class OpenGLWidget : public QOpenGLWidget
  {
  Q_OBJECT;

  public:
    typedef enum
    {
      tBlueConfig ,
      SWC ,
      NsolScene
    } TDataFileType;

    typedef enum
    {
      PROTOTYPE_OFF = 0 ,
      PROTOTYPE_ON
    } TPrototypeEnum;

    struct EventLabel
    {
      QWidget* upperWidget;
      QLabel* colorLabel;
      QLabel* label;

      EventLabel( )
        : upperWidget{ nullptr }
        , colorLabel{ nullptr }
        , label{ nullptr }
      { };
    };

    OpenGLWidget( QWidget* parent = nullptr ,
                  Qt::WindowFlags windowFlags = Qt::WindowFlags( ) ,
                  const std::string& zeqUri = "" );

    virtual ~OpenGLWidget( );

    void createParticleSystem( );

    /** \brief Sets a new spikes player and initializes.
     * \param[in] p SpikesPlayer pointer.
     *
     */
    void setPlayer( simil::SpikesPlayer* p , const simil::TDataType type );

    const tGidPosMap& getGidPositions( ) const;

    void idleUpdate( bool idleUpdate_ = true );

    TPlaybackMode playbackMode( void );

    void playbackMode( TPlaybackMode mode );

    bool completedStep( void );

    simil::SimulationPlayer* player( );

    float currentTime( void );

    void circuitScaleFactor( vec3 scale_ , bool update = true );

    vec3 circuitScaleFactor( void ) const;

    DomainManager* domainManager( void );

    void SetAlphaBlendingAccumulative( bool accumulative = true );

    void subsetEventsManager( simil::SubsetEventManager* manager );

    simil::SubsetEventManager* subsetEventsManager( void );

    const scoop::ColorPalette& colorPalette( void );

    std::shared_ptr<visimpl::Camera> camera() const
    {
      return _camera;
    }

    /** \brief Returns the current camera position.
     *
     */
    CameraPosition cameraPosition( ) const;

    /** \brief Moves the camera to the given position.
     * \param[in] pos CameraPosition reference.
     *
     */
    void setCameraPosition( const CameraPosition& pos );

    /** \brief Resets the view.
     *
     */
    void resetParticles( );

  signals:

    void updateSlider( float );

    void stepCompleted( void );

    void attributeStatsComputed( void );

    void pickedSingle( unsigned int );

    void planesColorChanged( const QColor& );

  public slots:

    void updateData( void );

    void home( void );

    void updateCameraBoundingBox( bool setBoundingBox = false );

    void setMode( int mode );

    void showInactive( bool state );

    void changeShader( int i );

    void setSelectedGIDs( const std::unordered_set< uint32_t >& gids );

    void clearSelection( void );

    void setUpdateSelection( void );

    void setUpdateAttributes( void );

    void selectAttrib( int newAttrib );

    void showEventsActivityLabels( bool show );

    void showCurrentTimeLabel( bool show );

    void clippingPlanes( bool active );

    void paintClippingPlanes( int paint_ );

    void toggleClippingPlanes( void );

    void clippingPlanesReset( void );

    void clippingPlanesHeight( float height_ );

    float clippingPlanesHeight( void );

    void clippingPlanesWidth( float width_ );

    float clippingPlanesWidth( void );

    void clippingPlanesDistance( float distance_ );

    float clippingPlanesDistance( void );

    void clippingPlanesColor( const QColor& color_ );

    QColor clippingPlanesColor( void );

    void changeClearColor( void );

    void toggleUpdateOnIdle( void );

    void toggleShowFPS( void );

    void toggleWireframe( void );

    void Play( void );

    void Pause( void );

    void PlayPause( void );

    void Stop( void );

    void Repeat( bool repeat );

    void PlayAt( float timePos );

    void Restart( void );

    void PreviousStep( void );

    void NextStep( void );

    void changeSimulationColorMapping( const TTransferFunction& colors );

    TTransferFunction getSimulationColorMapping( void );

    void changeSimulationSizeFunction( const TSizeFunction& sizes );

    TSizeFunction getSimulationSizeFunction( void );

    void simulationDeltaTime( float value );

    float simulationDeltaTime( void );

    void simulationStepsPerSecond( float value );

    float simulationStepsPerSecond( void );

    void simulationStepByStepDuration( float value );

    float simulationStepByStepDuration( void );

    void changeSimulationDecayValue( float value );

    float getSimulationDecayValue( void );

    GIDVec getPlanesContainedElements( void ) const;

  protected:
    void _resolveFlagsOperations( void );

    void _updateParticles( );

    void _paintParticles( void );

    void _paintPlanes( void );

    void _focusOn( const tBoundingBox& boundingBox );

    void _initClippingPlanes( void );

    void _genPlanesFromBoundingBox( void );

    void _genPlanesFromParameters( void );

    void _updatePlanes( void );

    void _rotatePlanes( float yaw , float pitch );

    void _backtraceSimulation( void );

    void _initRenderToTexture( void );

    void _configureSimulationFrame( void );

    void _configureStepByStepFrame( double elapsedRenderTimeMilliseconds );

    void _configurePreviousStep( void );

    void _configureStepByStep( void );

    void _modeChange( void );

    void _attributeChange( void );

    void _updateSelection( void );

    void _updateAttributes( void );

    void _updateNewData( void );

    bool _updateData( bool force = false );

    void _createEventLabels( void );

    void _updateEventLabelsVisibility( void );

    std::vector< bool > _activeEventsAt( float time );

    virtual void initializeGL( void );

    virtual void paintGL( void );

    virtual void resizeGL( int w , int h );

    virtual void mousePressEvent( QMouseEvent* event );

    virtual void mouseReleaseEvent( QMouseEvent* event );

    virtual void wheelEvent( QWheelEvent* event );

    virtual void mouseMoveEvent( QMouseEvent* event );

    virtual void keyPressEvent( QKeyEvent* event );

    /** \brief Connects the player to ZeroEQ signaling.
     *
     */
    void connectPlayerZeroEQ( );

    std::unordered_set< uint32_t > _selectedGIDs;

#ifdef VISIMPL_USE_ZEROEQ

    std::string _zeqUri;

#endif

    QOpenGLFunctions_4_0_Core _gl;

    QLabel* _fpsLabel;
    QLabel* _labelCurrentTime;
    bool _showFps;
    bool _showCurrentTime;

    bool _wireframe;

    std::shared_ptr< Camera > _camera;
    glm::vec3 _lastCameraPosition;

    vec3 _scaleFactor;
    bool _scaleFactorExternal;

    bool _focusOnSelection;
    bool _pendingSelection;
    bool _backtrace;

    TPlaybackMode _playbackMode;

    unsigned int _frameCount;

    int _mouseX , _mouseY;
    bool _rotation;
    bool _translation;

    bool _idleUpdate;
    bool _paint;

    QColor _currentClearColor;
    float _particleRadiusThreshold;

    std::chrono::time_point< std::chrono::system_clock > _then;
    std::chrono::time_point< std::chrono::system_clock > _lastFrame;

    bool _particleSystemInitialized;
    reto::ShaderProgram* _shaderClippingPlanes;
    simil::SpikesPlayer* _player;

    std::shared_ptr< reto::ClippingPlane > _clippingPlaneLeft;
    std::shared_ptr< reto::ClippingPlane > _clippingPlaneRight;
    evec3 _planesCenter;
    evec3 _planeNormalLeft;
    evec3 _planeNormalRight;
    std::vector< Eigen::Vector3f > _planePosLeft;
    std::vector< Eigen::Vector3f > _planePosRight;
    Eigen::Matrix4f _planeRotation;
    float _planeHeight;
    float _planeWidth;
    float _planeDistance;
    bool _rotationPlanes;
    bool _translationPlanes;
    bool _clipping;
    bool _paintClippingPlanes;
    Plane _planeLeft;
    Plane _planeRight;
    evec4 _planesColor;

    double _deltaTime;
    double _sbsTimePerStep;
    double _sbsInvTimePerStep;
    double _sbsBeginTime;
    double _sbsEndTime;
    double _sbsCurrentTime;
    double _sbsCurrentRenderDelta;

    simil::SpikesCRange _sbsStepSpikes;
    simil::SpikesCIter _sbsCurrentSpike;

    bool _sbsPlaying;
    bool _sbsFirstStep;
    bool _sbsNextStep;
    bool _sbsPrevStep;

    double _simDeltaTime;
    double _timeStepsPerSecond;
    double _simTimePerSecond;

    bool _firstFrame;

    float _renderSpeed;
    float _maxFPS;

    float _simPeriod;
    float _simPeriodMicroseconds;
    float _renderPeriod;

    float _sliderUpdatePeriod;
    float _sliderUpdatePeriodMicroseconds;

    float _elapsedTimeSliderAcc;
    float _elapsedTimeSimAcc;

    bool _alphaBlendingAccumulative;
    bool _showSelection;

    bool _flagNewData;
    bool _flagUpdateSelection;
    bool _flagUpdateAttributes;

    VisualMode _newMode;

    bool _flagAttribChange;
    tNeuronAttributes _newAttrib;
    tNeuronAttributes _currentAttrib;

    bool _showActiveEvents;
    simil::SubsetEventManager* _subsetEvents;
    std::vector< EventLabel > _eventLabels;
    QGridLayout* _eventLabelsLayout;
    std::vector< std::vector< bool >> _eventsActivation;
    float _deltaEvents;

    DomainManager _domainManager;
    tBoundingBox _boundingBoxHome;
    QString _homePosition;

    scoop::ColorPalette _colorPalette;

    QPoint _pickingPosition;

    tGidPosMap _gidPositions; // particle positions * scale.

    // Render to texture
    reto::ShaderProgram* _screenPlaneShader;

    unsigned int _quadVAO;
    unsigned int _weightFrameBuffer;
    unsigned int _accumulationTexture;
    unsigned int _revealTexture;
  };
} // namespace visimpl

#endif // __VISIMPL__OPENGLWIDGET__
