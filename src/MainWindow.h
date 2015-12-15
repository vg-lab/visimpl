#include <QMainWindow>
#include <QDockWidget>
#include <QPushButton>
#include <QSlider>
#include <QTimer>

#include "OpenGLWidget.h"



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
                       OpenGLWidget::TSimulationType simulationType,
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

protected:

  QString _lastOpenedFileName;
  QIcon playIcon;
  QIcon pauseIcon;

private:

  void initSimulationDock( void );
  void initSimColorDock( void );

  Ui::MainWindow* _ui;
  OpenGLWidget* _openGLWidget;

//  visimpl::SimulationPlayer* _player;
  QDockWidget* _simulationDock;
  QDockWidget* _simConfigurationDock;
  QSlider* _simSlider;
  QPushButton* _playButton;
  QLabel* _startTimeLabel;
  QLabel* _endTimeLabel;

};
