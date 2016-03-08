#include <QMainWindow>
#include <QDockWidget>
#include <QPushButton>
#include <QSlider>
#include <QTimer>
#include <QRadioButton>

#include "OpenGLWidget.h"
//#include "SimulationPlayer.h"

#include <sumrice/sumrice.h>
// #include "SimulationSummaryWidget.h"

// #include "EditorTF/TransferFunctionEditor.h"


namespace Ui
{
class MainWindow;
}

class MainWindow
  : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow( QWidget* parent = 0,
                       bool updateOnIdle = true );
  ~MainWindow( void );

  void init( const std::string& zeqUri = "" );
  void showStatusBarMessage ( const QString& message );

  void openBlueConfig( const std::string& fileName,
                       visimpl::TSimulationType simulationType,
                       const std::string& report);

  void openXMLScene( const std::string& fileName );
  void openSWCFile( const std::string& fileName );


public slots:

  void openBlueConfigThroughDialog( void );
  void openXMLSceneThroughDialog( void );
  void openSWCFileThroughDialog( void );

  void Play( void );
  void Stop( void );
  void Repeat( bool repeat );
  void PlayAt( void );
  void PlayAt( int );
  void Restart( void );
  void GoToEnd( void );

  void UpdateSimulationSlider( float percentage );

  void UpdateSimulationColorMapping( void );
  void PreviewSimulationColorMapping( void );
  void changeEditorColorMapping( void );
  void changeEditorSizeFunction( void );
  void UpdateSimulationSizeFunction( void );
  void PreviewSimulationSizeFunction( void );

  void changeEditorDecayValue( void );
  void UpdateSimulationDecayValue( void );

  void AlphaBlendingToggled( void );

protected:

#ifdef VISIMPL_USE_ZEQ

  void _onSelectionEvent( const zeq::Event& event_ );
  void _setZeqUri( const std::string& );
  static void* _Subscriber( void* subscriber );

  bool _zeqConnection;

  servus::URI _uri;
  zeq::Subscriber* _subscriber;

  pthread_t _subscriberThread;


#endif

  QString _lastOpenedFileName;
  QIcon _playIcon;
  QIcon _pauseIcon;

private:

  void initPlaybackDock( void );
  void initSimColorDock( void );
  void initSummaryWidget( void );

  Ui::MainWindow* _ui;
  OpenGLWidget* _openGLWidget;
  Summary* _summary;

//  visimpl::SimulationPlayer* _player;
  QDockWidget* _simulationDock;
  QSlider* _simSlider;
  QPushButton* _playButton;
  QLabel* _startTimeLabel;
  QLabel* _endTimeLabel;

  QDockWidget* _simConfigurationDock;
  TransferFunctionEditor* _tfEditor;
  TransferFunctionWidget* _tfWidget;
  ParticleSizeWidget* _psWidget;
  QDoubleSpinBox* _decayBox;

  QRadioButton* _alphaNormalButton;
  QRadioButton* _alphaAccumulativeButton;
};
