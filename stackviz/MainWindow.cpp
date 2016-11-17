#include "MainWindow.h"
#include <QDebug>
#include <QFileDialog>
#include <QInputDialog>
#include <QGridLayout>

#include <boost/bind.hpp>

#include <thread>

MainWindow::MainWindow( QWidget* parent_ )
  : QMainWindow( parent_ )
  // , _lastOpenedFileName( "" )
  , _ui( new Ui::MainWindow )
  // , _player( nullptr )
{
  _ui->setupUi( this );

#ifdef VISIMPL_USE_BRION
  _ui->actionOpenBlueConfig->setEnabled( true );
#else
  _ui->actionOpenBlueConfig->setEnabled( false );
#endif

  connect( _ui->actionQuit, SIGNAL( triggered( )),
           QApplication::instance(), SLOT( quit( )));

  _columnsNumber = 100;
  resizingEnabled = false;
//  QTimer::singleShot( 0, this, SLOT( loadComplete( )));
}

void MainWindow::init( const std::string& zeqUri )
{

  connect( _ui->actionOpenBlueConfig, SIGNAL( triggered( )),
           this, SLOT( openBlueConfigThroughDialog( )));

  initPlaybackDock( );


  #ifdef VISIMPL_USE_ZEROEQ

  _setZeqUri( zeqUri );

  #endif

}

MainWindow::~MainWindow( void )
{
  delete _ui;

  if( _thread )
    delete _thread;
}


void MainWindow::showStatusBarMessage ( const QString& message )
{
  _ui->statusbar->showMessage( message );
}

void MainWindow::openBlueConfig( const std::string& fileName,
                                 simil::TSimulationType simulationType,
                                 const std::string& reportLabel )
{
  _simulationType = simulationType;

  switch( _simulationType )
 {
   case simil::TSimSpikes:
   {
     simil::SpikesPlayer* player = new simil::SpikesPlayer( );
     player->LoadData( simil::TDataType::TBlueConfig,  fileName );
     _player = player;

//     _player->deltaTime( _deltaTime );
     break;
   }
   case simil::TSimVoltages:
     _player = new simil::VoltagesPlayer( fileName, reportLabel, true);
//     _deltaTime = _player->deltaTime( );
     break;

   default:
     VISIMPL_THROW("Cannot load an undefined simulation type.");

 }

  configurePlayer( );

}

void MainWindow::openBlueConfigThroughDialog( void )
{
#ifdef VISIMPL_USE_BRION

   QString path = QFileDialog::getOpenFileName(
     this, tr( "Open BlueConfig" ), _lastOpenedFileName,
     tr( "BlueConfig ( BlueConfig CircuitConfig);; All files (*)" ),
     nullptr, QFileDialog::DontUseNativeDialog );

   if (path != QString( "" ))
   {
     bool ok1, ok2;
     QInputDialog simTypeDialog;
     simil::TSimulationType simType;
     QStringList items = {"Spikes", "Voltages"};

     QString text = QInputDialog::getItem(
       this, tr( "Please select simulation type" ),
       tr( "Type:" ), items, 0, false, &ok1 );

     if( !ok1 )
       return;

     if( text == items[0] )
     {
       simType = simil::TSimSpikes;
       ok2 = true;
     }
     else
     {
       simType = simil::TSimVoltages;

       text = QInputDialog::getText(
           this, tr( "Please select report" ),
           tr( "Report:" ), QLineEdit::Normal,
           "soma", &ok2 );
     }

     if ( ok1 && ok2 && !text.isEmpty( ))
     {
 //      std::string targetLabel = text.toStdString( );
       std::string reportLabel = text.toStdString( );
       _lastOpenedFileName = QFileInfo(path).path( );
       std::string fileName = path.toStdString( );
       openBlueConfig( fileName, simType, reportLabel );
     }


   }
#endif

}


void MainWindow::openHDF5File( const std::string& networkFile,
                               simil::TSimulationType simulationType,
                               const std::string& activityFile )
{
  _simulationType = simulationType;

  simil::SpikesPlayer* player = new simil::SpikesPlayer( );
  player->LoadData( simil::TDataType::THDF5,  networkFile, activityFile );
  _player = player;

  configurePlayer( );
}

void MainWindow::configurePlayer( void )
{
  _startTimeLabel->setText(
      QString::number( (double)_player->startTime( )));

  _endTimeLabel->setText(
        QString::number( (double)_player->endTime( )));

#ifdef SIMIL_USE_ZEROEQ
  _player->connectZeq( _zeqUri );

  _player->zeqEvents( )->frameReceived.connect(
      boost::bind( &MainWindow::UpdateSimulationSlider, this, _1 ));

  _player->zeqEvents( )->playbackOpReceived.connect(
      boost::bind( &MainWindow::ApplyPlaybackOperation, this, _1 ));
#endif

 // changeEditorColorMapping( );
  initSummaryWidget( );
}

void MainWindow::initPlaybackDock( )
{
  _simulationDock = new QDockWidget( );
  _simulationDock->setMinimumHeight( 100 );
  _simulationDock->setSizePolicy( QSizePolicy::MinimumExpanding,
                                  QSizePolicy::MinimumExpanding );

  _playing = false;
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
//  QPushButton* repeatButton = new QPushButton( );
  _repeatButton = new QPushButton( );
  _repeatButton->setCheckable( true );
  _repeatButton->setChecked( false );

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
  _repeatButton->setIcon( repeatIcon );

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
  dockLayout->addWidget( _repeatButton, row, 7, 1, 1 );
  dockLayout->addWidget( prevButton, row, 8, 1, 1 );
  dockLayout->addWidget( _playButton, row, 9, 2, 2 );
  dockLayout->addWidget( stopButton, row, 11, 1, 1 );
  dockLayout->addWidget( nextButton, row, 12, 1, 1 );

  connect( _playButton, SIGNAL( clicked( )),
           this, SLOT( PlayPause( )));

  connect( stopButton, SIGNAL( clicked( )),
             this, SLOT( Stop( )));

  connect( nextButton, SIGNAL( clicked( )),
             this, SLOT( GoToEnd( )));

  connect( prevButton, SIGNAL( clicked( )),
             this, SLOT( Restart( )));

  connect( _repeatButton, SIGNAL( clicked( )),
           this, SLOT( Repeat( )));

  connect( _simSlider, SIGNAL( sliderPressed( )),
           this, SLOT( PlayAt( )));

  _simulationDock->setWidget( content );
  this->addDockWidget( Qt::BottomDockWidgetArea,
                       _simulationDock );
}



void MainWindow::initSummaryWidget( )
{

  _summary = new Summary( nullptr, Summary::T_STACK_EXPANDABLE );

  if( _simulationType == simil::TSimSpikes )
  {
    simil::SpikesPlayer* spikesPlayer =
        dynamic_cast< simil::SpikesPlayer* >( _player);

    _summary->Init( spikesPlayer->spikeReport( ),
                             spikesPlayer->gids( ));
  }

  _stackLayout = new QGridLayout( );

  this->setCentralWidget( _summary );

  connect( _ui->actionAutoNamingSelections, SIGNAL( triggered( )),
           _summary, SLOT( toggleAutoNameSelections( )));

  connect( _summary, SIGNAL( histogramClicked( float )),
           this, SLOT( PlayAt( float )));

  connect( _summary, SIGNAL( histogramClicked( visimpl::MultiLevelHistogram* )),
             this, SLOT( HistogramClicked( visimpl::MultiLevelHistogram* )));

}

void MainWindow::PlayPause( bool notify )
{
  if( _playing )
    Pause( notify );
  else
    Play( notify );
}

void MainWindow::Play( bool notify )
{

  if( _player )
  {
      _player->Play( );
      _playButton->setIcon( _pauseIcon );
      _playing = true;

      if( notify )
      {
#ifdef VISIMPL_USE_ZEROEQ
      _player ->zeqEvents( )->sendPlaybackOp( zeroeq::gmrv::PLAY );
#endif
      }
  }

}

void MainWindow::Pause( bool notify )
{
  if( _player )
  {
    _player->Pause( );
    _playButton->setIcon( _playIcon );
    _playing = false;

    if( notify )
    {
  #ifdef VISIMPL_USE_ZEROEQ
    _player ->zeqEvents( )->sendPlaybackOp( zeroeq::gmrv::PAUSE );
  #endif
    }
  }
}

void MainWindow::Stop( bool notify )
{
  if( _player )
  {
    _player->Stop( );
    _playButton->setIcon( _playIcon );
    _startTimeLabel->setText(
          QString::number( (double)_player ->startTime( )));
    _playing = false;
    if( notify )
    {
#ifdef VISIMPL_USE_ZEROEQ
      _player ->zeqEvents( )->sendPlaybackOp( zeroeq::gmrv::STOP );
#endif
    }

  }
}

void MainWindow::Repeat( bool notify )
{
  if( _player )
  {
    bool repeat = _repeatButton->isChecked( );
    _player->loop( repeat );

    if( notify )
    {
#ifdef VISIMPL_USE_ZEROEQ
      _player ->zeqEvents( )->sendPlaybackOp( repeat ?
                                  zeroeq::gmrv::ENABLE_LOOP :
                                  zeroeq::gmrv::DISABLE_LOOP );
#endif
    }

  }
}

void MainWindow::PlayAt( bool notify )
{
  if( _player )
  {
    PlayAt( _simSlider->sliderPosition( ), notify );
  }
}

void MainWindow::PlayAt( float percentage, bool notify )
{
  if( _player )
  {
    int sliderPos = percentage *
                    ( _simSlider->maximum( ) - _simSlider->minimum( )) +
                    _simSlider->minimum( );

    PlayAt( sliderPos, notify );
  }
}

void MainWindow::PlayAt( int sliderPosition, bool notify )
{
  if( _player )
  {

    int value = _simSlider->value( );
    float percentage = float( value - _simSlider->minimum( )) /
                       float( _simSlider->maximum( ) - _simSlider->minimum( ));
    _simSlider->setSliderPosition( sliderPosition );

    _playButton->setIcon( _pauseIcon );

    _player->PlayAt( percentage );
    _playing = true;

    if( notify )
    {
#ifdef VISIMPL_USE_ZEROEQ
    // Send event
    _player ->zeqEvents( )->sendFrame( _simSlider->minimum( ),
                           _simSlider->maximum( ),
                           sliderPosition );

    _player ->zeqEvents( )->sendPlaybackOp( zeroeq::gmrv::PLAY );
#endif
    }
  }
}

void MainWindow::Restart( bool notify )
{
  if( _player )
  {
    bool playing = _playing;
    _player->Stop( );
    _playing = false;
    if( playing )
    {
     _player->Play( );
     _playing = true;
    }
    if( _playing )
      _playButton->setIcon( _pauseIcon );
    else
      _playButton->setIcon( _playIcon );

    if( notify )
    {
#ifdef VISIMPL_USE_ZEROEQ
    _player->zeqEvents( )->sendPlaybackOp( zeroeq::gmrv::BEGIN );
#endif
    }

  }
}

void MainWindow::GoToEnd( bool notify )
{
  if( _player )
  {
    //TODO implement GOTOEND

    if( notify )
    {
#ifdef VISIMPL_USE_ZEROEQ
    _player ->zeqEvents( )->sendPlaybackOp( zeroeq::gmrv::END );
#endif
    }
  }
}

void MainWindow::UpdateSimulationSlider( float percentage )
{
  double currentTime = percentage * ( _player->endTime( ) - _player->startTime( )) + _player->startTime( );

  _startTimeLabel->setText(

  QString::number( currentTime ));

  int total = _simSlider->maximum( ) - _simSlider->minimum( );

  int position = percentage * total;

  _simSlider->setSliderPosition( position );
}


#ifdef VISIMPL_USE_ZEROEQ

#ifdef VISIMPL_USE_GMRVLEX

  void MainWindow::ApplyPlaybackOperation( unsigned int playbackOp )
  {
    zeroeq::gmrv::PlaybackOperation operation =
        ( zeroeq::gmrv::PlaybackOperation ) playbackOp;

    switch( operation )
    {
      case zeroeq::gmrv::PLAY:
//        std::cout << "Received play" << std::endl;
        Play( false );
        break;
      case zeroeq::gmrv::PAUSE:
        Pause( false );
//        std::cout << "Received pause" << std::endl;
        break;
      case zeroeq::gmrv::STOP:
//        std::cout << "Received stop" << std::endl;
        Stop( false );
        break;
      case zeroeq::gmrv::BEGIN:
//        std::cout << "Received begin" << std::endl;
        Restart( false );
        break;
      case zeroeq::gmrv::END:
//        std::cout << "Received end" << std::endl;
        GoToEnd( false );
        break;
      case zeroeq::gmrv::ENABLE_LOOP:
//        std::cout << "Received enable loop" << std::endl;
        _zeqEventRepeat( true );
        break;
      case zeroeq::gmrv::DISABLE_LOOP:
//        std::cout << "Received disable loop" << std::endl;
        _zeqEventRepeat( false );
        break;
      default:
        break;
    }

  }

  void MainWindow::_zeqEventRepeat( bool repeat )
  {
    _repeatButton->setChecked( repeat );
  }

  void MainWindow::HistogramClicked( visimpl::MultiLevelHistogram* histogram )
  {
    const GIDUSet* selection;

    if( histogram->filteredGIDs( ).size( ) == 0)
      selection = &_summary->gids( );
    else
      selection = &histogram->filteredGIDs( );

    std::vector< uint32_t > selected ( selection->begin( ),
                                   selection->end( ));

    std::cout << "Publishing selection of " << selected.size( ) << std::endl;
//    std::vector< uint32_t > se( {1, 2, 3} );

//     selectedIds( selected );
//    selectedIds.setIds( selected );

//    std::cout << "Sending selection of size " << selected.size( ) << std::endl;
    _publisher->publish( lexis::data::SelectedIDs( selected ));
  }

#endif

void MainWindow::_setZeqUri( const std::string& uri_ )
{
  _zeqUri = uri_.empty( ) ? zeroeq::DEFAULT_SESSION : uri_;

  _zeqConnection = true;
  _subscriber = new zeroeq::Subscriber( _zeqUri );
  _publisher = new zeroeq::Publisher( _zeqUri );

  _subscriber->subscribe(
      lexis::data::SelectedIDs::ZEROBUF_TYPE_IDENTIFIER( ),
      [&]( const void* data_, const size_t size_ )
      { _onSelectionEvent( lexis::data::SelectedIDs::create( data_, size_ ));});

//  pthread_create( &_subscriberThread, NULL, _Subscriber, _subscriber );
  _thread = new std::thread( [&]() { while( true ) _subscriber->receive( 10000 );});

}

//void* MainWindow::_Subscriber( void* subs )
//{
//  zeroeq::Subscriber* subscriber = static_cast< zeroeq::Subscriber* >( subs );
//  while ( true )
//  {
//    subscriber->receive( 10000 );
//  }
//  pthread_exit( NULL );
//}

void MainWindow::_onSelectionEvent( lexis::data::ConstSelectedIDsPtr selected )
{

//  std::vector< unsigned int > selected =
//      zeroeq::hbp::deserializeSelectedIDs( event_ );
  std::vector< uint32_t > ids = std::move( selected->getIdsVector( ));
  std::cout << "Received selection with " << ids.size( ) << " elements" << std::endl;

  GIDUSet selectedSet( ids.begin( ), ids.end( ));

  if( _summary )
  {
    visimpl::Selection selection;

    selection.gids = selectedSet;

    _summary->AddNewHistogram( selection, true );
  }

}

#endif

void MainWindow::resizeEvent( QResizeEvent * event_ )
{
  QMainWindow::resizeEvent( event_ );

  if( resizingEnabled )
  {
//    unsigned int columnsWidth = width( ) / _columnsNumber;
//    unsigned int currentWidth = width( ) - columnsWidth;
//    unsigned int currentHeight =  _summary->heightPerRow( ) *
//                                  _summary->histogramsNumber( );
//
//    _contentWidget->setMinimumWidth( width( ) );
//    _contentWidget->setMinimumHeight( currentHeight + 2 );
//
//    std::cout << width( ) << " -> " << currentWidth << std::endl;
////    _summary->setMinimumWidth( currentWidth );
//    _summary->resize( currentWidth, currentHeight );
  }
}

void MainWindow::loadComplete( void )
{
  resizingEnabled = true;
  _summary->showMarker( false );
}


//void MainWindow::updateStackView( void )
//{
//  for( unsigned int i = 0 ; i < _summary->histogramsNumber( ); i++)
//  {
//    _summary->createSelection( i );
//  }
//}
