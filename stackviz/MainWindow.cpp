/*
 * Copyright (c) 2015-2020 VG-Lab/URJC.
 *
 * Authors: Sergio E. Galindo <sergio.galindo@urjc.es>
 *
 * This file is part of ViSimpl <https://github.com/vg-lab/visimpl>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3.0 as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#ifdef VISIMPL_USE_GMRVLEX
#include <gmrvlex/version.h>
#endif
#ifdef VISIMPL_USE_ZEROEQ
#include <zeroeq/version.h>
#endif
#ifdef VISIMPL_USE_RETO

#include <reto/version.h>

#endif
#ifdef VISIMPL_USE_SCOOP

#include <scoop/version.h>

#endif
#ifdef VISIMPL_USE_SIMIL

#include <simil/version.h>

#endif
#ifdef VISIMPL_USE_PREFR
#include <prefr/version.h>
#endif

#include <stackviz/version.h>

#include "MainWindow.h"
#include <QDebug>
#include <QFileDialog>
#include <QInputDialog>
#include <QGridLayout>
#include <QInputDialog>
#include <QMessageBox>
#include <QShortcut>
#include <QDateTime>
#include <QtGlobal>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <boost/bind.hpp>

#include <thread>

#include <acuterecorder/acuterecorder.h>

#include <sumrice/sumrice.h>
#include <sumrice/Utils.h>

using namespace stackviz;

template< class T >
void ignore( const T& )
{ }

constexpr int SLIDER_MAX = 1000;

MainWindow::MainWindow( QWidget* parent_ )
  : QMainWindow( parent_ )
  , _ui( new Ui::MainWindow )
  , _simulationType( simil::TSimNetwork )
  , _summary( nullptr )
  , _player( nullptr )
  , _subsetEventManager( nullptr )
  , _autoCalculateCorrelations( false )
  , _dockSimulation( nullptr )
  , _playButton( nullptr )
  , _simSlider( nullptr )
  , _repeatButton( nullptr )
  , _goToButton( nullptr )
  , _playing( false )
  , _playIcon( ":/icons/play.svg" )
  , _pauseIcon( ":/icons/pause.svg" )
  , _startTimeLabel( nullptr )
  , _endTimeLabel( nullptr )
  , _displayManager( nullptr )
  , m_loader{ nullptr }
  , m_loaderDialog{ nullptr }
  , m_dataInspector{ nullptr }
  , _recorder{ nullptr }
#ifdef SIMIL_WITH_REST_API
  , _restConnectionInformation( )
  , _alreadyConnected( false )
#endif
{
  _ui->setupUi( this );

  auto recorderAction = RecorderUtils::recorderAction( );
  _ui->menuTools->insertAction( _ui->menuTools->actions( ).first( ) ,
                                recorderAction );
  _ui->toolBar->addAction( recorderAction );

  connect( recorderAction , SIGNAL( triggered( bool )) ,
           this , SLOT( openRecorder( )));

#ifdef VISIMPL_USE_SIMIL
  _ui->actionOpenBlueConfig->setEnabled( true );
#else
  _ui->actionOpenBlueConfig->setEnabled( false );
#endif

  connect( _ui->actionQuit , SIGNAL( triggered( void )) ,
           QApplication::instance( ) , SLOT( quit( void )));

  // Connect about dialog
  connect( _ui->actionAbout , SIGNAL( triggered( void )) ,
           this , SLOT( aboutDialog( void )));

#ifdef SIMIL_WITH_REST_API
  _ui->actionConnectRESTserver->setEnabled( true );
#endif

  m_dataInspector = new DataInspector( "" );
  m_dataInspector->hide( );
}

void MainWindow::init( const std::string& session )
{
#ifdef VISIMPL_USE_ZEROEQ
  const auto session_ = session.empty() ? zeroeq::DEFAULT_SESSION : session;

  try
  {
    auto &zInstance = visimpl::ZeroEQConfig::instance();
    if(!zInstance.isConnected())
    {
      zInstance.connect(session_);
    }

    if(zInstance.isConnected())
    {
      zInstance.subscriber()->subscribe(lexis::data::SelectedIDs::ZEROBUF_TYPE_IDENTIFIER(),
                                        [&](const void* data_, unsigned long long size_)
                                        { _onSelectionEvent(lexis::data::SelectedIDs::create(data_,  size_));});
    }
  }
  catch ( std::exception& e )
  {
    std::cerr << "Exception when initializing ZeroEQ. ";
    std::cerr << e.what( ) << __FILE__ << ":" << __LINE__ << std::endl;
  }
  catch ( ... )
  {
    std::cerr << "Unknown exception when initializing ZeroEQ. " << __FILE__
              << ":" << __LINE__ << std::endl;
  }
#endif

  connect( _ui->actionOpenBlueConfig , SIGNAL( triggered( void )) ,
           this , SLOT( openBlueConfigThroughDialog( void )));

  connect( _ui->actionOpenCSVFiles , SIGNAL( triggered( void )) ,
           this , SLOT( openCSVFilesThroughDialog( void )));

  connect( _ui->actionOpenH5Files , SIGNAL( triggered( void )) ,
           this , SLOT( openH5FilesThroughDialog( void )));

  connect( _ui->actionConnectRESTserver , SIGNAL( triggered( void )) , this ,
           SLOT( openRESTThroughDialog( )));

  connect( _ui->actionOpenSubsetEventsFile , SIGNAL( triggered( void )) ,
           this , SLOT( openSubsetEventsFileThroughDialog( void )));

  connect( _ui->actionOpenGroupsFile , SIGNAL( triggered( void )) ,
           this , SLOT( openGroupsThroughDialog( void )));

  connect( _ui->actionCloseData , SIGNAL( triggered( bool )) ,
           this , SLOT( closeData( )));

  _ui->actionOpenSubsetEventsFile->setEnabled( false );
  _ui->actionOpenGroupsFile->setEnabled( false );

  initPlaybackDock( );

  connect( _dockSimulation->toggleViewAction( ) , SIGNAL( toggled( bool )) ,
           _ui->actionTogglePlaybackDock , SLOT( setChecked( bool )));

  connect( _ui->actionTogglePlaybackDock , SIGNAL( triggered( void )) ,
           this , SLOT( togglePlaybackDock( void )));

  connect( _ui->actionShowDataManager , SIGNAL( triggered( void )) ,
           this , SLOT( showDisplayManagerWidget( void )));

#ifdef VISIMPL_USE_ZEROEQ
  auto &zInst = visimpl::ZeroEQConfig::instance();
  if(zInst.isConnected())
    zInst.startReceiveLoop();

  _ui->actionTogglePlaybackDock->setChecked( true );

#else
  // to avoid compilation warnings about unused parameter.
  ignore( session );
#endif

  _ui->toolBar->setContextMenuPolicy( Qt::PreventContextMenu );
  _ui->menubar->setContextMenuPolicy( Qt::PreventContextMenu );

  _ui->actionShowDataManager->setEnabled( false );
  _ui->actionCloseData->setEnabled( false );
}

MainWindow::~MainWindow( void )
{
  delete _ui;

#ifdef VISIMPL_USE_ZEROEQ

  auto &zInstance = visimpl::ZeroEQConfig::instance();
  if(zInstance.isConnected())
  {
    zInstance.disconnect();
  }

#endif
}

void MainWindow::showStatusBarMessage( const QString& message )
{
  _ui->statusbar->showMessage( message );
}

void
MainWindow::openSubsetEventFile( const std::string& filePath , bool append )
{
  if ( filePath.empty( ) || !_subsetEventManager ) return;

  QApplication::setOverrideCursor( Qt::WaitCursor );

  if ( !append ) _subsetEventManager->clear( );

  _summary->clearEvents( );

  QString errorText;
  try
  {
    if ( filePath.find( "json" ) != std::string::npos )
    {
      _subsetEventManager->loadJSON( filePath );
    }
    else if ( filePath.find( "h5" ) != std::string::npos ||
              filePath.find( "hdf5" ) != std::string::npos )
    {
      _subsetEventManager->loadH5( filePath );
      _autoCalculateCorrelations = true;
    }
    else
    {
      errorText = tr( "Subset Events file not found: %1" ).arg(
        QString::fromStdString( filePath ));
    }
  }
  catch ( const std::exception& e )
  {
    if ( _subsetEventManager ) _subsetEventManager->clear( );

    errorText = QString::fromLocal8Bit( e.what( ));
  }

  QApplication::restoreOverrideCursor( );

  if ( !errorText.isEmpty( ))
  {
    QMessageBox::critical( this , tr( "Error loading Events file" ) ,
                           errorText , QMessageBox::Ok );
    return;
  }
}

void MainWindow::openBlueConfigThroughDialog( void )
{
#ifdef VISIMPL_USE_SIMIL

  const QString filename = QFileDialog::getOpenFileName(
    this , tr( "Open BlueConfig" ) , _lastOpenedFileNamePath ,
    tr( "BlueConfig ( BlueConfig CircuitConfig);; All files (*)" ) ,
    nullptr , QFileDialog::DontUseNativeDialog );

  if ( !filename.isEmpty( ))
  {
    bool ok1 , ok2;
    QInputDialog simTypeDialog;
    simil::TSimulationType simType;
    const QStringList items = { "Spikes" };//, "Voltages"};

    QString text = QInputDialog::getItem(
      this , tr( "Please select simulation type" ) ,
      tr( "Type:" ) , items , 0 , false , &ok1 );

    if ( !ok1 )
      return;

    if ( text == items[ 0 ] )
    {
      simType = simil::TSimSpikes;
      ok2 = true;
    }
    else
    {
      simType = simil::TSimVoltages;

      text = QInputDialog::getText(
        this , tr( "Please select report" ) ,
        tr( "Report:" ) , QLineEdit::Normal ,
        "soma" , &ok2 );
    }

    if ( !ok1 || !ok2 || text.isEmpty( )) return;

    const auto target = QInputDialog::getText( this , tr( "Target" ) ,
                                               tr( "BlueConfig Target:" ) ,
                                               QLineEdit::Normal , "" , &ok1 );

    if ( ok1 && !target.isEmpty( ))
    {
      _lastOpenedFileNamePath = QFileInfo( filename ).path( );
      loadData( simil::TBlueConfig , filename.toStdString( ) ,
                target.toStdString( ) , simType );
    }
  }
#endif
}

void MainWindow::openCSVFilesThroughDialog( void )
{
  const QString networkFilename = QFileDialog::getOpenFileName(
    this , tr( "Open CSV Network description file" ) , _lastOpenedFileNamePath ,
    tr( "CSV (*.csv);; All files (*)" ) ,
    nullptr , QFileDialog::DontUseNativeDialog );

  if ( !networkFilename.isEmpty( ))
  {

    const QString activityFilename = QFileDialog::getOpenFileName(
      this , tr( "Open CSV Activity file" ) , _lastOpenedFileNamePath ,
      tr( "CSV (*.csv);; All files (*)" ) ,
      nullptr , QFileDialog::DontUseNativeDialog );

    if ( !activityFilename.isEmpty( ))
    {
      _lastOpenedFileNamePath = QFileInfo( networkFilename ).path( );
      loadData( simil::TCSV , networkFilename.toStdString( ) ,
                activityFilename.toStdString( ) , simil::TSimSpikes );
    }
  }
}

void MainWindow::openSubsetEventsFileThroughDialog( void )
{
  const QString eventsFilename = QFileDialog::getOpenFileName( this ,
                                                               tr(
                                                                 "Open file containing subsets/events data" ) ,
                                                               _lastOpenedSubsetsFileName ,
                                                               tr(
                                                                 "JSON (*.json);; hdf5 (*.h5 *.hdf5);; All files (*)" ) ,
                                                               nullptr ,
                                                               QFileDialog::DontUseNativeDialog );

  if ( !eventsFilename.isEmpty( ))
  {
    _lastOpenedSubsetsFileName = QFileInfo( eventsFilename ).path( );

    openSubsetEventFile( eventsFilename.toStdString( ) , false );

    _summary->generateEventsRep( );
    _summary->importSubsetsFromSubsetMngr( );

    if ( _displayManager )
      _displayManager->refresh( );
  }
}

void MainWindow::configurePlayer( void )
{
  _startTimeLabel->setText(
    QString::number( _player->startTime( ) , 'f' , 3 ));

  _endTimeLabel->setText(
    QString::number( _player->endTime( ) , 'f' , 3 ));

  m_dataInspector->setSimPlayer( _player );

#ifdef SIMIL_USE_ZEROEQ
  try
  {
    auto &zInstance = visimpl::ZeroEQConfig::instance();
    if(zInstance.isConnected())
    {
      _player->connectZeq(zInstance.subscriber(), zInstance.publisher());

      const auto eventMgr = _player->zeqEvents();
      if (eventMgr)
      {
        eventMgr->frameReceived.connect(
            boost::bind(&MainWindow::UpdateSimulationSlider, this, _1));
        eventMgr->playbackOpReceived.connect(
            boost::bind(&MainWindow::ApplyPlaybackOperation, this, _1));
      }
    }
  }
  catch (const std::exception &e)
  {
    std::cerr << "Exception when initializing player events. ";
    std::cerr << e.what() << std::endl << " " << __FILE__
              << ":" << __LINE__ << std::endl;
  }
  catch (...)
  {
    std::cerr << "Unknown exception when initializing player events. "
              << __FILE__ << ":" << __LINE__ << std::endl;
  }
#endif
}

void MainWindow::togglePlaybackDock( void )
{
  if ( _ui->actionTogglePlaybackDock->isChecked( ))
    _dockSimulation->show( );
  else
    _dockSimulation->close( );

  update( );
}

void MainWindow::showDisplayManagerWidget( void )
{
  if ( !_summary ) return;

  if ( !_displayManager )
  {
    _displayManager = new visimpl::DisplayManagerWidget( );
    _displayManager->init( _summary->eventWidgets( ) ,
                           _summary->histogramWidgets( ));

    connect( _displayManager ,
             SIGNAL( eventVisibilityChanged( unsigned int , bool )) ,
             _summary , SLOT( eventVisibility( unsigned int , bool )));

    connect( _displayManager ,
             SIGNAL( subsetVisibilityChanged( unsigned int , bool )) ,
             _summary , SLOT( subsetVisibility( unsigned int , bool )));

    connect( _displayManager , SIGNAL( removeEvent( unsigned int )) ,
             _summary , SLOT( removeEvent( unsigned int )));

    connect( _displayManager , SIGNAL( removeHistogram( unsigned int )) ,
             _summary , SLOT( removeSubset( unsigned int )));

  }

  _displayManager->refresh( );

  _displayManager->show( );
}

void MainWindow::initPlaybackDock( )
{
  _dockSimulation = new QDockWidget( );
  _dockSimulation->setMinimumHeight( 100 );
  _dockSimulation->setSizePolicy( QSizePolicy::MinimumExpanding ,
                                  QSizePolicy::MinimumExpanding );

  _playing = false;
  constexpr unsigned int totalHSpan = 20;

  auto content = new QWidget( );
  auto dockLayout = new QGridLayout( );
  content->setLayout( dockLayout );

  _simSlider = new CustomSlider( Qt::Horizontal );
  _simSlider->setMinimum( 0 );
  _simSlider->setMaximum( SLIDER_MAX );
  _simSlider->setSizePolicy( QSizePolicy::Preferred ,
                             QSizePolicy::Preferred );

  _playButton = new QPushButton( _playIcon , tr( "" ));
  _playButton->setSizePolicy( QSizePolicy::MinimumExpanding ,
                              QSizePolicy::MinimumExpanding );
  auto stopButton = new QPushButton( QIcon( ":/icons/stop.svg" ) , tr( "" ));
  auto nextButton = new QPushButton( QIcon( ":/icons/next.svg" ) , tr( "" ));
  auto prevButton = new QPushButton( QIcon( ":/icons/previous.svg" ) ,
                                     tr( "" ));

  _repeatButton = new QPushButton( QIcon( ":/icons/repeat.svg" ) , tr( "" ));
  _repeatButton->setCheckable( true );
  _repeatButton->setChecked( false );

  _goToButton = new QPushButton( );
  _goToButton->setText( QString( "Play at..." ));

  _startTimeLabel = new QLabel( "" );
  _startTimeLabel->setSizePolicy( QSizePolicy::MinimumExpanding ,
                                  QSizePolicy::Preferred );
  _endTimeLabel = new QLabel( "" );
  _endTimeLabel->setSizePolicy( QSizePolicy::Preferred ,
                                QSizePolicy::Preferred );

  unsigned int row = 2;
  dockLayout->addWidget( _startTimeLabel , row , 0 , 1 , 2 );
  dockLayout->addWidget( _simSlider , row , 1 , 1 , totalHSpan - 3 );
  dockLayout->addWidget( _endTimeLabel , row , totalHSpan - 2 , 1 , 1 ,
                         Qt::AlignRight );

  row++;
  dockLayout->addWidget( _repeatButton , row , 7 , 1 , 1 );
  dockLayout->addWidget( prevButton , row , 8 , 1 , 1 );
  dockLayout->addWidget( _playButton , row , 9 , 2 , 2 );
  dockLayout->addWidget( stopButton , row , 11 , 1 , 1 );
  dockLayout->addWidget( nextButton , row , 12 , 1 , 1 );
  dockLayout->addWidget( _goToButton , row , 13 , 1 , 1 );

  connect( _playButton , SIGNAL( clicked( )) ,
           this , SLOT( PlayPause( )));

  connect( stopButton , SIGNAL( clicked( )) ,
           this , SLOT( Stop( )));

  connect( nextButton , SIGNAL( clicked( )) ,
           this , SLOT( GoToEnd( )));

  connect( prevButton , SIGNAL( clicked( )) ,
           this , SLOT( Restart( )));

  connect( _repeatButton , SIGNAL( clicked( )) ,
           this , SLOT( Repeat( )));

  connect( _simSlider , SIGNAL( sliderPressed( )) ,
           this , SLOT( PlayAtPosition( )));

  connect( _goToButton , SIGNAL( clicked( )) ,
           this , SLOT( playAtButtonClicked( )));

  _dockSimulation->setWidget( content );
  this->addDockWidget( Qt::BottomDockWidgetArea ,
                       _dockSimulation );

  _dockSimulation->setEnabled( false );
}

void MainWindow::initSummaryWidget( )
{
  _summary = new visimpl::Summary( this , visimpl::T_STACK_EXPANDABLE );

  if ( _simulationType == simil::TSimSpikes ||
       _simulationType == simil::TSimNetwork )
  {
    _summary->Init( _player->data( ));
    _summary->simulationPlayer( _player );
  }

  if ( centralWidget( ))
  {
    auto toRemove = centralWidget( );
    layout( )->removeWidget( toRemove );
    delete toRemove;
  }
  this->setCentralWidget( _summary );

  connect( _ui->actionAutoNamingSelections , SIGNAL( triggered( )) ,
           _summary , SLOT( toggleAutoNameSelections( )));

  _ui->actionFill_Plots->setChecked( true );
  connect( _ui->actionFill_Plots , SIGNAL( triggered( bool )) ,
           _summary , SLOT( fillPlots( bool )));

  connect( _ui->actionShowPanels , SIGNAL( triggered( bool )) ,
           _summary , SLOT( showConfigPanels( bool )));

  connect( _summary , SIGNAL( histogramClicked( float )) ,
           this , SLOT( PlayAtPercentage( float )));

  connect( m_dataInspector , SIGNAL( simDataChanged( )) ,
           _summary , SLOT( UpdateHistograms( )));

  connect( m_dataInspector , SIGNAL( simDataChanged( )) ,
           this , SLOT( onDataUpdated( )));

#ifdef VISIMPL_USE_ZEROEQ
  connect( _summary, SIGNAL( histogramClicked( visimpl::HistogramWidget* )),
             this, SLOT( HistogramClicked( visimpl::HistogramWidget* )));
#endif

  _ui->actionFocusOnPlayhead->setVisible( true );
  connect( _ui->actionFocusOnPlayhead , SIGNAL( triggered( )) ,
           _summary , SLOT( focusPlayback( )));

  if ( _autoCalculateCorrelations )
  {
    calculateCorrelations( );
  }

  QTimer::singleShot( 0 , _summary , SLOT( adjustSplittersSize( )));
}

void MainWindow::PlayPause( bool notify )
{
  if ( _playing )
    Pause( notify );
  else
    Play( notify );
}

void MainWindow::Play( bool notify )
{
  if ( _player )
  {
    _player->Play( );
    _playButton->setIcon( _pauseIcon );
    _playing = true;

    if ( notify )
    {
#ifdef VISIMPL_USE_GMRVLEX
      sendZeroEQPlaybackOperation(zeroeq::gmrv::PLAY);
#endif
    }
  }
}

void MainWindow::Pause( bool notify )
{
  if ( _player )
  {
    _player->Pause( );
    _playButton->setIcon( _playIcon );
    _playing = false;

    _startTimeLabel->setText(
      QString::number( _player->currentTime( ) , 'f' , 3 ));

    if ( notify )
    {
#ifdef VISIMPL_USE_GMRVLEX
      sendZeroEQPlaybackOperation(zeroeq::gmrv::PAUSE);
#endif
    }
  }
}

void MainWindow::Stop( bool notify )
{
  if ( _player )
  {
    _player->Stop( );
    _playButton->setIcon( _playIcon );
    _startTimeLabel->setText(
      QString::number( _player->startTime( ) , 'f' , 3 ));
    _playing = false;
    if ( notify )
    {
#ifdef VISIMPL_USE_GMRVLEX
      sendZeroEQPlaybackOperation(zeroeq::gmrv::STOP);
#endif
    }
  }
}

void MainWindow::Repeat( bool notify )
{
  if ( _player )
  {
    const bool repeat = _repeatButton->isChecked( );
    _player->loop( repeat );

    if ( notify )
    {
#ifdef VISIMPL_USE_GMRVLEX
      const auto op = repeat ? zeroeq::gmrv::ENABLE_LOOP : zeroeq::gmrv::DISABLE_LOOP;
      sendZeroEQPlaybackOperation(op);
#endif
    }
  }
}

void MainWindow::PlayAtPosition( bool notify )
{
  if ( _player )
  {
    PlayAtPosition( _simSlider->sliderPosition( ) , notify );
  }
}

void MainWindow::PlayAtPercentage( float percentage , bool notify )
{
  if ( _player )
  {
    const auto tBegin = _player->startTime( );
    const auto tEnd = _player->endTime( );
    const auto timePos = ( percentage * ( tEnd - tBegin )) + tBegin;

    PlayAtTime( timePos , notify );
  }
}

void MainWindow::PlayAtPosition( int sliderPosition , bool notify )
{
  if ( _player )
  {
    PlayAtPercentage( static_cast<float>(sliderPosition) / SLIDER_MAX ,
                      notify );
  }
}

void MainWindow::PlayAtTime( float timePos , bool notify )
{
  if ( _player )
  {
    const auto tBegin = _player->startTime( );
    const auto tEnd = _player->endTime( );
    const auto newTimePos = std::max( tBegin , std::min( tEnd , timePos ));
    const auto percentage = ( newTimePos - tBegin ) / ( tEnd - tBegin );

    _simSlider->setSliderPosition( percentage * SLIDER_MAX );

    _playButton->setIcon( _pauseIcon );

    _playing = true;

    _player->PlayAtTime( newTimePos );

    _startTimeLabel->setText(
      QString::number( _player->currentTime( ) , 'f' , 3 ));

    if ( notify )
    {
#ifdef VISIMPL_USE_ZEROEQ
      try
      {
        // Send event
        if(_player->zeqEvents())
        {
          _player ->zeqEvents( )->sendFrame( _player->startTime(),
                                             _player->endTime(),
                                             _player->currentTime() );
        }
      }
      catch(const std::exception &e)
      {
        std::cerr << "Exception when sending frame. " << e.what() << ". "
                  << __FILE__ << ":" << __LINE__ << std::endl;
      }
      catch(...)
      {
        std::cerr << "Unknown exception when sending frame. "
                  << __FILE__ << ":" << __LINE__ << std::endl;
      }
#endif
#ifdef VISIMPL_USE_GMRVLEX
      sendZeroEQPlaybackOperation(zeroeq::gmrv::PLAY);
#endif
    }
  }
}

void MainWindow::Restart( bool notify )
{
  if ( _player )
  {
    bool playing = _playing;
    _player->Stop( );
    _playing = false;
    if ( playing )
    {
      _player->Play( );
      _playing = true;
    }
    if ( _playing )
      _playButton->setIcon( _pauseIcon );
    else
      _playButton->setIcon( _playIcon );

    if ( notify )
    {
#ifdef VISIMPL_USE_GMRVLEX
      sendZeroEQPlaybackOperation(zeroeq::gmrv::BEGIN);
#endif
    }
  }
}

void MainWindow::GoToEnd( bool notify )
{
  if ( _player )
  {
    //TODO implement GOTOEND

    if ( notify )
    {
#ifdef VISIMPL_USE_GMRVLEX
      sendZeroEQPlaybackOperation(zeroeq::gmrv::END);
#endif
    }
  }
}

void MainWindow::UpdateSimulationSlider( float position )
{
  // NOTE: this method receives the position in time, not percentage.
  const auto tBegin = _player->startTime( );
  const auto tEnd = _player->endTime( );
  const auto newPosition = std::min( tEnd , std::max( tBegin , position ));
  const auto isOverflow = ( std::abs( newPosition - position ) >
                            std::numeric_limits< float >::epsilon( ));

  PlayAtTime( newPosition , isOverflow );

  if ( isOverflow )
    Pause( true );

  if ( _summary )
    _summary->repaintHistograms( );

  if ( _ui->actionFollowPlayhead->isChecked( ))
    _summary->focusPlayback( );
}

#ifdef VISIMPL_USE_ZEROEQ

#ifdef VISIMPL_USE_GMRVLEX

void MainWindow::ApplyPlaybackOperation(unsigned int playbackOp)
{
  const auto operation = static_cast<zeroeq::gmrv::PlaybackOperation>(playbackOp);

  switch (operation)
  {
    case zeroeq::gmrv::PLAY:
      Play(false);
      break;
    case zeroeq::gmrv::PAUSE:
      Pause(false);
      break;
    case zeroeq::gmrv::STOP:
      Stop(false);
      break;
    case zeroeq::gmrv::BEGIN:
      Restart(false);
      break;
    case zeroeq::gmrv::END:
      GoToEnd(false);
      break;
    case zeroeq::gmrv::ENABLE_LOOP:
      _zeqEventRepeat(true);
      break;
    case zeroeq::gmrv::DISABLE_LOOP:
      _zeqEventRepeat(false);
      break;
    default:
      break;
  }
}

void MainWindow::_zeqEventRepeat(bool repeat)
{
  _repeatButton->setChecked(repeat);
}

void MainWindow::HistogramClicked(visimpl::HistogramWidget *histogram)
{
  const visimpl::GIDUSet *selection;

  if (histogram->filteredGIDs().size() == 0)
    selection = &_summary->gids();
  else
    selection = &histogram->filteredGIDs();

  std::vector<uint32_t> selected(selection->begin(), selection->end());

  try
  {
    auto &zInstance = visimpl::ZeroEQConfig::instance();
    if(zInstance.isConnected())
    {
      zInstance.publisher()->publish(lexis::data::SelectedIDs(selected));
    }
  }
  catch(std::exception &e)
  {
    std::cerr << "Exception sending histogram id event. " << e.what() << ". "
              << __FILE__ << ":" << __LINE__ << std::endl;
  }
  catch(...)
  {
    std::cerr << "Unknown exception sending histogram id event."
              << __FILE__ << ":" << __LINE__ << std::endl;
  }
}

#endif

void MainWindow::_onSelectionEvent(lexis::data::ConstSelectedIDsPtr selected)
{
  if (_summary && _ui->actionAddZeroEQhistograms->isChecked())
  {
    std::vector<uint32_t> ids = selected->getIdsVector();

    visimpl::GIDUSet selectedSet(ids.begin(), ids.end());

    visimpl::Selection selection;

    selection.gids = selectedSet;
    selection.name = std::string("Selection ") + std::to_string(_summary->histogramsNumber());

    _summary->AddNewHistogram(selection, true);
  }
}

#endif

void MainWindow::playAtButtonClicked( void )
{
  bool ok;
  const double result = QInputDialog::getDouble( this , tr(
                                                   "Set simulation time to play:" ) ,
                                                 tr( "Simulation time" ) ,
                                                 static_cast<double>(_player->currentTime( )) ,
                                                 static_cast<double>(_player->startTime( )) ,
                                                 static_cast<double>(_player->endTime( )) ,
                                                 3 , &ok , Qt::Popup );

  if ( ok )
  {
    float percentage = ( result - _player->startTime( )) /
                       ( _player->endTime( ) - _player->startTime( ));

    percentage = std::max( 0.0f , std::min( 1.0f , percentage ));

    PlayAtPercentage( percentage );
  }
}

void MainWindow::loadComplete( void )
{
  _summary->showMarker( false );
}

void MainWindow::addCorrelation( const std::string& subset )
{
  _correlations.push_back( subset );
}

void MainWindow::calculateCorrelations( void )
{
  if ( !_subsetEventManager ) return;

  // TODO: mirar @gael.
  auto pdata = dynamic_cast<simil::SpikeData*>(_player->data( ).get( ));
  visimpl::CorrelationComputer cc( pdata );

  const auto eventNames = _subsetEventManager->eventNames( );

  constexpr double deltaTime = 0.125;

  cc.configureEvents( eventNames , deltaTime );

  auto correlateSubsets = [ &eventNames , &cc ]( const std::string& event )
  {
    cc.correlateSubset( event , eventNames , deltaTime , 2600 , 2900 );
  };
  std::for_each( _correlations.cbegin( ) , _correlations.cend( ) ,
                 correlateSubsets );

  const auto names = cc.correlationNames( );

  auto addHistogram = [ this , &cc ]( const std::string& name )
  {
    auto correlation = cc.correlation( name );

    if ( !correlation ) return;

    visimpl::Selection selection;
    selection.name = correlation->fullName;
    selection.gids = cc.getCorrelatedNeurons( correlation->fullName );

    _summary->AddNewHistogram( selection );
  };
  std::for_each( names.cbegin( ) , names.cend( ) , addHistogram );
}

void MainWindow::aboutDialog( void )
{
  QString msj =
    QString( "<h2>ViSimpl - StackViz</h2>" ) +
    tr( "A multi-view visual analyzer of brain simulation data. " ) + "<br>" +
    tr( "Version " ) + stackviz::Version::getString( ).c_str( ) +
    tr( " rev (%1)<br>" ).arg( stackviz::Version::getRevision( )) +
    "<a href='https://vg-lab.es/visimpl/'>https://vg-lab.es/visimpl</a>" +
    "<h4>" + tr( "Build info:" ) + "</h4>" +
    "<ul><li>Qt " + QT_VERSION_STR +

    #ifdef VISIMPL_USE_GMRVLEX
    "</li><li>GmrvLex " + GMRVLEX_REV_STRING +
    #else
    "</li><li>GmrvLex " + tr( "support not built." ) +
    #endif

    #ifdef VISIMPL_USE_PREFR
    "</li><li>prefr " + PREFR_REV_STRING +
    #else
    "</li><li>prefr " + tr( "support not built." ) +
    #endif

    #ifdef VISIMPL_USE_RETO
    "</li><li>ReTo " + RETO_REV_STRING +
    #else
    "</li><li>ReTo " + tr ("support not built.") +
    #endif

    #ifdef VISIMPL_USE_SCOOP
    "</li><li>Scoop " + SCOOP_REV_STRING +
    #else
    "</li><li>Scoop " + tr ("support not built.") +
    #endif

    #ifdef VISIMPL_USE_SIMIL
    "</li><li>SimIL " + SIMIL_REV_STRING +
    #else
    "</li><li>SimIL " + tr ("support not built.") +
    #endif

    #ifdef VISIMPL_USE_ZEROEQ
    "</li><li>ZeroEQ " + zeroeq::Version::getRevString().c_str() +
    #else
    "</li><li>ZeroEQ " + tr( "support not built." ) +
    #endif

    "</li><li>AcuteRecorder " + ACUTERECORDER_REV_STRING +

    "</li></ul>" + "<h4>" + tr( "Developed by:" ) + "</h4>" +
    "VG-Lab / URJC / UPM"
    "<br><a href='https://vg-lab.es'>https://vg-lab.es</a>"
    "<br>(C) 2015-" +
    QString::number( QDateTime::currentDateTime( ).date( ).year( )) + "<br><br>"
                                                                      "<a href='https://vg-lab.es'><img src=':/icons/logoVGLab.png'/></a>"
                                                                      "&nbsp;&nbsp;&nbsp;&nbsp;"
                                                                      "<a href='https://www.urjc.es'><img src=':/icons/logoURJC.png' /></a>"
                                                                      "&nbsp;&nbsp;&nbsp;&nbsp;"
                                                                      "<a href='https://www.upm.es'><img src=':/icons/logoUPM.png' /></a>";

  QMessageBox::about( this , tr( "About StackViz" ) , msj );
}

void MainWindow::openH5FilesThroughDialog( void )
{
  const auto networkFilename = QFileDialog::getOpenFileName( this , tr(
                                                               "Open a HDF5 network file" ) ,
                                                             _lastOpenedFileNamePath ,
                                                             tr(
                                                               "hdf5 ( *.h5 *.hdf5);; All files (*)" ) ,
                                                             nullptr ,
                                                             QFileDialog::DontUseNativeDialog );

  if ( networkFilename.isEmpty( )) return;

  const auto activityFilename = QFileDialog::getOpenFileName( this , tr(
                                                                "Open a HDF5 activity file" ) ,
                                                              _lastOpenedFileNamePath ,
                                                              tr(
                                                                "hdf5 ( *.h5 *.hdf5);; All files (*)" ) ,
                                                              nullptr ,
                                                              QFileDialog::DontUseNativeDialog );

  if ( activityFilename.isEmpty( )) return;

  loadData( simil::THDF5 , networkFilename.toStdString( ) ,
            activityFilename.toStdString( ) , simil::TSimSpikes );
}

void MainWindow::updateUIonOpen( const std::string& eventsFile )
{
  configurePlayer( );

  initSummaryWidget( );

  openSubsetEventFile( eventsFile , true );

  _summary->generateEventsRep( );
  _summary->importSubsetsFromSubsetMngr( );

  if ( _displayManager )
    _displayManager->refresh( );

  _ui->actionShowDataManager->setEnabled( true );
  _ui->actionOpenSubsetEventsFile->setEnabled( true );
  _ui->actionOpenGroupsFile->setEnabled( true );
  _ui->actionCloseData->setEnabled( true );

  _dockSimulation->setEnabled( true );
}

void stackviz::MainWindow::loadData( const simil::TDataType type ,
                                     const std::string& arg1 ,
                                     const std::string& arg2 ,
                                     const simil::TSimulationType simType ,
                                     const std::string& subsetEventFile )
{
  updateGeometry( );

  Q_ASSERT( type != simil::TDataType::TREST );
  m_dataInspector->setUpdatesEnabled( false );
  _ui->actionConfigureRESTconnection->setEnabled( false );
  _ui->actionConnectRESTserver->setEnabled( false );

  _simulationType = simType;
  m_subsetEventFile = subsetEventFile;

  QApplication::setOverrideCursor( Qt::WaitCursor );

  closeLoadingDialog( );

  m_loaderDialog = new LoadingDialog( this );
  m_loaderDialog->show( );

  m_loader = std::make_shared< LoaderThread >( );
  m_loader->setData( type , arg1 , arg2 );

  connect( m_loader.get( ) , SIGNAL( finished( )) , this ,
           SLOT( onLoadFinished( )));
  connect( m_loader.get( ) , SIGNAL( progress( int )) , m_loaderDialog ,
           SLOT( setProgress( int )));
  connect( m_loader.get( ) , SIGNAL( network( unsigned int )) , m_loaderDialog ,
           SLOT( setNetwork( unsigned int )));
  connect( m_loader.get( ) , SIGNAL( spikes( unsigned int )) , m_loaderDialog ,
           SLOT( setSpikesValue( unsigned int )));

  m_loader->start( );
}

void MainWindow::openRecorder( void )
{
  auto action = qobject_cast< QAction* >( sender( ));
  // The button stops the recorder if found.
  if ( _recorder != nullptr )
  {
    if ( action ) action->setDisabled( true );

    RecorderUtils::stopAndWait( _recorder , this );
    _recorder = nullptr;
    return;
  }

  RSWParameters params;
  params.widgetsToRecord.emplace_back( "Main Widget" , this );
  params.defaultFPS = 30;
  params.includeScreens = false;
  params.stabilizeFramerate = true;

  if ( !_ui->actionAdvancedRecorderOptions->isChecked( ))
  {
    params.showWorker = false;
    params.showWidgetSourceMode = false;
    params.showSourceParameters = false;
  }

  RecorderDialog dialog( nullptr , params , true );
  dialog.setWindowIcon( QIcon( ":/visimpl.png" ));
  dialog.setFixedSize( 800 , 600 );
  if ( dialog.exec( ) == QDialog::Accepted )
  {
    _recorder = dialog.getRecorder( );
    connect( _recorder , SIGNAL( finished( )) ,
             _recorder , SLOT( deleteLater( )));
    connect( _recorder , SIGNAL( finished( )) ,
             this , SLOT( finishRecording( )));
    if ( action ) action->setChecked( true );
  }
  else
  {
    if ( action ) action->setChecked( false );
  }
}

void MainWindow::finishRecording( )
{
  auto action = _ui->menuTools->actions( ).first( );
  action->setEnabled( true );
  action->setChecked( false );
}

void stackviz::MainWindow::onLoadFinished( )
{
  if ( m_loader )
  {
    const auto errors = m_loader->errors( );
    if ( !errors.empty( ))
    {
      closeLoadingDialog( );
      _player->Clear( );
      _subsetEventManager = nullptr;
      QApplication::restoreOverrideCursor( );

      const auto message = QString::fromStdString( errors );
      QMessageBox::critical( this , tr( "Error loading data" ) , message ,
                             QMessageBox::Ok );

      m_loader = nullptr;
      return;
    }
  }

  const auto dataType = m_loader->type( );

  switch ( dataType )
  {
    case simil::TBlueConfig:
    case simil::TCSV:
    case simil::THDF5:
    {
      const auto spikeData = m_loader->simulationData( );

      _player = new simil::SpikesPlayer( );
      _player->LoadData( spikeData );

      m_loader = nullptr;
    }
      break;
    case simil::TREST:
    {
#ifdef SIMIL_WITH_REST_API
      const auto netData = m_loader->network( );
      const auto simData = m_loader->simulationData( );

      _player = new simil::SpikesPlayer( );
      _player->LoadData( netData , simData );

      m_loader = nullptr;

#endif
    }
      break;
    case simil::TDataUndefined:
    default:
    {
      m_loader = nullptr;
      closeLoadingDialog( );
      QMessageBox::critical( this , tr( "Error loading data" ) ,
                             tr( "Data type is undefined after loading" ) ,
                             QMessageBox::Ok );

      return;
    }
      break;
  }

  if ( m_loaderDialog )
  {
    const auto gids = _player->gidsSize( );
    const auto spikes = reinterpret_cast<simil::SpikesPlayer*>(_player)->spikesSize( );
    m_loaderDialog->setNetwork( gids );
    m_loaderDialog->setSpikesValue( spikes );
  }

  _subsetEventManager = _player->data( )->subsetsEvents( );

  updateUIonOpen( m_subsetEventFile );

  closeLoadingDialog( );

  QApplication::restoreOverrideCursor( );
}

void stackviz::MainWindow::sendZeroEQPlaybackOperation( const unsigned int op )
{
#ifdef SIMIL_USE_ZEROEQ
  try
  {
    if(_player ->zeqEvents( ))
    {
      _player ->zeqEvents( )->sendPlaybackOp( static_cast<zeroeq::gmrv::PlaybackOperation>(op) );
    }
  }
  catch(const std::exception &e)
  {
    std::cerr << "Exception when sending play operation. " << e.what() << ". "
              << __FILE__ << ":" << __LINE__ << std::endl;
  }
  catch(...)
  {
    std::cerr << "Unknown exception when sending play operation. "
              << __FILE__ << ":" << __LINE__  << std::endl;
  }
#else
  ignore( op ); // c++17 [[maybe_unused]]
#endif
}

void stackviz::MainWindow::onDataUpdated( )
{
  const float tBegin = _player->startTime( );
  const float tEnd = _player->endTime( );
  const float tCurrent = _player->currentTime( );

  if ( tEnd > tBegin )
  {
    _startTimeLabel->setText( QString::number( tCurrent , 'f' , 3 ));
    _endTimeLabel->setText( QString::number( tEnd , 'f' , 3 ));

    const float percentage =
      static_cast<float>(tCurrent - tBegin) / ( tEnd - tBegin );
    _simSlider->setValue( percentage * SLIDER_MAX );
  }
}

void stackviz::MainWindow::closeEvent( QCloseEvent* e )
{
  if ( _recorder )
  {
    QMessageBox msgBox( this );
    msgBox.setWindowTitle( tr( "Exit StackViz" ));
    msgBox.setWindowIcon( QIcon( ":/visimpl.png" ));
    msgBox.setText(
      tr( "A recording is being made. Do you really want to exit StackViz?" ));
    msgBox.setStandardButtons( QMessageBox::Cancel | QMessageBox::Yes );

    if ( msgBox.exec( ) != QMessageBox::Yes )
    {
      e->ignore( );
      return;
    }

    RecorderUtils::stopAndWait( _recorder , this );
    _recorder = nullptr;
  }

  QMainWindow::closeEvent( e );
}

void stackviz::MainWindow::openRESTThroughDialog( )
{
#ifdef SIMIL_WITH_REST_API

  if ( _alreadyConnected && _player != nullptr )
  {
    ReconnectRESTDialog dialog( this );
    if ( dialog.exec( ) == QDialog::Rejected )
      return;

    if ( dialog.getSelection( ) !=
         ReconnectRESTDialog::Selection::NEW_CONNECTION )
    {
      _restConnectionInformation.network =
        dialog.getSelection( ) ==
        ReconnectRESTDialog::Selection::SPIKES_AND_NETWORK
        ? std::weak_ptr< simil::Network >( )
        : _player->getNetwork( );

      loadRESTData( _restConnectionInformation );
      return;
    }
  }

  ConnectRESTDialog dialog( this );

  ConnectRESTDialog::Connection connection;
  dialog.setRESTConnection( connection );

  if ( QDialog::Accepted == dialog.exec( ))
  {
    connection = dialog.getRESTConnection( );
    const auto restOpt = dialog.getRESTOptions( );

    simil::LoaderRestData::Configuration config;
    config.api =
      connection.protocol.compare( "NEST" , Qt::CaseInsensitive ) == 0 ?
      simil::LoaderRestData::Rest_API::NEST :
      simil::LoaderRestData::Rest_API::ARBOR;
    config.port = connection.port;
    config.url = connection.url.toStdString( );
    config.waitTime = restOpt.waitTime;
    config.failTime = restOpt.failTime;
    config.spikesSize = restOpt.spikesSize;

    _restConnectionInformation = config;
    _alreadyConnected = true;

    loadRESTData( config );
  }
#else
  const auto title = tr("Connect REST API");
  const auto message = tr("REST data loading is unsupported.");
  QMessageBox::critical(this, title, message, QMessageBox::Ok);
#endif
}

void stackviz::MainWindow::configureREST( )
{
#ifdef SIMIL_WITH_REST_API
  if ( !m_loader )
  {
    const QString message = tr( "There is no REST connection!" );
    QMessageBox::warning( this , tr( "REST API Options" ) , message ,
                          QMessageBox::Ok );
    return;
  }

  auto loader = m_loader->RESTLoader( );
  auto options = loader->getConfiguration( );

  RESTConfigurationWidget::Options dialogOptions;
  dialogOptions.waitTime = options.waitTime;
  dialogOptions.failTime = options.failTime;
  dialogOptions.spikesSize = options.spikesSize;

  ConfigureRESTDialog dialog( this , Qt::WindowFlags( ) , dialogOptions );
  if ( QDialog::Accepted == dialog.exec( ))
  {
    dialogOptions = dialog.getRESTOptions( );

    simil::LoaderRestData::Configuration config;
    config.waitTime = dialogOptions.waitTime;
    config.failTime = dialogOptions.failTime;
    config.spikesSize = dialogOptions.spikesSize;

    loader->setConfiguration( config );
  }
#else
  const auto title = tr("Configure REST API");
  const auto message = tr("REST data loading is unsupported.");
  QMessageBox::critical(this, title, message, QMessageBox::Ok);
#endif
}

void stackviz::MainWindow::closeData( )
{
#ifdef SIMIL_WITH_REST_API
  _alreadyConnected = false;
  if ( m_loader && m_loader->type( ) == simil::TREST )
  {
    CloseDataDialog dialog( this );
    const auto result = dialog.exec( );

    if ( result == QDialog::Rejected ) return;

    if ( dialog.keepNetwork( ))
    {
      QApplication::setOverrideCursor( Qt::WaitCursor );

      Stop( );

      m_dataInspector->setCheckUpdates( false );

      _summary->UpdateHistograms( );

      m_dataInspector->update( );
      m_dataInspector->setCheckUpdates( true );

      _simSlider->setSliderPosition( 0 );

      repaint( );

      QApplication::processEvents( );

      QApplication::restoreOverrideCursor( );

      return;
    }
  }
#endif

  QApplication::setOverrideCursor( Qt::WaitCursor );
  _player->Clear( );
  _subsetEventManager = nullptr;

  // remove histograms
  if ( _summary )
  {
    layout( )->removeWidget( _summary );
    _summary->deleteLater( );
    _summary = nullptr;
    setCentralWidget( new QWidget( ));
  }

  if ( _displayManager )
  {
    if ( _displayManager->isVisible( )) _displayManager->hide( );
    _displayManager->deleteLater( );
    _displayManager = nullptr;
  }

  _correlations.clear( );

  _ui->actionCloseData->setEnabled( false );
  _dockSimulation->setEnabled( false );

  m_dataInspector->setUpdatesEnabled( false );
  m_dataInspector->setSimPlayer( nullptr );

  QApplication::restoreOverrideCursor( );
}

void stackviz::MainWindow::closeLoadingDialog( )
{
  if ( m_loaderDialog )
  {
    m_loaderDialog->close( );
    delete m_loaderDialog;
    m_loaderDialog = nullptr;
  }
}

#ifdef SIMIL_WITH_REST_API

void stackviz::MainWindow::loadRESTData(
  const simil::LoaderRestData::Configuration& config )
{
  closeLoadingDialog( );

  QApplication::setOverrideCursor( Qt::WaitCursor );

  m_dataInspector->setCheckUpdates( false );

  m_loaderDialog = new LoadingDialog( this );
  m_loaderDialog->show( );

  QApplication::processEvents( );

  m_loader = std::make_shared< LoaderThread >( );
  m_loader->setRESTConfiguration( config );

  connect( m_loader.get( ) , SIGNAL( finished( )) ,
           this , SLOT( onLoadFinished( )));
  connect( m_loader.get( ) , SIGNAL( progress( int )) ,
           m_loaderDialog , SLOT( setProgress( int )));
  connect( m_loader.get( ) , SIGNAL( network( unsigned int )) ,
           m_loaderDialog , SLOT( setNetwork( unsigned int )));
  connect( m_loader.get( ) , SIGNAL( spikes( unsigned int )) ,
           m_loaderDialog , SLOT( setSpikesValue( unsigned int )));

  m_loader->start( );
}

#endif

void MainWindow::openGroupsThroughDialog( )
{
  const auto title = tr( "Load Groups" );

  const QString groupsFilename = QFileDialog::getOpenFileName( this , title ,
                                                               _lastOpenedGroupsFileName ,
                                                               tr(
                                                                 "Json files (*.json)" ) ,
                                                               nullptr ,
                                                               QFileDialog::ReadOnly |
                                                               QFileDialog::DontUseNativeDialog );

  if ( groupsFilename.isEmpty( ))
    return;

  QFile file{ groupsFilename };
  if ( !file.open( QIODevice::ReadOnly ))
  {
    const auto message = tr( "Couldn't open file %1" ).arg( groupsFilename );

    QMessageBox msgbox( this );
    msgbox.setWindowTitle( title );
    msgbox.setIcon( QMessageBox::Icon::Critical );
    msgbox.setText( message );
    msgbox.setWindowIcon( QIcon( ":/visimpl.png" ));
    msgbox.setStandardButtons( QMessageBox::Ok );
    msgbox.exec( );
    return;
  }

  const auto contents = file.readAll( );
  QJsonParseError jsonError;
  const auto jsonDoc = QJsonDocument::fromJson( contents , &jsonError );
  if ( jsonDoc.isNull( ) || !jsonDoc.isObject( ))
  {
    const auto message = tr(
      "Couldn't read the contents of %1 or parsing error." ).arg(
      groupsFilename );

    QMessageBox msgbox{ this };
    msgbox.setWindowTitle( title );
    msgbox.setIcon( QMessageBox::Icon::Critical );
    msgbox.setText( message );
    msgbox.setWindowIcon( QIcon( ":/visimpl.png" ));
    msgbox.setStandardButtons( QMessageBox::Ok );
    msgbox.setDetailedText( jsonError.errorString( ));
    msgbox.exec( );
    return;
  }

  const auto jsonObj = jsonDoc.object( );
  if ( jsonObj.isEmpty( ))
  {
    const auto message = tr( "Error parsing the contents of %1." ).arg(
      groupsFilename );

    QMessageBox msgbox{ this };
    msgbox.setWindowTitle( title );
    msgbox.setIcon( QMessageBox::Icon::Critical );
    msgbox.setText( message );
    msgbox.setWindowIcon( QIcon( ":/visimpl.png" ));
    msgbox.setStandardButtons( QMessageBox::Ok );
    msgbox.exec( );
    return;
  }

  QApplication::setOverrideCursor( Qt::WaitCursor );

  const auto jsonGroups = jsonObj.value( "groups" ).toArray( );
  for ( const auto group: jsonGroups )
  {
    const auto o = group.toObject( );

    const auto name = o.value( "name" ).toString( );
    const auto gidsStrings = o.value( "gids" ).toString( ).split( "," );

    visimpl::GIDUSet gids;
    auto addGids = [ &gids ]( const QString s )
    {
      if ( s.contains( ":" ))
      {
        auto limits = s.split( ":" );
        for ( unsigned int id = limits.first( ).toUInt( );
              id <= limits.last( ).toUInt( ); ++id )
          gids.insert( id );
      }
      else
      {
        gids.insert( s.toUInt( ));
      }
    };
    std::for_each( gidsStrings.cbegin( ) , gidsStrings.cend( ) , addGids );

    visimpl::Selection selection;
    selection.gids = gids;
    selection.name = name.toStdString( );

#ifdef VISIMPL_USE_ZEROEQ
    _summary->AddNewHistogram(selection, false);
#else
    _summary->AddNewHistogram( selection );
#endif
  }

  _summary->UpdateHistograms( );

  if ( _displayManager )
  {
    _displayManager->dirtyHistograms( );
    _displayManager->refresh( );
  }

  QApplication::restoreOverrideCursor( );
}
