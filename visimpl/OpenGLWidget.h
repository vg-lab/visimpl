/*
 * @file  OpenGLWidget.h
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */

#ifndef __VISIMPL__OPENGLWIDGET__
#define __VISIMPL__OPENGLWIDGET__

#include <QOpenGLFunctions>
#include <QOpenGLWidget>
#include <QLabel>
#include <chrono>
#include <unordered_set>
#include <queue>

#define VISIMPL_SKIP_GLEW_INCLUDE 1

#define PREFR_SKIP_GLEW_INCLUDE 1

#define SIM_SLIDER_UPDATE_PERIOD 0.25f

#include <prefr/prefr.h>
#include <reto/reto.h>
#include <simil/simil.h>

#include "types.h"

#include "prefr/ColorSource.h"
#include "prefr/ColorOperationModel.h"

#include "render/Plane.h"

#include "DomainManager.h"

#include <sumrice/sumrice.h>
#include <scoop/scoop.h>

#ifdef VISIMPL_USE_ZEROEQ
  #include <zeroeq/zeroeq.h>
  #include <servus/uri.h>

  #include <pthread.h>
  #include <mutex>
#endif

namespace visimpl
{
  typedef enum
  {
    CONTINUOUS = 0,
    STEP_BY_STEP,
    AB_REPEAT
  } TPlaybackMode;

  class OpenGLWidget
    : public QOpenGLWidget
    , public QOpenGLFunctions
  {

    Q_OBJECT;

  public:

    typedef enum
    {
      tBlueConfig,
      SWC,
      NsolScene
    } TDataFileType;

    typedef enum
    {
      PROTOTYPE_OFF = 0,
      PROTOTYPE_ON
    } TPrototypeEnum;

    struct EventLabel
    {
    public:

      QWidget* upperWidget;
      QFrame* frame;
      QLabel* label;

    };

    OpenGLWidget( QWidget* parent = 0,
                  Qt::WindowFlags windowFlags = 0,
                  const std::string& zeqUri = "" );
    ~OpenGLWidget( void );

    void createParticleSystem(  );
    void loadData( const std::string& fileName,
                   const simil::TDataType = simil::TDataType::TBlueConfig,
                   simil::TSimulationType simulationType = simil::TSimSpikes,
                   const std::string& report = std::string( "" ));

    void loadRestData( const std::string& url,
                   const simil::TDataType ,
                   simil::TSimulationType simulationType,
                   const std::string& port);

    void idleUpdate( bool idleUpdate_ = true );

    TPlaybackMode playbackMode( void );
    void playbackMode( TPlaybackMode mode );

    bool completedStep( void );

    simil::SimulationPlayer* player( );
    float currentTime( void );

    void setGroupVisibility( unsigned int i, bool state );

    void circuitScaleFactor( vec3 scale_, bool update = true );
    vec3 circuitScaleFactor( void ) const;

    DomainManager* domainManager( void );

    void resetParticles( void );

    void SetAlphaBlendingAccumulative( bool accumulative = true );

    void subsetEventsManager( simil::SubsetEventManager* manager );

    simil::SubsetEventManager* subsetEventsManager( void );

    const scoop::ColorPalette& colorPalette( void );

  signals:

    void updateSlider( float );

    void stepCompleted( void );

    void attributeStatsComputed( void );

    void pickedSingle( unsigned int );

  public slots:

    void updateData( void );

    void home( void );
    void updateCameraBoundingBox( bool setBoundingBox = false );

    void setMode( int mode );
    void showInactive( bool state );

    void changeShader( int i );

    void setSelectedGIDs( const std::unordered_set< uint32_t >& gids  );
    void clearSelection( void );

    void setUpdateSelection( void );
    void setUpdateGroups( void );
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

    void toggleShowUnselected( void );
    void changeClearColor( void );
    void toggleUpdateOnIdle( void );
    void toggleShowFPS( void );
    void toggleWireframe( void );

    void Play( void );
    void Pause( void );
    void PlayPause( void );
    void Stop( void );
    void Repeat( bool repeat );
    void PlayAt( float percentage );
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



    void _updateParticles( float renderDelta );
    void _paintParticles( void );
    void _paintPlanes( void );

    void _focusOn( const tBoundingBox& boundingBox );

    void _initClippingPlanes( void );

    void _genPlanesFromBoundingBox( void );
    void _genPlanesFromParameters( void );
    void _updatePlanes( void );
    void _rotatePlanes( float yaw, float pitch );

    void _setShaderParticles( void );

    void _pickSingle( void );

    void _backtraceSimulation( void );

    void _configureSimulationFrame( void );
    void _configureStepByStepFrame( double elapsedRenderTimeMilliseconds );

    void _configurePreviousStep( void );
    void _configureStepByStep( void );

    void _modeChange( void );
    void _attributeChange( void );

    void _updateSelection( void );
    void _updateGroups( void );
    void _updateGroupsVisibility( void );
    void _updateAttributes( void );
    void _updateNewData( void );

    void _updateData( void );

    void _createEventLabels( void );
    void _updateEventLabelsVisibility( void );

    std::vector< bool > _activeEventsAt( float time );

    virtual void initializeGL( void );
    virtual void paintGL( void );
    virtual void resizeGL( int w, int h );

    virtual void mousePressEvent( QMouseEvent* event );
    virtual void mouseReleaseEvent( QMouseEvent* event );
    virtual void wheelEvent( QWheelEvent* event );
    virtual void mouseMoveEvent( QMouseEvent* event );
    virtual void keyPressEvent( QKeyEvent* event );


    std::unordered_set< uint32_t > _selectedGIDs;

    std::queue< std::pair< unsigned int, bool >> _pendingGroupStateChanges;

  #ifdef VISIMPL_USE_ZEROEQ

    std::string _zeqUri;

  #endif

    QLabel* _fpsLabel;
    QLabel* _labelCurrentTime;
    bool _showFps;
    bool _showCurrentTime;

    bool _wireframe;

    Camera* _camera;
    glm::vec3 _lastCameraPosition;

    vec3 _scaleFactor;
    bool _scaleFactorExternal;

//    glm::vec3 _boundingBoxMin;
//    glm::vec3 _boundingBoxMax;

    bool _focusOnSelection;
    bool _pendingSelection;
    bool _backtrace;

    TPlaybackMode _playbackMode;

    unsigned int _frameCount;

    int _mouseX, _mouseY;
    bool _rotation;
    bool _translation;

    bool _idleUpdate;
    bool _paint;

    QColor _currentClearColor;
    float _particleRadiusThreshold;

    std::chrono::time_point< std::chrono::system_clock > _then;
    std::chrono::time_point< std::chrono::system_clock > _lastFrame;

    tShaderParticlesType _currentShader;
    reto::ShaderProgram* _shaderParticlesCurrent;
    reto::ShaderProgram* _shaderParticlesDefault;
    reto::ShaderProgram* _shaderParticlesSolid;
    prefr::RenderProgram* _shaderPicking;
    reto::ShaderProgram* _shaderClippingPlanes;

    prefr::ParticleSystem* _particleSystem;
    prefr::GLPickRenderer* _pickRenderer;

    simil::TSimulationType _simulationType;
    simil::LoaderSimData* _importer;
    simil::SpikesPlayer* _player;

    reto::ClippingPlane* _clippingPlaneLeft;
    reto::ClippingPlane* _clippingPlaneRight;
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
    float _renderPeriodMicroseconds;

    float _sliderUpdatePeriod;
    float _sliderUpdatePeriodMicroseconds;

    float _elapsedTimeRenderAcc;
    float _elapsedTimeSliderAcc;
    float _elapsedTimeSimAcc;

    bool _alphaBlendingAccumulative;
    bool _showSelection;

    bool _flagNewData;
    bool _flagResetParticles;
    bool _flagUpdateSelection;
    bool _flagUpdateGroups;
    bool _flagUpdateAttributes;
    bool _flagPickingSingle;
    bool _flagPickingHighlighted;
    bool _flagChangeShader;
    bool _flagUpdateRender;

    bool _flagModeChange;
    tVisualMode _newMode;

    bool _flagAttribChange;
    tNeuronAttributes _newAttrib;
    tNeuronAttributes _currentAttrib;

    bool _showActiveEvents;
    simil::SubsetEventManager* _subsetEvents;
    std::vector< EventLabel > _eventLabels;
    QGridLayout* _eventLabelsLayout;
    std::vector< std::vector< bool >> _eventsActivation;
    float _deltaEvents;

    DomainManager* _domainManager;
    tBoundingBox _boundingBoxHome;

    scoop::ColorPalette _colorPalette;

    QPoint _pickingPosition;
    unsigned int _selectedPickingSingle;

    tGidPosMap _gidPositions;
    TGIDSet _gids;
    TPosVect _positions;
  };

} // namespace visimpl

#endif // __VISIMPL__OPENGLWIDGET__

