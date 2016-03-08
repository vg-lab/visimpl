#include <QMainWindow>
#include <QDockWidget>
#include <QPushButton>
#include <QSlider>
#include <QTimer>
#include <QLabel>
#include <QGridLayout>
#include <QScrollArea>
#include <QCheckBox>

// #include "SimulationPlayer.h"
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
  explicit MainWindow( QWidget* parent = 0 );
  ~MainWindow( void );

  void init( const std::string& zeqUri = "" );
  void showStatusBarMessage ( const QString& message );

  void openBlueConfig( const std::string& fileName,
                       visimpl::TSimulationType simulationType,
                       const std::string& report);

public slots:

  void openBlueConfigThroughDialog( void );

  void Play( void );
  void Stop( void );
  void Repeat( bool repeat );
  void PlayAt( void );
  void PlayAt( int );
  void Restart( void );
  void GoToEnd( void );

  void UpdateSimulationSlider( float percentage );

protected slots:

  void loadComplete( void );

protected:

  void resizeEvent(QResizeEvent * event);

#ifdef VISIMPL_USE_ZEQ

  void _onSelectionEvent( const zeq::Event& event_ );
  void _setZeqUri( const std::string& );
  static void* _Subscriber( void* subscriber );

  bool _zeqConnection;

  servus::URI _uri;
  zeq::Subscriber* _subscriber;

  pthread_t _subscriberThread;

  ZeqEventsManager* _zeqEvents;

#endif

  void initSummaryWidget( void );
  void initPlaybackDock( void );
  void updateStackView( void );

  QString _lastOpenedFileName;

  visimpl::TSimulationType _simulationType;

  Summary* _summary;
  visimpl::SimulationPlayer* _player;


  // Playback Control
  QDockWidget* _simulationDock;
  QPushButton* _playButton;
  CustomSlider* _simSlider;

  QIcon _playIcon;
  QIcon _pauseIcon;

  QLabel* _startTimeLabel;
  QLabel* _endTimeLabel;

private:

  Ui::MainWindow* _ui;

  QWidget* _contentWidget;
  QGridLayout* _stackLayout;
  unsigned int _columnsNumber;
  bool resizingEnabled;
  std::vector< QCheckBox* > _checkBoxes;

};
