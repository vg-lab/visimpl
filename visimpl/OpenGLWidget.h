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

#define VISIMPL_SKIP_GLEW_INCLUDE 1

#define PREFR_SKIP_GLEW_INCLUDE 1

#define SIM_SLIDER_UPDATE_PERIOD 0.25f

#include <prefr/prefr.h>
#include <reto/reto.h>
#include <simil/simil.h>

#include "prefr/ColorSource.h"
#include "prefr/ColorOperationModel.h"

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

    void createParticleSystem( float scale = 1.0f );
    void loadData( const std::string& fileName,
                   const simil::TDataType = simil::TDataType::TBlueConfig,
                   simil::TSimulationType simulationType = simil::TSimSpikes,
                   const std::string& report = std::string( "" ));

    void idleUpdate( bool idleUpdate_ = true );

    simil::SimulationPlayer* player( );
    void resetParticles( void );

    void SetAlphaBlendingAccumulative( bool accumulative = true );

    void subsetEventsManager( simil::SubsetEventManager* manager );

  signals:

    void updateSlider( float );

  public slots:

    void togglePaintNeurons( void );
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
    void GoToEnd( void );

    void changeSimulationColorMapping( const TTransferFunction& colors );
    TTransferFunction getSimulationColorMapping( void );

    void changeSimulationSizeFunction( const TSizeFunction& sizes );
    TSizeFunction getSimulationSizeFunction( void );

    void simulationDeltaTime( float value );
    float simulationDeltaTime( void );

    void simulationStepsPerSecond( float value );
    float simulationStepsPerSecond( void );

    void changeSimulationDecayValue( float value );
    float getSimulationDecayValue( void );

    void setSelectedGIDs( const std::unordered_set< uint32_t >& gids  );
    void showSelection( bool );
    void updateSelection( void );
    void clearSelection( void );

    void showEventsActivityLabels( bool show );

  protected:

    virtual void initializeGL( void );
    virtual void paintGL( void );
    virtual void resizeGL( int w, int h );

    virtual void mousePressEvent( QMouseEvent* event );
    virtual void mouseReleaseEvent( QMouseEvent* event );
    virtual void wheelEvent( QWheelEvent* event );
    virtual void mouseMoveEvent( QMouseEvent* event );
    virtual void keyPressEvent( QKeyEvent* event );

    void configureSimulation( void );
    void updateParticles( float renderDelta );
    void paintParticles( void );

    void updateEventLabelsVisibility( void );

    std::vector< bool > activeEventsAt( float time );

    std::unordered_set< uint32_t > _selectedGIDs;

  #ifdef VISIMPL_USE_ZEROEQ

    std::string _zeqUri;

  #endif

    QLabel* _fpsLabel;
    bool _showFps;

    bool _wireframe;

    reto::Camera* _camera;
    glm::vec3 _lastCameraPosition;

    glm::vec3 _boundingBoxMin;
    glm::vec3 _boundingBoxMax;

    bool _focusOnSelection;
    bool _pendingSelection;

    unsigned int _frameCount;

    int _mouseX, _mouseY;
    bool _rotation;
    bool _translation;

    bool _idleUpdate;
    bool _paint;

    QColor _currentClearColor;

    std::chrono::time_point< std::chrono::system_clock > _then;
    std::chrono::time_point< std::chrono::system_clock > _lastFrame;

    reto::ShaderProgram* _particlesShader;
    prefr::ParticleSystem* _particleSystem;

    simil::TSimulationType _simulationType;
    simil::SimulationPlayer* _player;

    float _maxLife;
    float _deltaTime;

    float _simDeltaTime;
    float _timeStepsPerSecond;
    float _simTimePerSecond;

    bool _firstFrame;

    prefr::ColorOperationModel* _prototype;
    prefr::ColorOperationModel* _offPrototype;

    std::unordered_map< uint32_t, prefr::Cluster* > gidNodesMap;
    std::unordered_map< prefr::Cluster*, uint32_t > nodesGIDMap;

    float _renderSpeed;
    float _maxFPS;

    float _simPeriod;
    float _renderPeriod;

    float _elapsedTimeRenderAcc;
    float _elapsedTimeSliderAcc;
    float _elapsedTimeSimAcc;

    bool _alphaBlendingAccumulative;
    bool _showSelection;

    bool _resetParticles;
    bool _updateSelection;

    bool _showActiveEvents;
    simil::SubsetEventManager* _subsetEvents;
    std::vector< EventLabel > _eventLabels;
    QGridLayout* _eventLabelsLayout;
    std::vector< std::vector< bool >> _eventsActivation;
    float _deltaEvents;

    scoop::ColorPalette _eventLabelColors;
  };

} // namespace visimpl

#endif // __VISIMPL__OPENGLWIDGET__
