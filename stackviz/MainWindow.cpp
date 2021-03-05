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

#include <boost/bind.hpp>

#include <thread>

#include <sumrice/sumrice.h>

using namespace stackviz;

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
, _playIcon(":/icons/play.svg")
, _pauseIcon(":/icons/pause.svg")
, _startTimeLabel( nullptr )
, _endTimeLabel( nullptr )
, _displayManager( nullptr )
#ifdef VISIMPL_USE_ZEROEQ
, _zeqConnection( false )
, _subscriber( nullptr )
, _publisher( nullptr )
, _thread( nullptr )
#endif
{
  _ui->setupUi( this );

  #ifdef VISIMPL_USE_SIMIL
    _ui->actionOpenBlueConfig->setEnabled( true );
  #else
    _ui->actionOpenBlueConfig->setEnabled( false );
  #endif

  connect( _ui->actionQuit, SIGNAL( triggered( void )),
           QApplication::instance(), SLOT( quit( void )));

  // Connect about dialog
  connect( _ui->actionAbout, SIGNAL( triggered( void )),
           this, SLOT( aboutDialog( void )));
}

void MainWindow::init( const std::string&
  #ifdef VISIMPL_USE_ZEROEQ
                         zeqUri
  #endif
    )
{

  connect( _ui->actionOpenBlueConfig, SIGNAL( triggered( void )),
           this, SLOT( openBlueConfigThroughDialog( void )));

  connect( _ui->actionOpenCSVFiles, SIGNAL( triggered( void )),
           this, SLOT( openCSVFilesThroughDialog( void )));

  connect( _ui->actionOpenH5Files, SIGNAL( triggered( void )),
           this, SLOT( openH5FilesThroughDialog( void )));

  connect( _ui->actionOpenSubsetEventsFile, SIGNAL( triggered( void )),
           this, SLOT( openSubsetEventsFileThroughDialog( void )));

  initPlaybackDock( );

  connect( _dockSimulation->toggleViewAction( ), SIGNAL( toggled( bool )),
             _ui->actionTogglePlaybackDock, SLOT( setChecked( bool )));

  connect( _ui->actionTogglePlaybackDock, SIGNAL( triggered( void )),
           this, SLOT( togglePlaybackDock( void )));

  connect( _ui->actionShowDataManager, SIGNAL( triggered( void )),
           this, SLOT( showDisplayManagerWidget( void )));

#ifdef VISIMPL_USE_ZEROEQ

  _setZeqUri( zeqUri );
  _ui->actionTogglePlaybackDock->setChecked( true );

#endif
  _ui->toolBar->setContextMenuPolicy(Qt::PreventContextMenu);
  _ui->menubar->setContextMenuPolicy(Qt::PreventContextMenu);

  _ui->actionShowDataManager->setEnabled(false);
}

MainWindow::~MainWindow( void )
{
  delete _ui;

#ifdef VISIMPL_USE_ZEROEQ

  if( _zeqConnection )
  {
    _zeqConnection = false;
    _thread->join();

    delete _thread;

    if(_subscriber)
    {
      _subscriber->unsubscribe(lexis::data::SelectedIDs::ZEROBUF_TYPE_IDENTIFIER( ));
      delete _subscriber;
      _subscriber = nullptr;
    }

    if(_publisher)
    {
      delete _publisher;
      _publisher = nullptr;
    }
  }
#endif
}


void MainWindow::showStatusBarMessage ( const QString& message )
{
  _ui->statusbar->showMessage( message );
}

void MainWindow::openBlueConfig( const std::string& fileName,
                                 simil::TSimulationType simulationType,
                                 const std::string& target,
                                 const std::string& subsetEventFile )
{
  ( void ) target;

  _simulationType = simulationType;

  switch( _simulationType )
  {
    case simil::TSimSpikes:
    {
      auto spikeData = new simil::SpikeData( fileName, simil::TBlueConfig, target );
      spikeData->reduceDataToGIDS( );

      auto player = new simil::SpikesPlayer( );
      player->LoadData( spikeData );
      _player = player;

      _subsetEventManager = _player->data( )->subsetsEvents( );
      break;
   }
   case simil::TSimVoltages:
  #ifdef SIMIL_USE_BRION
//       _player = new simil::VoltagesPlayer( fileName, reportLabel, true);
  #else
       std::cerr << "SimIL without Brion support." << std::endl;
       exit( -1 );
  #endif
  //     _deltaTime = _player->deltaTime( );
       break;

   default:
     VISIMPL_THROW("Cannot load an undefined simulation type.");

  }

  updateUIonOpen(subsetEventFile);
}

void MainWindow::openHDF5File( const std::string& networkFile,
                               simil::TSimulationType simulationType,
                               const std::string& activityFile,
                               const std::string& subsetEventFile )
{
  _simulationType = simulationType;

  simil::SpikesPlayer *player = new simil::SpikesPlayer();
  player->LoadData(simil::TDataType::THDF5, networkFile, activityFile);
  _player = player;

  _subsetEventManager = _player->data()->subsetsEvents();

  updateUIonOpen(subsetEventFile);
}

void MainWindow::openCSVFile( const std::string& networkFile,
                              simil::TSimulationType simulationType,
                              const std::string& activityFile,
                              const std::string& subsetEventFile )
{
  _simulationType = simulationType;

  simil::SpikesPlayer *player = new simil::SpikesPlayer();
  _player = player;

  player->LoadData(simil::TDataType::TCSV, networkFile, activityFile);

  _subsetEventManager = _player->data()->subsetsEvents();

  updateUIonOpen(subsetEventFile);
}

void MainWindow::openSubsetEventFile(const std::string &filePath, bool append)
{
  if (filePath.empty() || !_subsetEventManager) return;

  if (!append) _subsetEventManager->clear();

  _summary->clearEvents();

  if (filePath.find("json") != std::string::npos)
  {
    _subsetEventManager->loadJSON(filePath);
  }
  else
    if (filePath.find("h5") != std::string::npos)
    {
      _subsetEventManager->loadH5(filePath);
      _autoCalculateCorrelations = true;
    }
    else
    {
      std::cout << "Subset Events file not found: " << filePath << std::endl;
    }
}

void MainWindow::openBlueConfigThroughDialog( void )
{
#ifdef VISIMPL_USE_SIMIL

   const QString filename = QFileDialog::getOpenFileName(
                      this, tr( "Open BlueConfig" ), _lastOpenedFileNamePath,
                      tr( "BlueConfig ( BlueConfig CircuitConfig);; All files (*)" ),
                      nullptr, QFileDialog::DontUseNativeDialog );

   if( !filename.isEmpty( ))
   {
     bool ok1, ok2;
     QInputDialog simTypeDialog;
     simil::TSimulationType simType;
     const QStringList items = {"Spikes"};//, "Voltages"};

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
       _lastOpenedFileNamePath = QFileInfo(filename).path( );
       QApplication::setOverrideCursor(Qt::WaitCursor);
       openBlueConfig( filename.toStdString(), simType, text.toStdString() );
       QApplication::restoreOverrideCursor();
     }
   }
#endif
}

void MainWindow::openCSVFilesThroughDialog( void )
{
    const QString networkFilename = QFileDialog::getOpenFileName(
          this, tr( "Open CSV Network description file" ), _lastOpenedFileNamePath,
          tr( "CSV (*.csv);; All files (*)" ),
          nullptr, QFileDialog::DontUseNativeDialog );

  if( !networkFilename.isEmpty( ))
  {
    simil::TSimulationType simType = simil::TSimSpikes;

    const QString activityFilename = QFileDialog::getOpenFileName(
            this, tr( "Open CSV Activity file" ), _lastOpenedFileNamePath,
            tr( "CSV (*.csv);; All files (*)" ),
            nullptr, QFileDialog::DontUseNativeDialog );

    if ( !activityFilename.isEmpty( ))
    {
      _lastOpenedFileNamePath = QFileInfo( networkFilename ).path( );
      QApplication::setOverrideCursor(Qt::WaitCursor);
      openCSVFile( networkFilename.toStdString( ), simType, activityFilename.toStdString( ) );
      QApplication::restoreOverrideCursor();
    }
  }
}

void MainWindow::openSubsetEventsFileThroughDialog( void )
{
    const QString eventsFilename = QFileDialog::getOpenFileName(this,
              tr( "Open file containing subsets/events data" ),
              _lastOpenedSubsetsFileName,
              tr( "JSON (*.json);; hdf5 (*.h5);; All files (*)" ),
              nullptr, QFileDialog::DontUseNativeDialog );

  if( !eventsFilename.isEmpty( ))
  {
    _lastOpenedSubsetsFileName = QFileInfo( eventsFilename ).path( );

    QApplication::setOverrideCursor(Qt::WaitCursor);
    openSubsetEventFile( eventsFilename.toStdString( ), false );
    QApplication::restoreOverrideCursor();

    _summary->generateEventsRep( );
    _summary->importSubsetsFromSubsetMngr( );

    if( _displayManager )
      _displayManager->refresh( );
  }
}

void MainWindow::configurePlayer( void )
{
  _startTimeLabel->setText(
        QString::number( static_cast<double>(_player->startTime( ))));

  _endTimeLabel->setText(
          QString::number( static_cast<double>(_player->endTime( ))));

  #ifdef SIMIL_USE_ZEROEQ
  _player->connectZeq( _zeqUri );

  _player->zeqEvents( )->frameReceived.connect(
        boost::bind( &MainWindow::UpdateSimulationSlider, this, _1 ));

  _player->zeqEvents( )->playbackOpReceived.connect(
        boost::bind( &MainWindow::ApplyPlaybackOperation, this, _1 ));
  #endif
}

void MainWindow::togglePlaybackDock(void)
{
  if (_ui->actionTogglePlaybackDock->isChecked())
    _dockSimulation->show();
  else
    _dockSimulation->close();

  update();
}

void MainWindow::showDisplayManagerWidget( void )
{
  if(!_summary) return;

  if( !_displayManager)
  {
    _displayManager = new DisplayManagerWidget( );
    _displayManager->init( _summary->eventWidgets(),
                           _summary->histogramWidgets( ));

    connect( _displayManager, SIGNAL( eventVisibilityChanged( unsigned int, bool )),
             _summary, SLOT( eventVisibility( unsigned int, bool )));

    connect( _displayManager, SIGNAL( subsetVisibilityChanged( unsigned int, bool )),
             _summary, SLOT( subsetVisibility( unsigned int, bool )));

    connect( _displayManager, SIGNAL( removeEvent( unsigned int )),
             _summary, SLOT( removeEvent( unsigned int )));

    connect( _displayManager, SIGNAL( removeHistogram( unsigned int )),
             _summary, SLOT( removeSubset( unsigned int )));

  }

  _displayManager->refresh( );

  _displayManager->show( );
}

void MainWindow::initPlaybackDock( )
{
  _dockSimulation = new QDockWidget( );
  _dockSimulation->setMinimumHeight( 100 );
  _dockSimulation->setSizePolicy( QSizePolicy::MinimumExpanding,
                                  QSizePolicy::MinimumExpanding );

  _playing = false;
  constexpr unsigned int totalHSpan = 20;

  auto content = new QWidget( );
  auto dockLayout = new QGridLayout( );
  content->setLayout( dockLayout );

  _simSlider = new CustomSlider( Qt::Horizontal );
  _simSlider->setMinimum( 0 );
  _simSlider->setMaximum( 1000 );
  _simSlider->setSizePolicy( QSizePolicy::Preferred,
                             QSizePolicy::Preferred );

  _playButton = new QPushButton(_playIcon, tr("") );
  _playButton->setSizePolicy( QSizePolicy::MinimumExpanding,
                              QSizePolicy::MinimumExpanding );
  auto stopButton = new QPushButton(QIcon(":/icons/stop.svg"), tr(""));
  auto nextButton = new QPushButton(QIcon(":/icons/next.svg"), tr(""));
  auto prevButton = new QPushButton(QIcon(":/icons/previous.svg"), tr(""));

  _repeatButton = new QPushButton(QIcon(":/icons/repeat.svg"), tr(""));
  _repeatButton->setCheckable(true);
  _repeatButton->setChecked(false);

  _goToButton = new QPushButton();
  _goToButton->setText(QString("Play at..."));

  _startTimeLabel = new QLabel( "" );
  _startTimeLabel->setSizePolicy( QSizePolicy::MinimumExpanding,
                                  QSizePolicy::Preferred );
  _endTimeLabel = new QLabel( "" );
  _endTimeLabel->setSizePolicy( QSizePolicy::Preferred,
                                QSizePolicy::Preferred );

  unsigned int row = 2;
  dockLayout->addWidget(_startTimeLabel, row, 0, 1, 2);
  dockLayout->addWidget(_simSlider, row, 1, 1, totalHSpan - 3);
  dockLayout->addWidget(_endTimeLabel, row, totalHSpan - 2, 1, 1, Qt::AlignRight);

  row++;
  dockLayout->addWidget(_repeatButton, row, 7, 1, 1);
  dockLayout->addWidget(prevButton, row, 8, 1, 1);
  dockLayout->addWidget(_playButton, row, 9, 2, 2);
  dockLayout->addWidget(stopButton, row, 11, 1, 1);
  dockLayout->addWidget(nextButton, row, 12, 1, 1);
  dockLayout->addWidget(_goToButton, row, 13, 1, 1);

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

  connect( _goToButton, SIGNAL( clicked( )),
           this, SLOT( playAtButtonClicked( )));

  _dockSimulation->setWidget( content );
  this->addDockWidget( Qt::BottomDockWidgetArea,
                         _dockSimulation );
}

void MainWindow::initSummaryWidget( )
{
  _summary = new visimpl::Summary( this, visimpl::T_STACK_EXPANDABLE );

  if( _simulationType == simil::TSimSpikes )
  {
    auto spikesPlayer = dynamic_cast< simil::SpikesPlayer* >( _player );

    _summary->Init( spikesPlayer->data( ));
    _summary->simulationPlayer( _player );
  }

  this->setCentralWidget( _summary );

  connect( _ui->actionAutoNamingSelections, SIGNAL( triggered( )),
           _summary, SLOT( toggleAutoNameSelections( )));

  _ui->actionFill_Plots->setChecked( true );
  connect( _ui->actionFill_Plots, SIGNAL( triggered( bool )),
           _summary, SLOT( fillPlots( bool )));

  connect( _summary, SIGNAL( histogramClicked( float )),
           this, SLOT( PlayAt( float )));

#ifdef VISIMPL_USE_ZEROEQ
  connect( _summary, SIGNAL( histogramClicked( visimpl::HistogramWidget* )),
             this, SLOT( HistogramClicked( visimpl::HistogramWidget* )));
#endif

  _ui->actionFocusOnPlayhead->setVisible( true );
  connect( _ui->actionFocusOnPlayhead, SIGNAL( triggered( )),
           _summary, SLOT( focusPlayback( )));

  if( _autoCalculateCorrelations )
  {
    calculateCorrelations( );
  }

  QTimer::singleShot( 0, _summary, SLOT( adjustSplittersSize( )));
}

void MainWindow::PlayPause(bool notify)
{
  if (_playing)
    Pause(notify);
  else
    Play(notify);
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
    const bool repeat = _repeatButton->isChecked( );
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

void MainWindow::PlayAt(bool notify)
{
  if (_player)
  {
    PlayAt(_simSlider->sliderPosition(), notify);
  }
}

void MainWindow::PlayAt(float percentage, bool notify)
{
  if (_player)
  {
    const int sliderPos = percentage * (_simSlider->maximum() - _simSlider->minimum()) + _simSlider->minimum();

    PlayAt(sliderPos, notify);
  }
}

void MainWindow::PlayAt( int sliderPosition, bool notify )
{
  if( _player )
  {
    const int value = _simSlider->value( );
    const float percentage = static_cast<float>( value - _simSlider->minimum( )) /
                             ( _simSlider->maximum( ) - _simSlider->minimum( ));
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

void MainWindow::UpdateSimulationSlider(float percentage)
{
  const double currentTime = percentage * (_player->endTime() - _player->startTime()) + _player->startTime();

  _startTimeLabel->setText(QString::number(currentTime));

  const int total = _simSlider->maximum() - _simSlider->minimum();

  const int position = percentage * total;

  _simSlider->setSliderPosition(position);

  if (_summary)
    _summary->repaintHistograms();

  if (_ui->actionFollowPlayhead->isChecked())
    _summary->focusPlayback();
}

#ifdef VISIMPL_USE_ZEROEQ

#ifdef VISIMPL_USE_GMRVLEX

void MainWindow::ApplyPlaybackOperation(unsigned int playbackOp)
{
  auto operation = static_cast<zeroeq::gmrv::PlaybackOperation>(playbackOp);

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

  _publisher->publish(lexis::data::SelectedIDs(selected));
}

#endif

void MainWindow::_setZeqUri(const std::string &uri_)
{
  _zeqUri = uri_.empty() ? zeroeq::DEFAULT_SESSION : uri_;

  _zeqConnection = true;
  _subscriber = new zeroeq::Subscriber(_zeqUri);
  _publisher = new zeroeq::Publisher(_zeqUri);

  _subscriber->subscribe(lexis::data::SelectedIDs::ZEROBUF_TYPE_IDENTIFIER(),
                         [&](const void *data_, const size_t size_)
                         { _onSelectionEvent( lexis::data::SelectedIDs::create( data_, size_ ));});

  _thread = new std::thread([&]()
  { while( _zeqConnection ) _subscriber->receive( 10000 );});
}

void MainWindow::_onSelectionEvent(lexis::data::ConstSelectedIDsPtr selected)
{
  std::vector<uint32_t> ids = selected->getIdsVector();

  visimpl::GIDUSet selectedSet(ids.begin(), ids.end());

  if (_summary)
  {
    visimpl::Selection selection;

    selection.gids = selectedSet;

    _summary->AddNewHistogram(selection, true);
  }
}

#endif

void MainWindow::playAtButtonClicked(void)
{
  bool ok;
  const double result = QInputDialog::getDouble(this, tr("Set simulation time to play:"),
                                                tr("Simulation time"), static_cast<double>(_player->currentTime()),
                                                static_cast<double>(_player->data()->startTime()),
                                                static_cast<double>(_player->data()->endTime()), 3, &ok, Qt::Popup);

  if (ok)
  {
    float percentage = (result - _player->data()->startTime()) /
                       (_player->data()->endTime() - _player->data()->startTime());

    percentage = std::max(0.0f, std::min(1.0f, percentage));

    PlayAt(percentage, true);
  }
}

void MainWindow::loadComplete(void)
{
  _summary->showMarker(false);
}

void MainWindow::addCorrelation(const std::string &subset)
{
  _correlations.push_back(subset);
}

void MainWindow::calculateCorrelations(void)
{
  visimpl::CorrelationComputer cc(dynamic_cast<simil::SpikeData*>(_player->data()));

  const auto eventNames = _subsetEventManager->eventNames();

  constexpr double deltaTime = 0.125;

  cc.configureEvents(eventNames, deltaTime);

  auto correlateSubsets = [&eventNames, deltaTime, &cc](const std::string &event)
  {
    cc.correlateSubset( event, eventNames, deltaTime, 2600, 2900 );
  };
  std::for_each(_correlations.cbegin(), _correlations.cend(), correlateSubsets);

  const auto names = cc.correlationNames();

  auto addHistogram = [this, &cc](const std::string &name)
  {
    auto correlation = cc.correlation( name );

    if( !correlation ) return;

    visimpl::Selection selection;
    selection.name = correlation->fullName;
    selection.gids = cc.getCorrelatedNeurons( correlation->fullName );

    _summary->AddNewHistogram( selection );
  };
  std::for_each(names.cbegin(), names.cend(), addHistogram);
}

void MainWindow::aboutDialog( void )
{
  QString msj =
    QString( "<h2>ViSimpl - StackViz</h2>" ) +
    tr( "A multi-view visual analyzer of brain simulation data. " ) + "<br>" +
    tr( "Version " ) + stackviz::Version::getString( ).c_str( ) +
    tr( " rev (%1)<br>").arg(stackviz::Version::getRevision( )) +
    "<a href='https://vg-lab.es/visimpl/'>https://vg-lab.es/visimpl</a>" +
    "<h4>" + tr( "Build info:" ) + "</h4>" +
    "<ul>"

#ifdef VISIMPL_USE_GMRVLEX
    "</li><li>GmrvLex " + GMRVLEX_REV_STRING +
#else
    "</li><li>GmrvLex " + tr ("support not built.") +
#endif

#ifdef VISIMPL_USE_PREFR
    "</li><li>prefr " + PREFR_REV_STRING +
#else
    "</li><li>prefr " + tr ("support not built.") +
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
    "</li><li>Brion " + BRION_REV_STRING +
#else
    "</li><li>SimIL " + tr ("support not built.") +
#endif

#if defined(SIMIL_USE_BRION) and defined(VISIMPL_USE_SIMIL)
    "</li><li>Brion " + BRION_REV_STRING +
#else
    "</li><li>Brion " + tr ("support not built.") +
#endif

#ifdef VISIMPL_USE_ZEROEQ
    "</li><li>ZeroEQ " + ZEROEQ_REV_STRING +
#else
    "</li><li>ZeroEQ " + tr ("support not built.") +
#endif

  "</li></ul>" +
  "<h4>" + tr( "Developed by:" ) + "</h4>" +
  "VG-Lab / URJC / UPM"
  "<br><a href='https://vg-lab.es'>https://vg-lab.es</a>"
  "<br>(C) 2015-" + QString::number(QDateTime::currentDateTime().date().year()) + "<br><br>"
  "<a href='https://vg-lab.es><img src=':/icons/logoVGLab.png'/></a>"
  "&nbsp;&nbsp;&nbsp;&nbsp;"
  "<a href='https://www.urjc.es'><img src=':/icons/logoURJC.png' /></a>"
  "&nbsp;&nbsp;&nbsp;&nbsp;"
  "<a href='https://www.upm.es'><img src=':/icons/logoUPM.png' /></a>";

  QMessageBox::about(this, tr( "About StackViz" ), msj );
}

void MainWindow::openH5FilesThroughDialog(void)
{
  const auto networkFilename = QFileDialog::getOpenFileName(this, tr("Open a H5 network file"),
                                                            _lastOpenedFileNamePath,
                                                            tr("hdf5 ( *.h5);; All files (*)"), nullptr,
                                                            QFileDialog::DontUseNativeDialog);

  if (networkFilename.isEmpty()) return;

  const auto activityFilename = QFileDialog::getOpenFileName(this, tr("Open a H5 activity file"),
                                                             _lastOpenedFileNamePath,
                                                             tr("hdf5 ( *.h5);; All files (*)"),
                                                             nullptr, QFileDialog::DontUseNativeDialog);

  if (activityFilename.isEmpty()) return;

  QApplication::setOverrideCursor(Qt::WaitCursor);
  openHDF5File(networkFilename.toStdString(), simil::TSimSpikes, activityFilename.toStdString());
  QApplication::restoreOverrideCursor();
}

void MainWindow::updateUIonOpen(const std::string &eventsFile)
{
  configurePlayer( );
  initSummaryWidget( );

  openSubsetEventFile( eventsFile, true );

  _summary->generateEventsRep( );
  _summary->importSubsetsFromSubsetMngr( );

  if( _displayManager )
    _displayManager->refresh( );

  _ui->actionShowDataManager->setEnabled(true);
}

