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

  void openData( const std::string& fileName,
                       visimpl::TSimulationType simulationType,
                       const std::string& report);

  void openXMLScene( const std::string& fileName );
  void openSWCFile( const std::string& fileName );


public slots:

  void openBlueConfigThroughDialog( void );
  void openXMLSceneThroughDialog( void );
  void openSWCFileThroughDialog( void );

  void PlayPause( bool notify = true );
  void Play( bool notify = true );
  void Pause( bool notify = true );
  void Stop( bool notify = true );
  void Repeat( bool notify = true );
  void PlayAt( bool notify = true );
  void PlayAt( float, bool notify = true );
  void PlayAt( int, bool notify = true );
  void Restart( bool notify = true );
  void GoToEnd( bool notify = true );

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


protected slots:

#ifdef VISIMPL_USE_ZEQ

#ifdef VISIMPL_USE_GMRVZEQ

  void ApplyPlaybackOperation( unsigned int playbackOp );
  void _zeqEventRepeat( bool repeat );

#endif

  void ClearSelection( void );

protected:


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
  QPushButton* _repeatButton;

  QDockWidget* _simConfigurationDock;
  TransferFunctionEditor* _tfEditor;
  TransferFunctionWidget* _tfWidget;
  ParticleSizeWidget* _psWidget;
  QDoubleSpinBox* _decayBox;

  QPushButton* _clearSelectionButton;
  QLabel* _selectionSizeLabel;

  QRadioButton* _alphaNormalButton;
  QRadioButton* _alphaAccumulativeButton;
};
