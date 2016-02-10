#ifndef __QNEUROLOTS__OPENGLWIDGET__
#define __QNEUROLOTS__OPENGLWIDGET__

#include <QOpenGLFunctions>
#include <QOpenGLWidget>
#include <QLabel>
#include <chrono>

#define NEUROLOTS_SKIP_GLEW_INCLUDE 1

#define PREFR_SKIP_GLEW_INCLUDE 1

#define SIM_SLIDER_UPDATE_PERIOD 1

#include <prefr/prefr.h>

#include "CShader.h"

#include <nlrender/nlrender.h>
//#include <nsol/nsol.h>
#include <brion/brion.h>
#include <brain/brain.h>

#include "SimulationPlayer.h"
#include "prefr/ColorEmissionNode.h"
#include "prefr/ColorOperationPrototype.h"
#include "EditorTF/TransferFunctionEditor.h"

#ifdef VISIMPL_USE_ZEQ
  #include <zeq/zeq.h>
  #include <zeq/hbp/hbp.h>
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
    TUndefined = 0,
    TSpikes,
    TVoltages
  } TSimulationType;

  OpenGLWidget( QWidget* parent = 0,
                Qt::WindowFlags windowFlags = 0,
                bool paintNeurons = true,
                const std::string& zeqUri = "" );
  ~OpenGLWidget( void );

  void createNeuronsCollection( void );
  void createParticleSystem( void );
  void loadData( const std::string& fileName,
                 const TDataFileType fileType = TDataFileType::tBlueConfig,
                 TSimulationType simulationType = TSpikes,
                 const std::string& report = std::string( "" ));

  void idleUpdate( bool idleUpdate_ = true )
  {
    _idleUpdate = idleUpdate_;
  }

  visimpl::SimulationPlayer* player( );
  void resetParticles( void );

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


#ifdef VISIMPL_USE_ZEQ

  void _onSelectionEvent( const zeq::Event& event_ );
  void _setZeqUri( const std::string& );
  static void* _Subscriber( void* subscriber );

  bool _zeqConnection;

  servus::URI _uri;
  zeq::Subscriber* _subscriber;

  pthread_t _subscriberThread;


#endif

  QLabel _fpsLabel;
  bool _showFps;

  bool _wireframe;

  nlrender::Camera* _camera;
//  neurolots::NeuronsCollection* _neuronsCollection;
  bool _paintNeurons;

  unsigned int _frameCount;

  int _mouseX, _mouseY;
  bool _rotation, _translation;

  bool _idleUpdate;
  bool _paint;

  QColor _currentClearColor;

  std::chrono::time_point< std::chrono::system_clock > _then;

  CShader* _particlesShader;
  prefr::ParticleSystem* _ps;

//  nsol::BBPSDKReader _bbpReader;
//  nsol::Columns _columns;
//  nsol::SimulationData _simulationData;
//  nsol::NeuronsMap _neurons;

//  float currentTime;
//  bbp::CompartmentReportReader* reader;

  TSimulationType _simulationType;
  visimpl::SimulationPlayer* _player;
  float _maxLife;
  float _deltaTime;
  bool _firstFrame;

  prefr::ColorOperationPrototype* _prototype;

//  brion::BlueConfig* _blueConfig;
//  brion::SpikeReport* _spikeReport;
//  brain::Circuit* _circuit;
//  brion::GIDSet _neuronGIDs;
//  brion::Spikes* _spikes;

  std::unordered_map< uint32_t, prefr::EmissionNode* > gidNodesMap;

private:

  float _elapsedTimeAcc;

};

#endif // __QNEUROLOTS__OPENGLWIDGET__
