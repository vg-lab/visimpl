#include "ui_visimpl.h"
#include "MainWindow.h"
#include <QDebug>
#include <QFileDialog>
#include <QInputDialog>
#include <QGridLayout>
#include <QGroupBox>
// #include "qt/CustomSlider.h"

#ifdef VISIMPL_USE_GMRVZEQ
  #include <gmrvzeq/gmrvzeq.h>
#endif

MainWindow::MainWindow( QWidget* parent_,
                        bool updateOnIdle )
  : QMainWindow( parent_ )
  , _lastOpenedFileName( "" )
  , _ui( new Ui::MainWindow )
  , _openGLWidget( nullptr )
{
  _ui->setupUi( this );

  _ui->actionUpdateOnIdle->setChecked( updateOnIdle );
  _ui->actionPaintNeurons->setChecked( true );
  _ui->actionShowFPSOnIdleUpdate->setChecked( false );

#ifdef VISIMPL_USE_BRION
  _ui->actionOpenBlueConfig->setEnabled( true );
#else
  _ui->actionOpenBlueConfig->setEnabled( false );
#endif

  connect( _ui->actionQuit, SIGNAL( triggered( )),
           QApplication::instance(), SLOT( quit( )));

}

void MainWindow::init( const std::string& zeqUri )
{

  _openGLWidget = new OpenGLWidget( 0, 0,
                                    _ui->actionPaintNeurons->isChecked( ),
                                    zeqUri );
  this->setCentralWidget( _openGLWidget );
  qDebug( ) << _openGLWidget->format( );

  _openGLWidget->idleUpdate( _ui->actionUpdateOnIdle->isChecked( ));

  connect( _ui->actionUpdateOnIdle, SIGNAL( triggered( )),
           _openGLWidget, SLOT( toggleUpdateOnIdle( )));

  connect( _ui->actionPaintNeurons, SIGNAL( triggered( )),
           _openGLWidget, SLOT( togglePaintNeurons( )));

  connect( _ui->actionBackgroundColor, SIGNAL( triggered( )),
           _openGLWidget, SLOT( changeClearColor( )));

  connect( _ui->actionShowFPSOnIdleUpdate, SIGNAL( triggered( )),
           _openGLWidget, SLOT( toggleShowFPS( )));

  connect( _ui->actionWireframe, SIGNAL( triggered( )),
           _openGLWidget, SLOT( toggleWireframe( )));

  connect( _ui->actionOpenBlueConfig, SIGNAL( triggered( )),
           this, SLOT( openBlueConfigThroughDialog( )));

  connect( _ui->actionOpenXMLScene, SIGNAL( triggered( )),
           this, SLOT( openXMLSceneThroughDialog( )));

  connect( _ui->actionOpenSWCFile, SIGNAL( triggered( )),
           this, SLOT( openSWCFileThroughDialog( )));


  initPlaybackDock( );
  initSimColorDock( );

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
                                 const std::string& reportLabel)
{
  _openGLWidget->loadData( fileName,
                           OpenGLWidget::TDataFileType::tBlueConfig,
                           simulationType, reportLabel );


  connect( _openGLWidget, SIGNAL( updateSlider( float )),
           this, SLOT( UpdateSimulationSlider( float )));


  _startTimeLabel->setText(
      QString::number( (double)_openGLWidget->player( )->startTime( )));

  _endTimeLabel->setText(
        QString::number( (double)_openGLWidget->player( )->endTime( )));

  _openGLWidget->player( )->zeqEvents( )->playbackOpReceived.connect(
      boost::bind( &MainWindow::ApplyPlaybackOperation, this, _1 ));

  changeEditorColorMapping( );
  changeEditorSizeFunction( );
  changeEditorDecayValue( );
  initSummaryWidget( );
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
    visimpl::TSimulationType simType;
    QStringList items = {"Spikes", "Voltages"};

    QString text = QInputDialog::getItem(
      this, tr( "Please select simulation type" ),
      tr( "Type:" ), items, 0, false, &ok1 );

    if( !ok1 )
      return;

    if( text == items[0] )
    {
      simType = visimpl::TSpikes;
      ok2 = true;
    }
    else
    {
      simType = visimpl::TVoltages;

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


void MainWindow::openXMLScene( const std::string& fileName )
{
  _openGLWidget->loadData( fileName,
    OpenGLWidget::TDataFileType::NsolScene );
}


void MainWindow::openXMLSceneThroughDialog( void )
{
#ifdef NSOL_USE_QT5CORE
  QString path = QFileDialog::getOpenFileName(
    this, tr( "Open XML Scene" ), _lastOpenedFileName,
    tr( "XML ( *.xml);; All files (*)" ), nullptr,
    QFileDialog::DontUseNativeDialog );

  if ( path != QString( "" ))
  {
    std::string fileName = path.toStdString( );
    openXMLScene( fileName );
  }
#endif

}


void MainWindow::openSWCFile( const std::string& fileName )
{
  _openGLWidget->loadData( fileName,
    OpenGLWidget::TDataFileType::SWC );
}


void MainWindow::openSWCFileThroughDialog( void )
{
  QString path = QFileDialog::getOpenFileName(
    this, tr( "Open Swc File" ), _lastOpenedFileName,
    tr( "swc ( *.swc);; All files (*)" ), nullptr,
    QFileDialog::DontUseNativeDialog );

  if ( path != QString( "" ))
  {
    std::string fileName = path.toStdString( );
    openSWCFile( fileName );
  }

}

void MainWindow::initPlaybackDock( void )
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

//  connect( _simSlider, SIGNAL( sliderMoved( )),
//             this, SLOT( PlayAt( )));

  _summary = new Summary( nullptr, Summary::T_STACK_FIXED );
//  _summary->setVisible( false );
  _summary->setMinimumHeight( 50 );

  dockLayout->addWidget( _summary, 0, 1, 2, totalHSpan - 3 );

  _simulationDock->setWidget( content );
  this->addDockWidget( Qt::/*DockWidgetAreas::enum_type::*/BottomDockWidgetArea,
                       _simulationDock );
}

void MainWindow::initSimColorDock( void )
{
  _simConfigurationDock = new QDockWidget( );
  _simConfigurationDock->setMinimumHeight( 100 );
  _simConfigurationDock->setMinimumWidth( 300 );
//  _simConfigurationDock->setMaximumHeight( 400 );
  _simConfigurationDock->setSizePolicy( QSizePolicy::MinimumExpanding,
                                  QSizePolicy::MinimumExpanding );

//  _tfEditor = new TransferFunctionEditor( );
  _tfWidget = new TransferFunctionWidget( );
  _tfWidget->setMinimumHeight( 100 );

  _psWidget = new ParticleSizeWidget( );
  _psWidget->setMinimumHeight( 150 );

  _decayBox = new QDoubleSpinBox( );
  _decayBox->setMinimum( 0.01 );
  _decayBox->setMaximum( 600.0 );

  _alphaNormalButton = new QRadioButton( "Normal" );
  _alphaAccumulativeButton = new QRadioButton( "Accumulative" );
  _openGLWidget->SetAlphaBlendingAccumulative( false );

  QWidget* container = new QWidget( );
  QVBoxLayout* verticalLayout = new QVBoxLayout( );
//  QPushButton* applyColorButton = new QPushButton( QString( "Apply" ));

  QGroupBox* tFunctionGB = new QGroupBox( "Transfer function" );
  QVBoxLayout* tfLayout = new QVBoxLayout( );
  tfLayout->addWidget( _tfWidget );
  tFunctionGB->setLayout( tfLayout );
  tFunctionGB->setMaximumHeight( 130 );

  QGroupBox* sFunctionGB = new QGroupBox( "Size function" );
  QVBoxLayout* sfLayout = new QVBoxLayout( );
  sfLayout->addWidget( _psWidget);
  sFunctionGB->setLayout( sfLayout );
  sFunctionGB->setMaximumHeight( 200 );

  QGroupBox* dFunctionGB = new QGroupBox( "Decay function" );
  QHBoxLayout* dfLayout = new QHBoxLayout( );
  dfLayout->addWidget( new QLabel( "Decay \n(simulation time): " ));
  dfLayout->addWidget( _decayBox );
  dFunctionGB->setLayout( dfLayout );
  dFunctionGB->setMaximumHeight( 200 );

  QGroupBox* rFunctionGB = new QGroupBox( "Alpha blending function" );
  QHBoxLayout* rfLayout = new QHBoxLayout( );
  rfLayout->addWidget( new QLabel( "Alhpa\nBlending: " ));
  rfLayout->addWidget( _alphaNormalButton );
  rfLayout->addWidget( _alphaAccumulativeButton );
  rFunctionGB->setLayout( rfLayout );
  rFunctionGB->setMaximumHeight( 200 );


  verticalLayout->setAlignment( Qt::AlignTop );
  verticalLayout->addWidget( tFunctionGB );
  verticalLayout->addWidget( sFunctionGB );
  verticalLayout->addWidget( dFunctionGB );
  verticalLayout->addWidget( rFunctionGB );


//  verticalLayout->addWidget( new QLabel( "Transfer function" ));
//
//  verticalLayout->addWidget( _tfWidget );
//
//  verticalLayout->addWidget( new QLabel( "Size function" ));
//  verticalLayout->addWidget( _psWidget ) ;

  container->setLayout( verticalLayout );
  _simConfigurationDock->setWidget( container );

  this->addDockWidget( Qt::/*DockWidgetAreas::enum_type::*/RightDockWidgetArea,
                       _simConfigurationDock );

//  connect( applyColorButton, SIGNAL( clicked( void )),
//             this, SLOT( UpdateSimulationColorMapping( void )));

  connect( _tfWidget, SIGNAL( colorChanged( void )),
           this, SLOT( UpdateSimulationColorMapping( void )));
  connect( _tfWidget, SIGNAL( previewColor( void )),
           this, SLOT( PreviewSimulationColorMapping( void )));

  connect( _psWidget, SIGNAL( sizeChanged( void )),
           this, SLOT( UpdateSimulationSizeFunction( void )));

  connect( _psWidget, SIGNAL( sizePreview( void )),
           this, SLOT( PreviewSimulationSizeFunction( void )));

  connect( _decayBox, SIGNAL( valueChanged( double )),
           this, SLOT( UpdateSimulationDecayValue( void )));

  connect( _alphaNormalButton, SIGNAL( toggled( bool )),
           this, SLOT( AlphaBlendingToggled( void ) ));

//  connect( _alphaAccumulativeButton, SIGNAL( toggled( bool )),
//           this, SLOT( AlphaBlendingToggled( void ) ));

  _alphaNormalButton->setChecked( true );
}

void MainWindow::initSummaryWidget( void )
{
  visimpl::TSimulationType simType =
      _openGLWidget->player( )->simulationType( );

  if( simType == visimpl::TSpikes )
  {
    visimpl::SpikesPlayer* spikesPlayer =
        dynamic_cast< visimpl::SpikesPlayer* >( _openGLWidget->player( ));

    std::cout << "Creating summary..." << std::endl;
//    GIDUSet gids;
//    _summary->AddGIDSelection( gids );
    _summary->CreateSummary( spikesPlayer->spikeReport( ),
                             spikesPlayer->gids( ));
//    _summary->setVisible( true );

  }
}

void MainWindow::PlayPause( bool notify )
{
  if( !_openGLWidget->player( )->isPlaying( ))
    Play( notify );
  else
    Pause( notify );
}

void MainWindow::Play( bool notify )
{
//  playIcon.swap( pauseIcon );

  if( _openGLWidget )
  {
    _openGLWidget->Play( );

    _playButton->setIcon( _pauseIcon );

    if( notify )
    {
#ifdef VISIMPL_USE_ZEQ
      _openGLWidget->player( )->zeqEvents( )->sendPlaybackOp( zeq::gmrv::PLAY );
#endif
    }
  }
}

void MainWindow::Pause( bool notify )
{
  if( _openGLWidget )
  {
    _openGLWidget->Pause( );
    _playButton->setIcon( _playIcon );

    if( notify )
    {
#ifdef VISIMPL_USE_ZEQ
    _openGLWidget->player( )->zeqEvents( )->sendPlaybackOp( zeq::gmrv::PAUSE );
#endif
    }
  }
}

void MainWindow::Stop( bool notify )
{
  if( _openGLWidget )
  {
    _openGLWidget->Stop( );
    _playButton->setIcon( _playIcon );
    _startTimeLabel->setText(
          QString::number( (double)_openGLWidget->player( )->startTime( )));

    if( notify )
    {
#ifdef VISIMPL_USE_ZEQ
      _openGLWidget->player( )->zeqEvents( )->sendPlaybackOp( zeq::gmrv::STOP );
#endif
    }

  }
}

void MainWindow::Repeat( bool notify )
{
  if( _openGLWidget )
  {
    bool repeat = _repeatButton->isChecked( );
//    std::cout << "Repeat " << boolalpha << repeat << std::endl;
    _openGLWidget->Repeat( repeat );

    if( notify )
    {
#ifdef VISIMPL_USE_ZEQ
      _openGLWidget->player( )->zeqEvents( )->sendPlaybackOp( repeat ?
                                  zeq::gmrv::ENABLE_LOOP :
                                  zeq::gmrv::DISABLE_LOOP );
#endif
    }

  }
}

void MainWindow::PlayAt( bool notify )
{
  if( _openGLWidget )
  {
    PlayAt( _simSlider->sliderPosition( ), notify );
  }
}

void MainWindow::PlayAt( int sliderPosition, bool notify )
{
  if( _openGLWidget )
  {

//    _openGLWidget->Pause( );
    _openGLWidget->resetParticles( );

    int value = _simSlider->value( );
    float percentage = float( value - _simSlider->minimum( )) /
                       float( _simSlider->maximum( ) - _simSlider->minimum( ));
    _simSlider->setSliderPosition( sliderPosition );

    _playButton->setIcon( _pauseIcon );

    _openGLWidget->PlayAt( percentage );

    if( notify )
    {
#ifdef VISIMPL_USE_ZEQ
    // Send event
    _openGLWidget->player( )->zeqEvents( )->sendFrame( _simSlider->minimum( ),
                           _simSlider->maximum( ),
                           sliderPosition );

    _openGLWidget->player( )->zeqEvents( )->sendPlaybackOp( zeq::gmrv::PLAY );
#endif
    }
  }
}

void MainWindow::Restart( bool notify )
{
  if( _openGLWidget )
  {
    _openGLWidget->Restart( );

    if( _openGLWidget->player( )->isPlaying( ))
      _playButton->setIcon( _pauseIcon );
    else
      _playButton->setIcon( _playIcon );

    if( notify )
    {
#ifdef VISIMPL_USE_ZEQ
    _openGLWidget->player( )->zeqEvents( )->sendPlaybackOp( zeq::gmrv::BEGIN );
#endif
    }

  }
}

void MainWindow::GoToEnd( bool notify )
{
  if( _openGLWidget )
  {
    _openGLWidget->GoToEnd( );

    if( notify )
    {
#ifdef VISIMPL_USE_ZEQ
    _openGLWidget->player( )->zeqEvents( )->sendPlaybackOp( zeq::gmrv::END );
#endif
    }
  }
}

void MainWindow::UpdateSimulationSlider( float percentage )
{
//  if( _openGLWidget->player( )->isPlaying( ))
  {

    _startTimeLabel->setText(
          QString::number( (double)_openGLWidget->player( )->currentTime( )));

//    float percentage = _openGLWidget->player( )->GetRelativeTime( );

    int total = _simSlider->maximum( ) - _simSlider->minimum( );

    int position = percentage * total;
//    std::cout << "Timer: " << percentage << " * "
//              << total << " = " << position << std::endl;

    _simSlider->setSliderPosition( position );
  }
}

void MainWindow::UpdateSimulationColorMapping( void )
{
//  TTransferFunction& colors = _tfEditor->getColorPoints( );

//  _openGLWidget->changeSimulationColorMapping( _tfEditor->getColorPoints( ));
  _openGLWidget->changeSimulationColorMapping( _tfWidget->getColors( ));
  _psWidget->SetFrameBackground( _tfWidget->getColors( false ));
}

void MainWindow::PreviewSimulationColorMapping( void )
{
  _openGLWidget->changeSimulationColorMapping( _tfWidget->getPreviewColors( ));
}

void MainWindow::changeEditorColorMapping( void )
{
//  _tfEditor->setColorPoints( _openGLWidget->getSimulationColorMapping( ));
  _tfWidget->setColorPoints( _openGLWidget->getSimulationColorMapping( ));
  _psWidget->SetFrameBackground( _tfWidget->getColors( false ));
}

void MainWindow::changeEditorSizeFunction( void )
{
  _psWidget->setSizeFunction( _openGLWidget->getSimulationSizeFunction() );
}

void MainWindow::UpdateSimulationSizeFunction( void )
{
  _openGLWidget->changeSimulationSizeFunction( _psWidget->getSizeFunction( ));
}

void MainWindow::PreviewSimulationSizeFunction( void )
{
  _openGLWidget->changeSimulationSizeFunction( _psWidget->getSizePreview( ));
}

void MainWindow::changeEditorDecayValue( void )
{
  _decayBox->setValue( _openGLWidget->getSimulationDecayValue( ));
}

void MainWindow::UpdateSimulationDecayValue( void )
{
  _openGLWidget->changeSimulationDecayValue( _decayBox->value( ));
}

void MainWindow::AlphaBlendingToggled( void )
{
  std::cout << "Changing alpha blending... ";
  if( _alphaNormalButton->isChecked( ))
  {
    std::cout << "Normal" << std::endl;
    _openGLWidget->SetAlphaBlendingAccumulative( false );
  }
  else
  {
    std::cout << "Accumulative" << std::endl;
    _openGLWidget->SetAlphaBlendingAccumulative( true );
  }
}


#ifdef VISIMPL_USE_ZEQ

#ifdef VISIMPL_USE_GMRVZEQ

  void MainWindow::ApplyPlaybackOperation( unsigned int playbackOp )
  {
    zeq::gmrv::PlaybackOperation operation =
        ( zeq::gmrv::PlaybackOperation ) playbackOp;

    switch( operation )
    {
      case zeq::gmrv::PLAY:
//        std::cout << "Received play" << std::endl;
        Play( false );
        break;
      case zeq::gmrv::PAUSE:
        Pause( false );
//        std::cout << "Received pause" << std::endl;
        break;
      case zeq::gmrv::STOP:
//        std::cout << "Received stop" << std::endl;
        Stop( false );
        break;
      case zeq::gmrv::BEGIN:
//        std::cout << "Received begin" << std::endl;
        Restart( false );
        break;
      case zeq::gmrv::END:
//        std::cout << "Received end" << std::endl;
        GoToEnd( false );
        break;
      case zeq::gmrv::ENABLE_LOOP:
//        std::cout << "Received enable loop" << std::endl;
        _zeqEventRepeat( true );
        break;
      case zeq::gmrv::DISABLE_LOOP:
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
    Repeat( false );
  }

#endif

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

  _openGLWidget->setSelectedGIDs( selectedSet );

  if( _summary )
  {
    _summary->AddGIDSelection( selectedSet );
  }

}

#endif
