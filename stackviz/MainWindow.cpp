#include "ui_stackviz.h"
#include "MainWindow.h"
#include <QDebug>
#include <QFileDialog>
#include <QInputDialog>
#include <QGridLayout>

#include <boost/bind.hpp>

MainWindow::MainWindow( QWidget* parent_ )
  : QMainWindow( parent_ )
  // , _lastOpenedFileName( "" )
  , _ui( new Ui::MainWindow )
  // , _player( nullptr )
{
  _ui->setupUi( this );

#ifdef VISIMPL_USE_BBPSDK
  _ui->actionOpenBlueConfig->setEnabled( true );
#else
  _ui->actionOpenBlueConfig->setEnabled( false );
#endif

  connect( _ui->actionQuit, SIGNAL( triggered( )),
           QApplication::instance(), SLOT( quit( )));

  init( )
}

void MainWindow::init( const std::string& /*zeqUri*/ )
{

  connect( _ui->actionOpenBlueConfig, SIGNAL( triggered( )),
           this, SLOT( openBlueConfigThroughDialog( )));

  // initSimulationDock( );

  // #ifdef VISIMPL_USE_ZEQ
  // if( !zeqUri.empty( ))
  // {
  //     _setZeqUri( zeqUri );
  // }
  // #endif
}

MainWindow::~MainWindow( void )
{
    delete _ui;
}


void MainWindow::showStatusBarMessage ( const QString& message )
{
  _ui->statusbar->showMessage( message );
}

void MainWindow::openBlueConfig( const std::string& /*fileName*/,
                                 visimpl::TSimulationType /*simulationType*/,
                                 const std::string& /*reportLabel*/)
{
  // _player->loadData( fileName,
  //                          OpenGLWidget::TDataFileType::tBlueConfig,
  //                          simulationType, reportLabel );


  // connect( _player, SIGNAL( updateSlider( float )),
  //          this, SLOT( UpdateSimulationSlider( float )));


  // _startTimeLabel->setText(
  //     QString::number( (double)_player->player( )->startTime( )));

  // _endTimeLabel->setText(
  //       QString::number( (double)_player->player( )->endTime( )));


  // changeEditorColorMapping( );
  // initSummaryWidget( );
}

void MainWindow::openBlueConfigThroughDialog( void )
{
#ifdef VISIMPL_USE_BRION

//   QString path = QFileDialog::getOpenFileName(
//     this, tr( "Open BlueConfig" ), _lastOpenedFileName,
//     tr( "BlueConfig ( BlueConfig CircuitConfig);; All files (*)" ),
//     nullptr, QFileDialog::DontUseNativeDialog );

//   if (path != QString( "" ))
//   {
//     bool ok1, ok2;
//     QInputDialog simTypeDialog;
//     visimpl::TSimulationType simType;
//     QStringList items = {"Spikes", "Voltages"};

//     QString text = QInputDialog::getItem(
//       this, tr( "Please select simulation type" ),
//       tr( "Type:" ), items, 0, false, &ok1 );

//     if( !ok1 )
//       return;

//     if( text == items[0] )
//     {
//       simType = visimpl::TSpikes;
//       ok2 = true;
//     }
//     else
//     {
//       simType = visimpl::TVoltages;

//       text = QInputDialog::getText(
//           this, tr( "Please select report" ),
//           tr( "Report:" ), QLineEdit::Normal,
//           "soma", &ok2 );
//     }

//     if ( ok1 && ok2 && !text.isEmpty( ))
//     {
// //      std::string targetLabel = text.toStdString( );
//       std::string reportLabel = text.toStdString( );
//       _lastOpenedFileName = QFileInfo(path).path( );
//       std::string fileName = path.toStdString( );
//       openBlueConfig( fileName, simType, reportLabel );
//     }


//   }
#endif

}

void MainWindow::initPlaybackDock( )
{
  _simulationDock = new QDockWidget( );
  _simulationDock->setMinimumHeight( 100 );
  _simulationDock->setSizePolicy( QSizePolicy::MinimumExpanding,
                                  QSizePolicy::MinimumExpanding );

  unsigned int totalHSpan = 20;

  QWidget* content = new QWidget( );
  QGridLayout* dockLayout = new QGridLayout( );
  content->setLayout( dockLayout );

  _simSlider = new CustomSlider( Qt::Horizontal );
  _simSlider->setMinimum( 0 );
  _simSlider->setMaximum( 1000 );
  _simSlider->setSizePolicy( QSizePolicy::Preferred,
                             QSizePolicy::Preferred );

//  QPushButton* playButton = new QPushButton( );
  _playButton = new QPushButton( );
  _playButton->setSizePolicy( QSizePolicy::MinimumExpanding,
                             QSizePolicy::MinimumExpanding );
  QPushButton* stopButton = new QPushButton( );
  QPushButton* nextButton = new QPushButton( );
  QPushButton* prevButton = new QPushButton( );
  QPushButton* repeatButton = new QPushButton( );
  repeatButton->setCheckable( true );
  repeatButton->setChecked( false );

//  QIcon playIcon;
//  QIcon pauseIcon;
  QIcon stopIcon;
  QIcon nextIcon;
  QIcon prevIcon;
  QIcon repeatIcon;

  _playIcon.addFile( QStringLiteral( ":/icons/play.png" ), QSize( ),
                    QIcon::Normal, QIcon::Off );
  _pauseIcon.addFile( QStringLiteral( ":/icons/pause.png" ), QSize( ),
                     QIcon::Normal, QIcon::Off) ;
  stopIcon.addFile( QStringLiteral( ":/icons/stop.png" ), QSize( ),
                    QIcon::Normal, QIcon::Off );
  nextIcon.addFile( QStringLiteral( ":/icons/next.png" ), QSize( ),
                    QIcon::Normal, QIcon::Off );
  prevIcon.addFile( QStringLiteral( ":/icons/previous.png" ), QSize( ),
                    QIcon::Normal, QIcon::Off );
  repeatIcon.addFile( QStringLiteral( ":/icons/repeat.png" ), QSize( ),
                      QIcon::Normal, QIcon::Off );

  _playButton->setIcon( _playIcon );
  stopButton->setIcon( stopIcon );
  nextButton->setIcon( nextIcon );
  prevButton->setIcon( prevIcon );
  repeatButton->setIcon( repeatIcon );

  _startTimeLabel = new QLabel( "" );
  _startTimeLabel->setSizePolicy( QSizePolicy::MinimumExpanding,
                                  QSizePolicy::Preferred );
  _endTimeLabel = new QLabel( "" );
  _endTimeLabel->setSizePolicy( QSizePolicy::Preferred,
                                  QSizePolicy::Preferred );

  unsigned int row = 2;
  dockLayout->addWidget( _startTimeLabel, row, 0, 1, 2 );
  dockLayout->addWidget( _simSlider, row, 1, 1, totalHSpan - 3 );
  dockLayout->addWidget( _endTimeLabel, row, totalHSpan - 2, 1, 1, Qt::AlignRight );

  row++;
  dockLayout->addWidget( repeatButton, row, 7, 1, 1 );
  dockLayout->addWidget( prevButton, row, 8, 1, 1 );
  dockLayout->addWidget( _playButton, row, 9, 2, 2 );
  dockLayout->addWidget( stopButton, row, 11, 1, 1 );
  dockLayout->addWidget( nextButton, row, 12, 1, 1 );

  connect( _playButton, SIGNAL( clicked( )),
           this, SLOT( Play( )));

  connect( stopButton, SIGNAL( clicked( )),
             this, SLOT( Stop( )));

  connect( nextButton, SIGNAL( clicked( )),
             this, SLOT( GoToEnd( )));

  connect( prevButton, SIGNAL( clicked( )),
             this, SLOT( Restart( )));

  connect( repeatButton, SIGNAL( toggled( bool )),
             this, SLOT( Repeat( bool )));

  connect( _simSlider, SIGNAL( sliderPressed( )),
           this, SLOT( PlayAt( )));

//  connect( _simSlider, SIGNAL( sliderMoved( )),
//             this, SLOT( PlayAt( )));

//  _summary = new Summary( nullptr, Summary::T_STACK_FIXED );
////  _summary->setVisible( false );
//  _summary->setMinimumHeight( 50 );
//
//  dockLayout->addWidget( _summary, 0, 1, 2, totalHSpan - 3 );

  _simulationDock->setWidget( content );
  this->addDockWidget( Qt::/*DockWidgetAreas::enum_type::*/BottomDockWidgetArea,
                       _simulationDock );
}


void MainWindow::Play( void )
{
//  playIcon.swap( pauseIcon );

  if( _player )
  {

    if( _player->isPlaying( ))
    {
      _player->Pause( );
      _playButton->setIcon( _pauseIcon );
    }
    else
    {
      _player->Play( );
      _playButton->setIcon( _playIcon );
    }
  }
}

void MainWindow::Stop( void )
{
  if( _player )
  {
    _player->Stop( );
    _playButton->setIcon( _playIcon );
    _startTimeLabel->setText(
          QString::number( (double)_player->startTime( )));
  }
}

void MainWindow::Repeat( bool repeat )
{
  if( _player )
  {
    _player->loop( repeat );
  }
}

void MainWindow::PlayAt( void )
{
  if( _player )
  {
    PlayAt( _simSlider->sliderPosition( ));
  }
}

void MainWindow::PlayAt( int sliderPosition )
{
  if( _player )
  {

    _player->Pause( );

    int value = _simSlider->value( );
    float percentage = float( value - _simSlider->minimum( )) /
                       float( _simSlider->maximum( ) - _simSlider->minimum( ));
    _simSlider->setSliderPosition( sliderPosition );

    _playButton->setIcon( _pauseIcon );

    _player->PlayAt( percentage );

    // Send event
  }
}

void MainWindow::Restart( void )
{
  if( _player )
  {
    _player->Stop( );
    _player->Play( );

    if( _player->isPlaying( ))
      _playButton->setIcon( _pauseIcon );
    else
      _playButton->setIcon( _playIcon );
  }
}

void MainWindow::GoToEnd( void )
{
  if( _player )
  {
    _player->GoTo( _player->endTime( ));
  }
}

void MainWindow::UpdateSimulationSlider( float percentage )
{
//  if( _player->player( )->isPlaying( ))
  {

    _startTimeLabel->setText(
          QString::number( (double) _player->currentTime( )));

//    float percentage = _player->player( )->GetRelativeTime( );

    int total = _simSlider->maximum( ) - _simSlider->minimum( );

    int position = percentage * total;
//    std::cout << "Timer: " << percentage << " * "
//              << total << " = " << position << std::endl;

    _simSlider->setSliderPosition( position );
  }
}

