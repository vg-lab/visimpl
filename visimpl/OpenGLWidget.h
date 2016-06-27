#ifndef __QNEUROLOTS__OPENGLWIDGET__
#define __QNEUROLOTS__OPENGLWIDGET__

#include <QOpenGLFunctions>
#include <QOpenGLWidget>
#include <QLabel>
#include <chrono>
#include <unordered_set>

#define NEUROLOTS_SKIP_GLEW_INCLUDE 1

#define PREFR_SKIP_GLEW_INCLUDE 1

#define SIM_SLIDER_UPDATE_PERIOD 0.5f

#include <prefr/prefr.h>

#include "CShader.h"

#include <nlrender/nlrender.h>
//#include <nsol/nsol.h>
#include <brion/brion.h>
#include <brain/brain.h>

//#include "SimulationPlayer.h"
#include "prefr/ColorEmissionNode.h"
#include "prefr/ColorOperationPrototype.h"
// #include "EditorTF/TransferFunctionEditor.h"
#include <sumrice/sumrice.h>

#ifdef VISIMPL_USE_ZEROEQ
  #include <zeroeq/zeroeq.h>
  #include <servus/uri.h>

  #include <pthread.h>
  #include <mutex>
#endif

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

  OpenGLWidget( QWidget* parent = 0,
                Qt::WindowFlags windowFlags = 0,
                bool paintNeurons = true,
                const std::string& zeqUri = "" );
  ~OpenGLWidget( void );

  void createNeuronsCollection( void );
  void createParticleSystem( float scale = 1.0f );
  void loadData( const std::string& fileName,
                 const visimpl::TDataType = visimpl::TDataType::TBlueConfig,
                 visimpl::TSimulationType simulationType = visimpl::TSimSpikes,
                 const std::string& report = std::string( "" ));

  void idleUpdate( bool idleUpdate_ = true )
  {
    _idleUpdate = idleUpdate_;
  }

  visimpl::SimulationPlayer* player( );
  void resetParticles( void );

  void SetAlphaBlendingAccumulative( bool accumulative = true );


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

  void changeSimulationDecayValue( float value );
  float getSimulationDecayValue( void );

#ifdef VISIMPL_USE_ZEROEQ

  void setSelectedGIDs( const std::unordered_set< uint32_t >& gids  );
  void showSelection( bool );
  void updateSelection( void );

  void ClearSelection( void );

#endif

protected slots:

  void reducePlaybackSpeed( );
  void increasePlaybackSpeed( );

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
  void updateSimulation( void );
  void paintParticles( void );

#ifdef VISIMPL_USE_ZEROEQ

  std::unordered_set< uint32_t > _selectedGIDs;
  std::string _zeqUri;

#endif

  QLabel _fpsLabel;
  bool _showFps;

  bool _wireframe;

  nlrender::Camera* _camera;
  glm::vec3 _lastCameraPosition;

//  neurolots::NeuronsCollection* _neuronsCollection;
  bool _focusOnSelection;

  unsigned int _frameCount;

  int _mouseX, _mouseY;
  bool _rotation, _translation;

  bool _idleUpdate;
  bool _paint;

  QColor _currentClearColor;

  std::chrono::time_point< std::chrono::system_clock > _then;
  std::chrono::time_point< std::chrono::system_clock > _lastFrame;

  CShader* _particlesShader;
  prefr::ParticleSystem* _ps;

//  nsol::BBPSDKReader _bbpReader;
//  nsol::Columns _columns;
//  nsol::SimulationData _simulationData;
//  nsol::NeuronsMap _neurons;

//  float currentTime;
//  bbp::CompartmentReportReader* reader;

  visimpl::TSimulationType _simulationType;
  visimpl::SimulationPlayer* _player;
  float _maxLife;
  float _deltaTime;
  bool _firstFrame;

  prefr::ColorOperationPrototype* _prototype;
  prefr::ColorOperationPrototype* _offPrototype;

//  brion::BlueConfig* _blueConfig;
//  brion::SpikeReport* _spikeReport;
//  brain::Circuit* _circuit;
//  brion::GIDSet _neuronGIDs;
//  brion::Spikes* _spikes;

  std::unordered_map< uint32_t, prefr::EmissionNode* > gidNodesMap;
  std::unordered_map< prefr::EmissionNode*, uint32_t > nodesGIDMap;

  float _playbackSpeed;
  float _renderSpeed;
  float _maxFPS;
  float _renderPeriod;
  float _elapsedTimeRenderAcc;
  float _elapsedTimeSliderAcc;
  float _elapsedTimeSimAcc;

  bool _alphaBlendingAccumulative;
  bool _showSelection;

};

#endif // __QNEUROLOTS__OPENGLWIDGET__
