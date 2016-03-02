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

#ifdef VISIMPL_USEBRION
  _ui->actionOpenBlueConfig->setEnabled( true );
#else
  _ui->actionOpenBlueConfig->setEnabled( false );
#endif

  connect( _ui->actionQuit, SIGNAL( triggered( )),
           QApplication::instance(), SLOT( quit( )));

  _columnsNumber = 100;
  resizingEnabled = false;
  QTimer::singleShot( 0, this, SLOT( loadComplete( )));
}

void MainWindow::init( const std::string& zeqUri )
{

  connect( _ui->actionOpenBlueConfig, SIGNAL( triggered( )),
           this, SLOT( openBlueConfigThroughDialog( )));

  initPlaybackDock( );


  #ifdef VISIMPL_USE_ZEQ
  if( !zeqUri.empty( ))
  {
     _setZeqUri( zeqUri );
  }
  #endif

}

MainWindow::~MainWindow( void )
{
    delete _ui;
}


void MainWindow::showStatusBarMessage ( const QString& message )
{
  _ui->statusbar->showMessage( message );
}

void MainWindow::openBlueConfig( const std::string& fileName,
                                 visimpl::TSimulationType simulationType,
                                 const std::string& reportLabel )
{
  _simulationType = simulationType;

  switch( _simulationType )
 {
   case visimpl::TSpikes:
     _player = new visimpl::SpikesPlayer( fileName, true );
//     _player->deltaTime( _deltaTime );
     break;

   case visimpl::TVoltages:
     _player = new visimpl::VoltagesPlayer( fileName, reportLabel, true);
//     _deltaTime = _player->deltaTime( );
     break;

   default:
     VISIMPL_THROW("Cannot load an undefined simulation type.");

 }


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
   initSummaryWidget( );
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



void MainWindow::initSummaryWidget( )
{


//  unsigned int widthPerColumn = width( ) / _columnsNumber;

  _summary = new Summary( nullptr, Summary::T_STACK_EXPANDABLE );

//  _summary->setMinimumHeight( _summary->heightPerRow( ));
//  _summary->setMinimumWidth( width( ) - widthPerColumn );
//  _summary->setSizePolicy( QSizePolicy::Maximum,
//                           QSizePolicy::Preferred );


  if( _simulationType == visimpl::TSpikes )
  {
    visimpl::SpikesPlayer* spikesPlayer =
        dynamic_cast< visimpl::SpikesPlayer* >( _player);


//    GIDUSet gids;
//    _summary->AddGIDSelection( gids );
    _summary->CreateSummary( spikesPlayer->spikeReport( ),
                             spikesPlayer->gids( ));
//    _summary->setVisible( true );
  }

  _stackLayout = new QGridLayout( );

  _contentWidget = new QWidget( );
//  _contentWidget->setSizePolicy( QSizePolicy::Expanding,
//                                QSizePolicy::Expanding );
  _contentWidget->setMinimumHeight( _summary->heightPerRow( ) * _summary->histogramsNumber() );
  _contentWidget->setMinimumWidth( width( ) - 5 );
  QScrollArea* scrollArea = new QScrollArea( );
//  QVBoxLayout* centralLayout = new QVBoxLayout( );

  _contentWidget->setLayout( _stackLayout );
  _stackLayout->addWidget( _summary, 0, 0, 1, _columnsNumber - 1 );
//  _stackLayout->addWidget( new QPushButton("Test"), 0, _columnsNumber, 1, 1);

  this->setCentralWidget( scrollArea );
  scrollArea->setWidget( _contentWidget );
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


#ifdef VISIMPL_USE_ZEQ
void MainWindow::_setZeqUri( const std::string& uri_ )
{
  _zeqConnection = true;
  _uri =  servus::URI( uri_ );
  _subscriber = new zeq::Subscriber( _uri );

  _subscriber->registerHandler( zeq::hbp::EVENT_SELECTEDIDS,
      boost::bind( &MainWindow::_onSelectionEvent , this, _1 ));

  pthread_create( &_subscriberThread, NULL, _Subscriber, _subscriber );

}

void* MainWindow::_Subscriber( void* subs )
{
  zeq::Subscriber* subscriber = static_cast< zeq::Subscriber* >( subs );
  while ( true )
  {
    subscriber->receive( 10000 );
  }
  pthread_exit( NULL );
}

void MainWindow::_onSelectionEvent( const zeq::Event& event_ )
{

  std::vector< unsigned int > selected =
      zeq::hbp::deserializeSelectedIDs( event_ );

  GIDUSet selectedSet( selected.begin( ), selected.end( ));

  if( _summary )
  {
    _summary->AddGIDSelection( selectedSet );
    _stackLayout->removeWidget( _summary );

    unsigned int rowsNumber = _summary->histogramsNumber( );

    _stackLayout->addWidget( _summary, 0, 0, rowsNumber,
                             _columnsNumber - 1 );

    QCheckBox* checkBox = new QCheckBox( );
    _checkBoxes.push_back( checkBox );
    _stackLayout->addWidget( checkBox, rowsNumber, _columnsNumber, 1, 1);
  }

}

#endif

void MainWindow::resizeEvent( QResizeEvent * event_ )
{
  QMainWindow::resizeEvent( event_ );

  if( resizingEnabled )
  {
    unsigned int columnsWidth = width( ) / _columnsNumber;
    unsigned int currentWidth = width( ) - columnsWidth;
    unsigned int currentHeight =  _summary->heightPerRow( ) *
                                  _summary->histogramsNumber( );

    _contentWidget->setMinimumWidth( width( ) );
    _contentWidget->setMinimumHeight( currentHeight + 2 );

    std::cout << width( ) << " -> " << currentWidth << std::endl;
//    _summary->setMinimumWidth( currentWidth );
    _summary->resize( currentWidth, currentHeight );
  }
}

void MainWindow::loadComplete( void )
{
  resizingEnabled = true;
  _summary->showMarker( false );
}

