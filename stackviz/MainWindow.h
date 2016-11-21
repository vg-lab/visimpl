/*
 * @file  MainWindow.h
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */

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
#include <simil/simil.h>
// #include "SimulationSummaryWidget.h"

// #include "EditorTF/TransferFunctionEditor.h"

#ifdef VISIMPL_USE_ZEROEQ
#include <zeroeq/zeroeq.h>
#include <thread>
#ifdef VISIMPL_USE_LEXIS
#include <lexis/lexis.h>
#endif
#ifdef VISIMPL_USE_GMRVLEX
#include <gmrvlex/gmrvlex.h>
#endif

#endif

#include "ui_stackviz.h"

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
                       simil::TSimulationType simulationType,
                       const std::string& report);

  void openHDF5File( const std::string& networkFile,
                     simil::TSimulationType simulationType,
                     const std::string& activityFile = "" );

public slots:

  void openBlueConfigThroughDialog( void );

  void PlayPause( bool notify = true );
  void Play( bool notify = true );
  void Pause( bool notify = true );
  void Stop( bool notify = true );
  void Repeat( bool notify = true);
  void PlayAt( bool notify = true );
  void PlayAt( int, bool notify = true );
  void PlayAt( float, bool notify = true );
  void Restart( bool notify = true );
  void GoToEnd( bool notify = true );

  void UpdateSimulationSlider( float percentage );

protected slots:

#ifdef VISIMPL_USE_GMRVLEX

  void ApplyPlaybackOperation( unsigned int playbackOp );
  void _zeqEventRepeat( bool repeat );

  void HistogramClicked( visimpl::MultiLevelHistogram* );

#endif

  void loadComplete( void );

protected:

  void resizeEvent(QResizeEvent * event);
  void configurePlayer( void );

#ifdef VISIMPL_USE_ZEROEQ

  void _onSelectionEvent( lexis::data::ConstSelectedIDsPtr selected );
  void _setZeqUri( const std::string& );
//  static void* _Subscriber( void* subscriber );

  bool _zeqConnection;

  std::string _zeqUri;

  servus::URI _uri;
  zeroeq::Subscriber* _subscriber;
  zeroeq::Publisher* _publisher;

//  pthread_t _subscriberThread;

  std::thread* _thread;

#endif

  void initSummaryWidget( void );
  void initPlaybackDock( void );

  QString _lastOpenedFileName;

  simil::TSimulationType _simulationType;

  Summary* _summary;
  QTimer _selectionsTimer;

  simil::SimulationPlayer* _player;

  // Playback Control
  QDockWidget* _simulationDock;
  QPushButton* _playButton;
  CustomSlider* _simSlider;
  QPushButton* _repeatButton;
  bool _playing;

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
