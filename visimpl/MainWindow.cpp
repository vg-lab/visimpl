/*
 * Copyright (c) 2015-2020 GMRV/URJC.
 *
 * Authors: Sergio E. Galindo <sergio.galindo@urjc.es>
 *
 * This file is part of ViSimpl <https://github.com/gmrvvis/visimpl>
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
#ifdef VISIMPL_USE_DEFLECT
#include <deflect/version.h>
#endif
#ifdef VISIMPL_USE_NSOL
#include <nsol/version.h>
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
#include <visimpl/version.h>

#include "MainWindow.h"

#include <QDebug>
#include <QFileDialog>
#include <QInputDialog>
#include <QGridLayout>
#include <QShortcut>
#include <QMessageBox>
#include <QColorDialog>

// #include "qt/CustomSlider.h"

#ifdef VISIMPL_USE_GMRVLEX
#include <gmrvlex/gmrvlex.h>
#endif

#include <thread>

#include <sumrice/sumrice.h>

namespace visimpl
{
  enum toolIndex
  {
    T_TOOL_Playback = 0,
    T_TOOL_Visual,
    T_TOOL_Selection,
    T_TOOL_Inpector
  };

  MainWindow::MainWindow( QWidget* parent_, bool updateOnIdle )
    : QMainWindow( parent_ )
#ifdef VISIMPL_USE_GMRVLEX
    , _zeqConnection( false )
    , _subscriber( nullptr )
    , _thread( nullptr )
#endif
    , _ui( new Ui::MainWindow )
    , _lastOpenedNetworkFileName( "" )
    , _openGLWidget( nullptr )
    , _domainManager( nullptr )
    , _subsetEvents( nullptr )
    , _summary( nullptr )
    , _simulationDock( nullptr )
    , _simSlider( nullptr )
    , _playButton( nullptr )
    , _startTimeLabel( nullptr )
    , _endTimeLabel( nullptr )
    , _repeatButton( nullptr )
    , _goToButton( nullptr )
    , _simConfigurationDock( nullptr )
    , _modeSelectionWidget( nullptr )
    , _toolBoxOptions( nullptr )
    , _groupBoxTransferFunction( nullptr )
    , _tfWidget( nullptr )
    , _selectionManager( nullptr )
    , _subsetImporter( nullptr )
    , _autoNameGroups( false )
    , _groupLayout( nullptr )
    , _decayBox( nullptr )
    , _deltaTimeBox( nullptr )
    , _timeStepsPSBox( nullptr )
    , _stepByStepDurationBox( nullptr )
    , _circuitScaleX( nullptr )
    , _circuitScaleY( nullptr )
    , _circuitScaleZ( nullptr )
    , _buttonImportGroups( nullptr )
    , _buttonClearGroups( nullptr )
    , _buttonAddGroup( nullptr )
    , _buttonClearSelection( nullptr )
    , _selectionSizeLabel( nullptr )
    , _alphaNormalButton( nullptr )
    , _alphaAccumulativeButton( nullptr )
    , _labelGID( nullptr )
    , _labelPosition( nullptr )
    , _groupBoxAttrib( nullptr )
    , _comboAttribSelection( nullptr )
    , _layoutAttribStats( nullptr )
    , _layoutAttribGroups( nullptr )
    , _checkClipping( nullptr )
    , _checkShowPlanes( nullptr )
    , _buttonResetPlanes( nullptr )
    , _spinBoxClippingHeight( nullptr )
    , _spinBoxClippingWidth( nullptr )
    , _spinBoxClippingDist( nullptr )
    , _frameClippingColor( nullptr )
    , _buttonSelectionFromClippingPlanes( nullptr )
  {
    _ui->setupUi( this );

    _ui->actionUpdateOnIdle->setChecked( updateOnIdle );
    _ui->actionShowFPSOnIdleUpdate->setChecked( false );

    _ui->actionShowEventsActivity->setChecked( false );

#ifdef SIMIL_USE_BRION
    _ui->actionOpenBlueConfig->setEnabled( true );
#else
    _ui->actionOpenBlueConfig->setEnabled( false );
#endif
  }

  void MainWindow::init( const std::string& zeqUri )
  {
    _openGLWidget = new OpenGLWidget( 0, 0, zeqUri );
    this->setCentralWidget( _openGLWidget );
    qDebug( ) << _openGLWidget->format( );

    _openGLWidget->idleUpdate( _ui->actionUpdateOnIdle->isChecked( ) );

    connect( _ui->actionUpdateOnIdle, SIGNAL( triggered( void ) ),
             _openGLWidget, SLOT( toggleUpdateOnIdle( void ) ) );

    connect( _ui->actionBackgroundColor, SIGNAL( triggered( void ) ),
             _openGLWidget, SLOT( changeClearColor( void ) ) );

    connect( _ui->actionShowFPSOnIdleUpdate, SIGNAL( triggered( void ) ),
             _openGLWidget, SLOT( toggleShowFPS( void ) ) );

    connect( _ui->actionShowEventsActivity, SIGNAL( triggered( bool ) ),
             _openGLWidget, SLOT( showEventsActivityLabels( bool ) ) );

    connect( _ui->actionShowCurrentTime, SIGNAL( triggered( bool ) ),
             _openGLWidget, SLOT( showCurrentTimeLabel( bool ) ) );

    connect( _ui->actionOpenBlueConfig, SIGNAL( triggered( void ) ), this,
             SLOT( openBlueConfigThroughDialog( void ) ) );

    connect( _ui->actionOpenCSVFiles, SIGNAL( triggered( void ) ), this,
             SLOT( openCSVFilesThroughDialog( void ) ) );

    connect( _ui->actionOpenSubsetEventsFile, SIGNAL( triggered( void ) ), this,
             SLOT( openSubsetEventsFileThroughDialog( void ) ) );

    connect( _ui->actionCloseData, SIGNAL( triggered( void ) ), this,
             SLOT( closeData( void ) ) );

    connect( _ui->actionQuit, SIGNAL( triggered( void ) ),
             QApplication::instance( ), SLOT( quit( void ) ) );

    // Connect about dialog
    connect( _ui->actionAbout, SIGNAL( triggered( void ) ), this,
             SLOT( dialogAbout( void ) ) );

    connect( _ui->actionHome, SIGNAL( triggered( void ) ), _openGLWidget,
             SLOT( home( void ) ) );

    connect( _openGLWidget, SIGNAL( stepCompleted( void ) ), this,
             SLOT( completedStep( void ) ) );

    connect( _openGLWidget, SIGNAL( pickedSingle( unsigned int ) ), this,
             SLOT( updateSelectedStatsPickingSingle( unsigned int ) ) );

    QAction* actionTogglePause = new QAction( this );
    actionTogglePause->setShortcut( Qt::Key_Space );

    connect( actionTogglePause, SIGNAL( triggered( ) ), this,
             SLOT( PlayPause( ) ) );
    addAction( actionTogglePause );

#ifdef VISIMPL_USE_ZEROEQ
    _ui->actionShowInactive->setEnabled( true );
    _ui->actionShowInactive->setChecked( true );

    _openGLWidget->showInactive( true );

    connect( _ui->actionShowInactive, SIGNAL( toggled( bool ) ), this,
             SLOT( showInactive( bool ) ) );

#else
    _ui->actionShowSelection->setEnabled( false );
#endif

    _initPlaybackDock( );
    _initSimControlDock( );

    connect( _simulationDock->toggleViewAction( ), SIGNAL( toggled( bool ) ),
             _ui->actionTogglePlaybackDock, SLOT( setChecked( bool ) ) );

    connect( _ui->actionTogglePlaybackDock, SIGNAL( triggered( ) ), this,
             SLOT( togglePlaybackDock( ) ) );

    connect( _simConfigurationDock->toggleViewAction( ),
             SIGNAL( toggled( bool ) ), _ui->actionToggleSimConfigDock,
             SLOT( setChecked( bool ) ) );

    connect( _ui->actionToggleSimConfigDock, SIGNAL( triggered( ) ), this,
             SLOT( toggleSimConfigDock( ) ) );

#ifdef VISIMPL_USE_ZEROEQ

    _setZeqUri( zeqUri );

#endif
  }

  MainWindow::~MainWindow( void )
  {
    delete _ui;
  }

  void MainWindow::showStatusBarMessage( const QString& message )
  {
    _ui->statusbar->showMessage( message );
  }

  void MainWindow::configureComponents( void )
  {
    _domainManager = _openGLWidget->domainManager( );
    _selectionManager->setGIDs( _domainManager->gids( ) );

    _subsetEvents = _openGLWidget->player( )->data( )->subsetsEvents( );
  }

  void MainWindow::openBlueConfig( const std::string& fileName,
                                   simil::TSimulationType simulationType,
                                   const std::string& reportLabel,
                                   const std::string& subsetEventFile )
  {
    _openGLWidget->loadData( fileName, simil::TDataType::TBlueConfig,
                             simulationType, reportLabel );

    configureComponents( );

    openSubsetEventFile( subsetEventFile, false );

    _configurePlayer( );

    QStringList attributes = {"Morphological type", "Functional type"};

    _comboAttribSelection->addItems( attributes );
  }

  void MainWindow::openBlueConfigThroughDialog( void )
  {
#ifdef SIMIL_USE_BRION

    QString path = QFileDialog::getOpenFileName(
      this, tr( "Open BlueConfig" ), _lastOpenedNetworkFileName,
      tr( "BlueConfig ( BlueConfig CircuitConfig);; All files (*)" ), nullptr,
      QFileDialog::DontUseNativeDialog );

    if ( path != QString( "" ) )
    {
      bool ok;

      simil::TSimulationType simType = simil::TSimSpikes;
      QString text = QInputDialog::getText( this, tr( "Please type a target" ),
                                            tr( "Target name:" ),
                                            QLineEdit::Normal, "Mosaic", &ok );

      if ( ok && !text.isEmpty( ) )
      {
        //      std::string targetLabel = text.toStdString( );
        std::string reportLabel = text.toStdString( );
        _lastOpenedNetworkFileName = QFileInfo( path ).path( );
        std::string fileName = path.toStdString( );
        openBlueConfig( fileName, simType, reportLabel );
      }
    }
#endif
  }

  void MainWindow::openCSVFilesThroughDialog( void )
  {
    QString pathNetwork = QFileDialog::getOpenFileName(
      this, tr( "Open CSV Network description file" ),
      _lastOpenedNetworkFileName, tr( "CSV (*.csv);; All files (*)" ), nullptr,
      QFileDialog::DontUseNativeDialog );

    if ( pathNetwork != QString( "" ) )
    {
      simil::TSimulationType simType = simil::TSimSpikes;

      QString pathActivity = QFileDialog::getOpenFileName(
        this, tr( "Open CSV Activity file" ), _lastOpenedNetworkFileName,
        tr( "CSV (*.csv);; All files (*)" ), nullptr,
        QFileDialog::DontUseNativeDialog );

      if ( !pathActivity.isEmpty( ) )
      {
        //      std::string targetLabel = text.toStdString( );
        _lastOpenedNetworkFileName = QFileInfo( pathNetwork ).path( );
        _lastOpenedActivityFileName = QFileInfo( pathActivity ).path( );
        std::string networkFile = pathNetwork.toStdString( );
        std::string activityFile = pathActivity.toStdString( );
        openCSVFile( networkFile, simType, activityFile );
      }
    }
  }

  void MainWindow::openHDF5File( const std::string& networkFile,
                                 simil::TSimulationType simulationType,
                                 const std::string& activityFile,
                                 const std::string& subsetEventFile )
  {
    _openGLWidget->loadData( networkFile, simil::TDataType::THDF5,
                             simulationType, activityFile );

    configureComponents( );

    openSubsetEventFile( subsetEventFile, false );

    _configurePlayer( );
  }

  void MainWindow::openCSVFile( const std::string& networkFile,
                                simil::TSimulationType simulationType,
                                const std::string& activityFile,
                                const std::string& subsetEventFile )
  {
    _openGLWidget->loadData( networkFile, simil::TDataType::TCSV,
                             simulationType, activityFile );

    configureComponents( );

    openSubsetEventFile( subsetEventFile, false );

    _configurePlayer( );
  }

#ifdef SIMIL_WITH_REST_API
  void MainWindow::openRestListener( const std::string& url,
                                     simil::TSimulationType simulationType,
                                     const std::string& port,
                                     const std::string& )
  {
    _openGLWidget->loadRestData( url, simil::TDataType::TREST, simulationType,
                                 port );

    configureComponents( );

    _configurePlayer( );
  }
#endif
  void MainWindow::openHDF5ThroughDialog( void )
  {
    QString path = QFileDialog::getOpenFileName(
      this, tr( "Open a H5 network file" ), _lastOpenedNetworkFileName,
      tr( "hdf5 ( *.h5);; All files (*)" ), nullptr,
      QFileDialog::DontUseNativeDialog );

    std::string networkFile;
    std::string activityFile;

    if ( path == QString( "" ) )
      return;

    networkFile = path.toStdString( );

    path =
      QFileDialog::getOpenFileName( this, tr( "Open a H5 activity file" ), path,
                                    tr( "hdf5 ( *.h5);; All files (*)" ),
                                    nullptr, QFileDialog::DontUseNativeDialog );

    if ( path == QString( "" ) )
    {
      return;
    }

    activityFile = path.toStdString( );

    openHDF5File( networkFile, simil::TSimSpikes, activityFile );
  }

  void MainWindow::openSubsetEventFile( const std::string& filePath,
                                        bool append )
  {
    if ( filePath.empty( ) || !_subsetEvents )
      return;

    if ( !append )
      _subsetEvents->clear( );

    if ( filePath.find( "json" ) != std::string::npos )
    {
      std::cout << "Loading JSON file: " << filePath << std::endl;
      _subsetEvents->loadJSON( filePath );

      _subsetImporter->reload( _subsetEvents );

      _openGLWidget->subsetEventsManager( _subsetEvents );
      _openGLWidget->showEventsActivityLabels(
        _ui->actionShowEventsActivity->isChecked( ) );
    }
    else if ( filePath.find( "h5" ) != std::string::npos )
    {
      std::cout << "Loading H5 file: " << filePath << std::endl;
      _subsetEvents->loadH5( filePath );

      _subsetImporter->reload( _subsetEvents );

      _openGLWidget->subsetEventsManager( _subsetEvents );
      _openGLWidget->showEventsActivityLabels(
        _ui->actionShowEventsActivity->isChecked( ) );
    }
    else
    {
      std::cout << "Subset Events file not found: " << filePath << std::endl;
    }
  }

  void MainWindow::openSubsetEventsFileThroughDialog( void )
  {
    QString filePath = QFileDialog::getOpenFileName(
      this, tr( "Open file containing subsets/events data" ),
      _lastOpenedSubsetsFileName, tr( "JSON (*.json);; All files (*)" ),
      nullptr, QFileDialog::DontUseNativeDialog );

    if ( !filePath.isEmpty( ) )
    {
      _lastOpenedSubsetsFileName = QFileInfo( filePath ).path( );

      openSubsetEventFile( filePath.toStdString( ), false );
    }
  }

  void MainWindow::closeData( void )
  {
    // TODO
  }

  void MainWindow::dialogAbout( void )
  {
    QString msj =
      QString( "<h2>ViSimpl</h2>" ) +
      tr( "A multi-view visual analyzer of brain simulation data. " ) + "<br>" +
      tr( "Version " ) + visimpl::Version::getString( ).c_str( ) +
      tr( " rev (%1)<br>" ).arg( visimpl::Version::getRevision( ) ) +
      "<a href='https://gmrv.es/visimpl/'>https://gmrv.es/visimpl</a>" +
      "<h4>" + tr( "Build info:" ) + "</h4>" +
      "<ul>"

#ifdef VISIMPL_USE_DEFLECT
      "</li><li>Deflect " +
      DEFLECT_REV_STRING +
#else
      "</li><li>Deflect " +
      tr( "support not built." ) +
#endif

#ifdef VISIMPL_USE_FIRES
      "</li><li>FiReS " + FIRES_REV_STRING +
#else
      "</li><li>FiReS " + tr( "support not built." ) +
#endif

#ifdef VISIMPL_USE_GMRVLEX
      "</li><li>GmrvLex " + GMRVLEX_REV_STRING +
#else
      "</li><li>GmrvLex " + tr( "support not built." ) +
#endif

#ifdef VISIMPL_USE_NSOL
      "</li><li>Nsol " + NSOL_REV_STRING +
#else
      "</li><li>Nsol " + tr( "support not built." ) +
#endif

#ifdef VISIMPL_USE_PREFR
      "</li><li>prefr " + PREFR_REV_STRING +
#else
      "</li><li>prefr " + tr( "support not built." ) +
#endif

#ifdef VISIMPL_USE_RETO
      "</li><li>ReTo " + RETO_REV_STRING +
#else
      "</li><li>ReTo " + tr( "support not built." ) +
#endif

#ifdef VISIMPL_USE_SCOOP
      "</li><li>Scoop " + SCOOP_REV_STRING +
#else
      "</li><li>Scoop " + tr( "support not built." ) +
#endif

#ifdef VISIMPL_USE_SIMIL
      "</li><li>SimIL " + SIMIL_REV_STRING +
#else
      "</li><li>SimIL " + tr( "support not built." ) +
#endif

#ifdef VISIMPL_USE_ZEROEQ
      "</li><li>ZeroEQ " + ZEROEQ_REV_STRING +
#else
      "</li><li>ZeroEQ " + tr( "support not built." ) +
#endif

      "</li></ul>" + "<h4>" + tr( "Developed by:" ) + "</h4>" +
      "GMRV / URJC / UPM"
      "<br><a href='https://gmrv.es/gmrvvis'>https://gmrv.es/gmrvvis</a>"
      //"<br><a href='mailto:gmrv@gmrv.es'>gmrv@gmrv.es</a><br><br>"
      "<br>(C) 2015-2017<br><br>"
      "<a href='https://gmrv.es/gmrvvis'><img src=':/icons/logoGMRV.png'/></a>"
      "&nbsp;&nbsp;&nbsp;&nbsp;"
      "<a href='https://www.urjc.es'><img src=':/icons/logoURJC.png' /></a>"
      "&nbsp;&nbsp;&nbsp;&nbsp;"
      "<a href='https://www.upm.es'><img src=':/icons/logoUPM.png' /></a>";

    QMessageBox::about( this, tr( "About ViSimpl" ), msj );
  }

  void MainWindow::dialogSelectionManagement( void )
  {
    if ( !_selectionManager )
      return;

    _selectionManager->show( );
  }

  void MainWindow::dialogSubsetImporter( void )
  {
    if ( !_subsetImporter )
      return;
    _subsetImporter->reload( _subsetEvents );
    _subsetImporter->show( );
  }

  void MainWindow::togglePlaybackDock( void )
  {
    if ( _ui->actionTogglePlaybackDock->isChecked( ) )
      _simulationDock->show( );
    else
      _simulationDock->close( );

    update( );
  }

  void MainWindow::toggleSimConfigDock( void )
  {
    if ( _ui->actionToggleSimConfigDock->isChecked( ) )
      _simConfigurationDock->show( );
    else
      _simConfigurationDock->close( );

    update( );
  }

  void MainWindow::_configurePlayer( void )
  {
    connect( _openGLWidget, SIGNAL( updateSlider( float ) ), this,
             SLOT( UpdateSimulationSlider( float ) ) );

#ifdef SIMIL_WITH_REST_API
    _objectInspectorGB->setSimPlayer(_openGLWidget->player( ));
    _subsetEvents = _openGLWidget->subsetEventsManager( );
#endif
    _startTimeLabel->setText(
      QString::number( ( double )_openGLWidget->player( )->startTime( ) ) );

    _endTimeLabel->setText(
      QString::number( ( double )_openGLWidget->player( )->endTime( ) ) );

#ifdef SIMIL_USE_ZEROEQ
    _openGLWidget->player( )->zeqEvents( )->playbackOpReceived.connect(
      boost::bind( &MainWindow::ApplyPlaybackOperation, this, _1 ) );

    _openGLWidget->player( )->zeqEvents( )->frameReceived.connect(
      boost::bind( &MainWindow::requestPlayAt, this, _1 ) );

#endif

    changeEditorColorMapping( );
    changeEditorSimDeltaTime( );
    changeEditorSimTimestepsPS( );
    changeEditorSizeFunction( );
    changeEditorDecayValue( );
    changeEditorStepByStepDuration( );
    changeCircuitScaleValue( );
    _initSummaryWidget( );
  }

  void MainWindow::_initPlaybackDock( void )
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
    _simSlider->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );

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

    _goToButton = new QPushButton( );
    _goToButton->setText( QString( "Play at..." ) );

    QIcon stopIcon;
    QIcon nextIcon;
    QIcon prevIcon;
    QIcon repeatIcon;

    _playIcon.addFile( QStringLiteral( ":/icons/play.png" ), QSize( ),
                       QIcon::Normal, QIcon::Off );
    _pauseIcon.addFile( QStringLiteral( ":/icons/pause.png" ), QSize( ),
                        QIcon::Normal, QIcon::Off );
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
    dockLayout->addWidget( _endTimeLabel, row, totalHSpan - 2, 1, 1,
                           Qt::AlignRight );

    row++;
    dockLayout->addWidget( _repeatButton, row, 7, 1, 1 );
    dockLayout->addWidget( prevButton, row, 8, 1, 1 );
    dockLayout->addWidget( _playButton, row, 9, 2, 2 );
    dockLayout->addWidget( stopButton, row, 11, 1, 1 );
    dockLayout->addWidget( nextButton, row, 12, 1, 1 );
    dockLayout->addWidget( _goToButton, row, 13, 1, 1 );

    connect( _playButton, SIGNAL( clicked( ) ), this, SLOT( PlayPause( ) ) );

    connect( stopButton, SIGNAL( clicked( ) ), this, SLOT( Stop( ) ) );

    connect( nextButton, SIGNAL( clicked( ) ), this, SLOT( NextStep( ) ) );

    connect( prevButton, SIGNAL( clicked( ) ), this, SLOT( PreviousStep( ) ) );

    connect( _repeatButton, SIGNAL( clicked( ) ), this, SLOT( Repeat( ) ) );

    connect( _simSlider, SIGNAL( sliderPressed( ) ), this, SLOT( PlayAt( ) ) );

    connect( _goToButton, SIGNAL( clicked( ) ), this,
             SLOT( playAtButtonClicked( ) ) );

    //  connect( _simSlider, SIGNAL( sliderMoved( )),
    //             this, SLOT( PlayAt( )));

    _summary = new visimpl::Summary( nullptr, visimpl::T_STACK_FIXED );
    //  _summary->setVisible( false );
    _summary->setMinimumHeight( 50 );

    dockLayout->addWidget( _summary, 0, 1, 2, totalHSpan - 3 );

    _simulationDock->setWidget( content );
    this->addDockWidget(
      Qt::/*DockWidgetAreas::enum_type::*/ BottomDockWidgetArea,
      _simulationDock );

    connect( _summary, SIGNAL( histogramClicked( float ) ), this,
             SLOT( PlayAt( float ) ) );
  }

  void MainWindow::_initSimControlDock( void )
  {
    _simConfigurationDock = new QDockWidget( );
    _simConfigurationDock->setMinimumHeight( 100 );
    _simConfigurationDock->setMinimumWidth( 300 );
    _simConfigurationDock->setSizePolicy( QSizePolicy::MinimumExpanding,
                                          QSizePolicy::MinimumExpanding );

    _tfWidget = new TransferFunctionWidget( );
    _tfWidget->setMinimumHeight( 100 );

    _subsetImporter = new SubsetImporter( );
    _subsetImporter->setMinimumHeight( 300 );
    _subsetImporter->setMinimumWidth( 500 );

    connect( _subsetImporter, SIGNAL( clickedAccept( void ) ), this,
             SLOT( importVisualGroups( void ) ) );

    _selectionManager = new SelectionManagerWidget( );
    //    _selectionManager->setWindowFlags( Qt::D );
    _selectionManager->setWindowModality( Qt::WindowModal );
    _selectionManager->setMinimumHeight( 300 );
    _selectionManager->setMinimumWidth( 500 );

    connect( _selectionManager, SIGNAL( selectionChanged( void ) ), this,
             SLOT( selectionManagerChanged( void ) ) );

    _deltaTimeBox = new QDoubleSpinBox( );
    _deltaTimeBox->setMinimum( 0.00000001 );
    _deltaTimeBox->setMaximum( 50 );
    _deltaTimeBox->setSingleStep( 0.05 );
    _deltaTimeBox->setDecimals( 5 );
    _deltaTimeBox->setMaximumWidth( 100 );

    _timeStepsPSBox = new QDoubleSpinBox( );
    _timeStepsPSBox->setMinimum( 0.00000001 );
    _timeStepsPSBox->setMaximum( 50 );
    _timeStepsPSBox->setSingleStep( 1.0 );
    _timeStepsPSBox->setDecimals( 5 );
    _timeStepsPSBox->setMaximumWidth( 100 );

    _stepByStepDurationBox = new QDoubleSpinBox( );
    _stepByStepDurationBox->setMinimum( 0.5 );
    _stepByStepDurationBox->setMaximum( 50 );
    _stepByStepDurationBox->setSingleStep( 1.0 );
    _stepByStepDurationBox->setDecimals( 3 );
    _stepByStepDurationBox->setMaximumWidth( 100 );

    _decayBox = new QDoubleSpinBox( );
    _decayBox->setMinimum( 0.01 );
    _decayBox->setMaximum( 600.0 );
    _decayBox->setDecimals( 5 );
    _decayBox->setMaximumWidth( 100 );

    _alphaNormalButton = new QRadioButton( "Normal" );
    _alphaAccumulativeButton = new QRadioButton( "Accumulative" );
    _openGLWidget->SetAlphaBlendingAccumulative( false );

    _buttonClearSelection = new QPushButton( "Discard" );
    _buttonClearSelection->setEnabled( false );
    _selectionSizeLabel = new QLabel( "0" );

    _buttonAddGroup = new QPushButton( "Add group" );
    _buttonAddGroup->setEnabled( false );
    _buttonAddGroup->setToolTip(
      "Click to create a group from current selection." );

    _labelGID = new QLabel( "" );
    _labelPosition = new QLabel( "" );

    _checkClipping = new QCheckBox( tr( "Clipping" ) );
    _checkShowPlanes = new QCheckBox( "Show planes" );
    _buttonResetPlanes = new QPushButton( "Reset" );
    _spinBoxClippingHeight = new QDoubleSpinBox( );
    _spinBoxClippingHeight->setDecimals( 2 );
    _spinBoxClippingHeight->setMinimum( 1 );
    _spinBoxClippingHeight->setMaximum( 99999 );

    _spinBoxClippingWidth = new QDoubleSpinBox( );
    _spinBoxClippingWidth->setDecimals( 2 );
    _spinBoxClippingWidth->setMinimum( 1 );
    _spinBoxClippingWidth->setMaximum( 99999 );

    _spinBoxClippingDist = new QDoubleSpinBox( );
    _spinBoxClippingDist->setDecimals( 2 );
    _spinBoxClippingDist->setMinimum( 1 );
    _spinBoxClippingDist->setMaximum( 99999 );

    QColor clippingColor( 255, 255, 255, 255 );
    _frameClippingColor = new QPushButton( );
    //      auto colors = _openGLWidget->colorPalette( ).colors( );
    // colors[ currentIndex % colors.size( )].first
    _frameClippingColor->setStyleSheet( "background-color: " +
                                        clippingColor.name( ) );
    _frameClippingColor->setMinimumSize( 20, 20 );
    _frameClippingColor->setMaximumSize( 20, 20 );

    _buttonSelectionFromClippingPlanes = new QPushButton( "To selection" );
    _buttonSelectionFromClippingPlanes->setToolTip(
      tr( "Create a selection set from elements between planes" ) );

    _circuitScaleX = new QDoubleSpinBox( );
    _circuitScaleX->setDecimals( 2 );
    _circuitScaleX->setMinimum( 0.01 );
    _circuitScaleX->setMaximum( 9999999 );
    _circuitScaleX->setSingleStep( 0.5 );
    _circuitScaleX->setMaximumWidth( 100 );

    _circuitScaleY = new QDoubleSpinBox( );
    _circuitScaleY->setDecimals( 2 );
    _circuitScaleY->setMinimum( 0.01 );
    _circuitScaleY->setMaximum( 9999999 );
    _circuitScaleY->setSingleStep( 0.5 );
    _circuitScaleY->setMaximumWidth( 100 );

    _circuitScaleZ = new QDoubleSpinBox( );
    _circuitScaleZ->setDecimals( 2 );
    _circuitScaleZ->setMinimum( 0.01 );
    _circuitScaleZ->setMaximum( 9999999 );
    _circuitScaleZ->setSingleStep( 0.5 );
    _circuitScaleZ->setMaximumWidth( 100 );

    QWidget* topContainer = new QWidget( );
    QVBoxLayout* verticalLayout = new QVBoxLayout( );

    _groupBoxTransferFunction =
      new QGroupBox( "Color and Size transfer function" );
    QVBoxLayout* tfLayout = new QVBoxLayout( );
    tfLayout->addWidget( _tfWidget );
    _groupBoxTransferFunction->setLayout( tfLayout );

    QGroupBox* tSpeedGB = new QGroupBox( "Simulation playback Configuration" );
    QGridLayout* sfLayout = new QGridLayout( );
    sfLayout->setAlignment( Qt::AlignTop );
    sfLayout->addWidget( new QLabel( "Simulation timestep:" ), 0, 0, 1, 1 );
    sfLayout->addWidget( _deltaTimeBox, 0, 1, 1, 1 );
    sfLayout->addWidget( new QLabel( "Timesteps per second:" ), 1, 0, 1, 1 );
    sfLayout->addWidget( _timeStepsPSBox, 1, 1, 1, 1 );
    sfLayout->addWidget( new QLabel( "Step playback duration (s):" ), 2, 0, 1,
                         1 );
    sfLayout->addWidget( _stepByStepDurationBox, 2, 1, 1, 1 );
    tSpeedGB->setLayout( sfLayout );

    QGroupBox* scaleGB = new QGroupBox( "Scale factor (X,Y,Z)" );
    QHBoxLayout* layoutScale = new QHBoxLayout( );
    scaleGB->setLayout( layoutScale );
    layoutScale->addWidget( _circuitScaleX );
    layoutScale->addWidget( _circuitScaleY );
    layoutScale->addWidget( _circuitScaleZ );

    QComboBox* comboShader = new QComboBox( );
    comboShader->addItems( {"Default", "Solid"} );
    comboShader->setCurrentIndex( 0 );

    QGroupBox* shaderGB = new QGroupBox( "Shader Configuration" );
    QHBoxLayout* shaderLayout = new QHBoxLayout( );
    shaderLayout->addWidget( new QLabel( "Current shader: " ) );
    shaderLayout->addWidget( comboShader );
    shaderGB->setLayout( shaderLayout );

    QGroupBox* dFunctionGB = new QGroupBox( "Decay function" );
    QHBoxLayout* dfLayout = new QHBoxLayout( );
    dfLayout->setAlignment( Qt::AlignTop );
    dfLayout->addWidget( new QLabel( "Decay (simulation time): " ) );
    dfLayout->addWidget( _decayBox );
    dFunctionGB->setLayout( dfLayout );

    QGroupBox* rFunctionGB = new QGroupBox( "Alpha blending function" );
    QHBoxLayout* rfLayout = new QHBoxLayout( );
    rfLayout->setAlignment( Qt::AlignTop );
    rfLayout->addWidget( new QLabel( "Alpha Blending: " ) );
    rfLayout->addWidget( _alphaNormalButton );
    rfLayout->addWidget( _alphaAccumulativeButton );
    rFunctionGB->setLayout( rfLayout );

    // Visual Configuration Container
    QWidget* vcContainer = new QWidget( );
    QVBoxLayout* vcLayout = new QVBoxLayout( );
    vcLayout->setAlignment( Qt::AlignTop );
    vcLayout->addWidget( scaleGB );
    vcLayout->addWidget( shaderGB );
    vcLayout->addWidget( dFunctionGB );
    vcLayout->addWidget( rFunctionGB );
    vcContainer->setLayout( vcLayout );

    QPushButton* buttonSelectionManager = new QPushButton( "..." );
    buttonSelectionManager->setToolTip(
      tr( "Show the selection management dialog" ) );
    buttonSelectionManager->setMaximumWidth( 30 );

    QGroupBox* selFunctionGB = new QGroupBox( "Current selection" );
    QHBoxLayout* selLayout = new QHBoxLayout( );
    selLayout->setAlignment( Qt::AlignTop );
    selLayout->addWidget( new QLabel( "Size: " ) );
    selLayout->addWidget( _selectionSizeLabel );
    selLayout->addWidget( buttonSelectionManager );
    selLayout->addWidget( _buttonAddGroup );
    selLayout->addWidget( _buttonClearSelection );
    selFunctionGB->setLayout( selLayout );

    QFrame* line = new QFrame( );
    line->setFrameShape( QFrame::HLine );
    line->setFrameShadow( QFrame::Sunken );

    QGroupBox* gbClippingPlanes = new QGroupBox( "Clipping planes" );
    QGridLayout* layoutClippingPlanes = new QGridLayout( );
    gbClippingPlanes->setLayout( layoutClippingPlanes );
    layoutClippingPlanes->addWidget( _checkClipping, 0, 0, 1, 1 );
    layoutClippingPlanes->addWidget( _checkShowPlanes, 0, 1, 1, 2 );
    layoutClippingPlanes->addWidget( _buttonSelectionFromClippingPlanes, 0, 3,
                                     1, 1 );
    layoutClippingPlanes->addWidget( line, 1, 0, 1, 4 );
    layoutClippingPlanes->addWidget( _frameClippingColor, 2, 0, 1, 1 );
    layoutClippingPlanes->addWidget( _buttonResetPlanes, 2, 1, 1, 1 );
    layoutClippingPlanes->addWidget( new QLabel( "Height" ), 2, 2, 1, 1 );
    layoutClippingPlanes->addWidget( _spinBoxClippingHeight, 2, 3, 1, 1 );
    layoutClippingPlanes->addWidget( new QLabel( "Distance" ), 3, 0, 1, 1 );
    layoutClippingPlanes->addWidget( _spinBoxClippingDist, 3, 1, 1, 1 );
    layoutClippingPlanes->addWidget( new QLabel( "Width" ), 3, 2, 1, 1 );
    layoutClippingPlanes->addWidget( _spinBoxClippingWidth, 3, 3, 1, 1 );

    QWidget* containerSelectionTools = new QWidget( );
    QVBoxLayout* layoutContainerSelection = new QVBoxLayout( );
    containerSelectionTools->setLayout( layoutContainerSelection );

    layoutContainerSelection->addWidget( selFunctionGB );
    layoutContainerSelection->addWidget( gbClippingPlanes );

#ifdef SIMIL_WITH_REST_API
    _objectInspectorGB = new DataInspector( "Object inspector" );
    /*QGroupBox* objectInspectoGB = new QGroupBox( "Object inspector" );
    QGridLayout* oiLayout = new QGridLayout( );
    oiLayout->setAlignment( Qt::AlignTop );*/
    _objectInspectorGB->addWidget( new QLabel( "GID:" ), 2, 0, 1, 1 );
    _objectInspectorGB->addWidget( _labelGID, 2, 1, 1, 3 );
    _objectInspectorGB->addWidget( new QLabel( "Position: " ), 3, 0, 1, 1 );
    _objectInspectorGB->addWidget( _labelPosition, 3, 1, 1, 3 );
    // objectInspectoGB->setLayout( oiLayout );*/
#endif

    QGroupBox* groupBoxGroups = new QGroupBox( "Current visualization groups" );
    _groupLayout = new QVBoxLayout( );
    _groupLayout->setAlignment( Qt::AlignTop );

    _buttonImportGroups = new QPushButton( "Import from..." );
    _buttonClearGroups = new QPushButton( "Clear" );
    _buttonClearGroups->setEnabled( false );

    {
      QWidget* groupContainer = new QWidget( );
      groupContainer->setLayout( _groupLayout );

      QScrollArea* scrollGroups = new QScrollArea( );
      scrollGroups->setWidget( groupContainer );
      scrollGroups->setWidgetResizable( true );
      scrollGroups->setFrameShape( QFrame::Shape::NoFrame );
      scrollGroups->setFrameShadow( QFrame::Shadow::Plain );
      scrollGroups->setHorizontalScrollBarPolicy( Qt::ScrollBarAsNeeded );
      scrollGroups->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn );

      QGridLayout* groupOuterLayout = new QGridLayout( );
      groupOuterLayout->setMargin( 0 );
      groupOuterLayout->addWidget( _buttonImportGroups, 0, 0, 1, 1 );
      groupOuterLayout->addWidget( _buttonClearGroups, 0, 1, 1, 1 );

      groupOuterLayout->addWidget( scrollGroups, 1, 0, 1, 2 );

      groupBoxGroups->setLayout( groupOuterLayout );
    }

    _groupBoxAttrib = new QGroupBox( "Attribute Mapping" );
    QGridLayout* layoutGroupAttrib = new QGridLayout( );
    _groupBoxAttrib->setLayout( layoutGroupAttrib );

    _comboAttribSelection = new QComboBox( );

    QGroupBox* gbAttribSel = new QGroupBox( "Attribute selection" );
    QHBoxLayout* lyAttribSel = new QHBoxLayout( );
    lyAttribSel->addWidget( _comboAttribSelection );
    gbAttribSel->setLayout( lyAttribSel );

    QGroupBox* gbAttribStats = new QGroupBox( "Statistics" );
    QScrollArea* attribScroll = new QScrollArea( );
    QWidget* attribContainer = new QWidget( );
    QVBoxLayout* attribContainerLayout = new QVBoxLayout( );
    gbAttribStats->setLayout( attribContainerLayout );
    attribContainerLayout->addWidget( attribScroll );

    attribScroll->setWidget( attribContainer );
    attribScroll->setWidgetResizable( true );
    attribScroll->setFrameShape( QFrame::Shape::NoFrame );
    attribScroll->setFrameShadow( QFrame::Shadow::Plain );
    attribScroll->setHorizontalScrollBarPolicy( Qt::ScrollBarAsNeeded );
    attribScroll->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn );

    _layoutAttribStats = new QVBoxLayout( );
    _layoutAttribStats->setAlignment( Qt::AlignTop );
    attribContainer->setLayout( _layoutAttribStats );

    QGroupBox* gbAttribGroups = new QGroupBox( "Groups" );
    _layoutAttribGroups = new QGridLayout( );
    _layoutAttribGroups->setAlignment( Qt::AlignTop );
    {
      QWidget* groupContainer = new QWidget( );
      groupContainer->setLayout( _layoutAttribGroups );

      QScrollArea* groupScroll = new QScrollArea( );
      groupScroll->setWidget( groupContainer );
      groupScroll->setWidgetResizable( true );
      groupScroll->setFrameShape( QFrame::Shape::NoFrame );
      groupScroll->setFrameShadow( QFrame::Shadow::Plain );
      groupScroll->setHorizontalScrollBarPolicy( Qt::ScrollBarAsNeeded );
      groupScroll->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn );

      QGridLayout* groupOuterLayout = new QGridLayout( );
      groupOuterLayout->setMargin( 0 );
      groupOuterLayout->addWidget( groupScroll );

      gbAttribGroups->setLayout( groupOuterLayout );
    }

    layoutGroupAttrib->addWidget( gbAttribSel, 0, 0, 1, 2 );
    layoutGroupAttrib->addWidget( gbAttribGroups, 0, 2, 3, 2 );
    layoutGroupAttrib->addWidget( gbAttribStats, 1, 0, 2, 2 );

    QWidget* containerTabSelection = new QWidget( );
    QVBoxLayout* tabSelectionLayout = new QVBoxLayout( );
    containerTabSelection->setLayout( tabSelectionLayout );
    tabSelectionLayout->addWidget( _groupBoxTransferFunction );

    QWidget* containerTabGroups = new QWidget( );
    QVBoxLayout* tabGroupsLayout = new QVBoxLayout( );
    containerTabGroups->setLayout( tabGroupsLayout );
    tabGroupsLayout->addWidget( groupBoxGroups );

    QWidget* containerTabAttrib = new QWidget( );
    QVBoxLayout* tabAttribLayout = new QVBoxLayout( );
    containerTabAttrib->setLayout( tabAttribLayout );
    tabAttribLayout->addWidget( _groupBoxAttrib );

    _modeSelectionWidget = new QTabWidget( );
    _modeSelectionWidget->addTab( containerTabSelection, tr( "Selection" ) );
    _modeSelectionWidget->addTab( containerTabGroups, tr( "Groups" ) );
    _modeSelectionWidget->addTab( containerTabAttrib, tr( "Attribute" ) );

    _toolBoxOptions = new QToolBox( );
    _toolBoxOptions->addItem( tSpeedGB, tr( "Playback Configuration" ) );
    _toolBoxOptions->addItem( vcContainer, tr( "Visual Configuration" ) );
    _toolBoxOptions->addItem( containerSelectionTools, tr( "Selection" ) );


#ifdef SIMIL_WITH_REST_API
    _toolBoxOptions->addItem( _objectInspectorGB, tr( "Inspector" ));

    connect( _objectInspectorGB, SIGNAL( simDataChanged( void )),
             _openGLWidget, SLOT( updateData( void )));
#endif

    verticalLayout->setAlignment( Qt::AlignTop );
    verticalLayout->addWidget( _modeSelectionWidget );
    verticalLayout->addWidget( _toolBoxOptions );

    topContainer->setLayout( verticalLayout );
    _simConfigurationDock->setWidget( topContainer );

    this->addDockWidget(
      Qt::/*DockWidgetAreas::enum_type::*/ RightDockWidgetArea,
      _simConfigurationDock );

    connect( _objectInspectoGB, SIGNAL( simDataChanged( void )),
             _openGLWidget, SLOT( updateData( void )));

    QTimer* timer = new QTimer( this );
    connect( timer, SIGNAL( timeout ( void )), _objectInspectoGB,
             SLOT( updateInfo( void ) ) );

    timer->start( 4000 );

    connect( _objectInspectoGB, SIGNAL( simDataChanged( void ) ), _summary,
             SLOT( UpdateHistograms( void ) ) );
    connect( _modeSelectionWidget, SIGNAL( currentChanged( int ) ),
             _openGLWidget, SLOT( setMode( int ) ) );

    connect( _openGLWidget, SIGNAL( attributeStatsComputed( void ) ), this,
             SLOT( updateAttributeStats( void ) ) );

    connect( _comboAttribSelection, SIGNAL( currentIndexChanged( int ) ),
             _openGLWidget, SLOT( selectAttrib( int ) ) );

    connect( comboShader, SIGNAL( currentIndexChanged( int ) ), _openGLWidget,
             SLOT( changeShader( int ) ) );

    connect( _tfWidget, SIGNAL( colorChanged( void ) ), this,
             SLOT( UpdateSimulationColorMapping( void ) ) );
    connect( _tfWidget, SIGNAL( colorChanged( void ) ), this,
             SLOT( UpdateSimulationSizeFunction( void ) ) );
    connect( _tfWidget, SIGNAL( previewColor( void ) ), this,
             SLOT( PreviewSimulationColorMapping( void ) ) );
    connect( _tfWidget, SIGNAL( previewColor( void ) ), this,
             SLOT( PreviewSimulationSizeFunction( void ) ) );

    connect( buttonSelectionManager, SIGNAL( clicked( void ) ), this,
             SLOT( dialogSelectionManagement( void ) ) );

    connect( _circuitScaleX, SIGNAL( valueChanged( double ) ), this,
             SLOT( updateCircuitScaleValue( void ) ) );

    connect( _circuitScaleY, SIGNAL( valueChanged( double ) ), this,
             SLOT( updateCircuitScaleValue( void ) ) );

    connect( _circuitScaleZ, SIGNAL( valueChanged( double ) ), this,
             SLOT( updateCircuitScaleValue( void ) ) );

    connect( _deltaTimeBox, SIGNAL( valueChanged( double ) ), this,
             SLOT( updateSimDeltaTime( void ) ) );

    connect( _timeStepsPSBox, SIGNAL( valueChanged( double ) ), this,
             SLOT( updateSimTimestepsPS( void ) ) );

    connect( _decayBox, SIGNAL( valueChanged( double ) ), this,
             SLOT( updateSimulationDecayValue( void ) ) );

    connect( _stepByStepDurationBox, SIGNAL( valueChanged( double ) ), this,
             SLOT( updateSimStepByStepDuration( void ) ) );

    connect( _alphaNormalButton, SIGNAL( toggled( bool ) ), this,
             SLOT( AlphaBlendingToggled( void ) ) );

    // Clipping planes

    _checkClipping->setChecked( true );
    connect( _checkClipping, SIGNAL( stateChanged( int ) ), this,
             SLOT( clippingPlanesActive( int ) ) );
    _checkClipping->setChecked( false );

    _checkShowPlanes->setChecked( true );
    connect( _checkShowPlanes, SIGNAL( stateChanged( int ) ), _openGLWidget,
             SLOT( paintClippingPlanes( int ) ) );

    connect( _buttonSelectionFromClippingPlanes, SIGNAL( clicked( void ) ),
             this, SLOT( selectionFromPlanes( void ) ) );

    connect( _buttonResetPlanes, SIGNAL( clicked( void ) ), this,
             SLOT( clippingPlanesReset( void ) ) );

    connect( _spinBoxClippingDist, SIGNAL( editingFinished( void ) ), this,
             SLOT( spinBoxValueChanged( void ) ) );

    connect( _spinBoxClippingHeight, SIGNAL( editingFinished( void ) ), this,
             SLOT( spinBoxValueChanged( void ) ) );

    connect( _spinBoxClippingWidth, SIGNAL( editingFinished( void ) ), this,
             SLOT( spinBoxValueChanged( void ) ) );

    connect( _frameClippingColor, SIGNAL( clicked( ) ), this,
             SLOT( colorSelectionClicked( ) ) );

    connect( _buttonClearSelection, SIGNAL( clicked( void ) ), this,
             SLOT( clearSelection( void ) ) );

    connect( _buttonAddGroup, SIGNAL( clicked( void ) ), this,
             SLOT( addGroupFromSelection( ) ) );

    connect( _buttonImportGroups, SIGNAL( clicked( void ) ), this,
             SLOT( dialogSubsetImporter( void ) ) );

    connect( _buttonClearGroups, SIGNAL( clicked( void ) ), this,
             SLOT( clearGroups( void ) ) );

    _alphaNormalButton->setChecked( true );
  }

  void MainWindow::_initSummaryWidget( void )
  {
    simil::TSimulationType simType =
      _openGLWidget->player( )->simulationType( );

    if ( simType == simil::TSimSpikes )
    {
      simil::SpikesPlayer* spikesPlayer =
        dynamic_cast< simil::SpikesPlayer* >( _openGLWidget->player( ) );

      std::cout << "Creating summary..." << std::endl;

      _summary->Init( spikesPlayer->data( ) );

      _summary->simulationPlayer( _openGLWidget->player( ) );
    }
  }

  void MainWindow::UpdateSimulationSlider( float percentage )
  {
    if ( !_openGLWidget || !_openGLWidget->player( ) )
      return;

    _startTimeLabel->setText(
      QString::number( ( double )_openGLWidget->currentTime( ) ) );
    // TODO UPDATE ENDTIME

    int total = _simSlider->maximum( ) - _simSlider->minimum( );

    int position = percentage * total;

    _simSlider->setSliderPosition( position );

    if ( _summary )
      _summary->repaintHistograms( );
  }

  void MainWindow::UpdateSimulationColorMapping( void )
  {
    _openGLWidget->changeSimulationColorMapping( _tfWidget->getColors( ) );
  }

  void MainWindow::PreviewSimulationColorMapping( void )
  {
    _openGLWidget->changeSimulationColorMapping(
      _tfWidget->getPreviewColors( ) );
  }

  void MainWindow::changeEditorColorMapping( void )
  {
    _tfWidget->setColorPoints( _openGLWidget->getSimulationColorMapping( ) );
  }

  void MainWindow::changeEditorSizeFunction( void )
  {
    _tfWidget->setSizeFunction( _openGLWidget->getSimulationSizeFunction( ) );
  }

  void MainWindow::UpdateSimulationSizeFunction( void )
  {
    _openGLWidget->changeSimulationSizeFunction(
      _tfWidget->getSizeFunction( ) );
  }

  void MainWindow::PreviewSimulationSizeFunction( void )
  {
    _openGLWidget->changeSimulationSizeFunction( _tfWidget->getSizePreview( ) );
  }

  void MainWindow::changeEditorSimDeltaTime( void )
  {
    _deltaTimeBox->setValue( _openGLWidget->simulationDeltaTime( ) );
  }

  void MainWindow::updateSimDeltaTime( void )
  {
    _openGLWidget->simulationDeltaTime( _deltaTimeBox->value( ) );
  }

  void MainWindow::changeEditorSimTimestepsPS( void )
  {
    _timeStepsPSBox->setValue( _openGLWidget->simulationStepsPerSecond( ) );
  }

  void MainWindow::updateSimTimestepsPS( void )
  {
    _openGLWidget->simulationStepsPerSecond( _timeStepsPSBox->value( ) );
  }

  void MainWindow::setCircuitSizeScaleFactor( vec3 scaleFactor )
  {
    _openGLWidget->circuitScaleFactor( scaleFactor );
  }

  vec3 MainWindow::getCircuitSizeScaleFactor( void ) const
  {
    return _openGLWidget->circuitScaleFactor( );
  }

  void MainWindow::changeCircuitScaleValue( void )
  {
    auto scale = _openGLWidget->circuitScaleFactor( );

    _circuitScaleX->blockSignals( true );
    _circuitScaleY->blockSignals( true );
    _circuitScaleZ->blockSignals( true );

    _circuitScaleX->setValue( scale.x );
    _circuitScaleY->setValue( scale.y );
    _circuitScaleZ->setValue( scale.z );

    _circuitScaleX->blockSignals( false );
    _circuitScaleY->blockSignals( false );
    _circuitScaleZ->blockSignals( false );
  }

  void MainWindow::updateCircuitScaleValue( void )
  {
    auto scale = _openGLWidget->circuitScaleFactor( );

    scale.x = _circuitScaleX->value( );
    scale.y = _circuitScaleY->value( );
    scale.z = _circuitScaleZ->value( );

    _openGLWidget->circuitScaleFactor( scale );
  }

  void MainWindow::changeEditorDecayValue( void )
  {
    _decayBox->setValue( _openGLWidget->getSimulationDecayValue( ) );
  }

  void MainWindow::updateSimulationDecayValue( void )
  {
    _openGLWidget->changeSimulationDecayValue( _decayBox->value( ) );
  }

  void MainWindow::changeEditorStepByStepDuration( void )
  {
    _stepByStepDurationBox->setValue(
      _openGLWidget->simulationStepByStepDuration( ) );
  }

  void MainWindow::updateSimStepByStepDuration( void )
  {
    _openGLWidget->simulationStepByStepDuration(
      _stepByStepDurationBox->value( ) );
  }

  void MainWindow::AlphaBlendingToggled( void )
  {
    std::cout << "Changing alpha blending... ";
    if ( _alphaNormalButton->isChecked( ) )
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

  void MainWindow::updateAttributeStats( void )
  {
    if ( !_domainManager )
      return;

    QLayoutItem* item;
    while ( ( item = _layoutAttribStats->takeAt( 0 ) ) )
    {
      _layoutAttribStats->removeWidget( item->widget( ) );
      delete item->widget( );
    }

    std::cout << "Updating stats..." << std::endl;

    auto stats = _domainManager->attributeStatistics( );

    for ( auto stat : stats )
    {
      auto name = std::get< T_TYPE_NAME >( stat );
      auto label = std::get< T_TYPE_LABEL >( stat );
      auto number = std::get< T_TYPE_STATS >( stat );
      QString text =
        ( QString( name.c_str( ) ) + ": " + QString::number( number ) );

      QLabel* textLabel = new QLabel( text );
      _layoutAttribStats->addWidget( textLabel );
    }

    while ( ( item = _layoutAttribGroups->takeAt( 0 ) ) )
    {
      _layoutAttribGroups->removeWidget( item->widget( ) );
      delete item->widget( );
    }

    _attribGroupsVisButtons.clear( );

    unsigned int currentIndex = 0;
    for ( auto group : _openGLWidget->domainManager( )->attributeGroups( ) )
    {
      QFrame* frame = new QFrame( );
      frame->setStyleSheet( "background-color: " + group->color( ).name( ) );
      frame->setMinimumSize( 20, 20 );
      frame->setMaximumSize( 20, 20 );

      group->name( std::get< T_TYPE_NAME >(
        stats[ currentIndex ] ) ); // names[ currentIndex] );

      QCheckBox* buttonVisibility = new QCheckBox( group->name( ).c_str( ) );
      buttonVisibility->setChecked( true );

      _attribGroupsVisButtons.push_back( buttonVisibility );

      connect( buttonVisibility, SIGNAL( clicked( ) ), this,
               SLOT( checkAttributeGroupsVisibility( ) ) );

      _layoutAttribGroups->addWidget( frame, currentIndex, 0, 1, 1 );
      _layoutAttribGroups->addWidget( buttonVisibility, currentIndex, 2, 1, 1 );

      ++currentIndex;
    }

    _layoutAttribGroups->update( );
  }

  void MainWindow::updateSelectedStatsPickingSingle( unsigned int selected )
  {
    auto pickingInfo = _domainManager->pickingInfoSimple( selected );

    _labelGID->setText(
      QString::number( std::get< T_PART_GID >( pickingInfo ) ) );

    auto position = std::get< T_PART_POSITION >( pickingInfo );

    QString posText = "( " + QString::number( position.x ) + ", " +
                      QString::number( position.y ) + ", " +
                      QString::number( position.z ) + " )";

    _labelPosition->setText( posText );

    _toolBoxOptions->setCurrentIndex( ( unsigned int )T_TOOL_Inpector );
  }

  void MainWindow::clippingPlanesReset( void )
  {
    _openGLWidget->clippingPlanesReset( );

    _resetClippingParams( );
  }

  void MainWindow::_resetClippingParams( void )
  {
    _spinBoxClippingDist->setValue( _openGLWidget->clippingPlanesDistance( ) );
    _spinBoxClippingHeight->setValue( _openGLWidget->clippingPlanesHeight( ) );
    _spinBoxClippingWidth->setValue( _openGLWidget->clippingPlanesWidth( ) );
  }

  void MainWindow::clippingPlanesActive( int state )
  {
    bool active = state;

    _checkShowPlanes->setEnabled( active );
    _buttonResetPlanes->setEnabled( active );
    _spinBoxClippingHeight->setEnabled( active );
    _spinBoxClippingWidth->setEnabled( active );
    _spinBoxClippingDist->setEnabled( active );
    _frameClippingColor->setEnabled( active );
    _buttonSelectionFromClippingPlanes->setEnabled( active );

    _openGLWidget->clippingPlanes( active );

    _resetClippingParams( );
  }

  void MainWindow::spinBoxValueChanged( void )
  {
    auto spinBox = dynamic_cast< QDoubleSpinBox* >( sender( ) );

    if ( spinBox == _spinBoxClippingDist )
      _openGLWidget->clippingPlanesDistance( spinBox->value( ) );
    else if ( spinBox == _spinBoxClippingHeight )
      _openGLWidget->clippingPlanesHeight( spinBox->value( ) );
    else if ( spinBox == _spinBoxClippingWidth )
      _openGLWidget->clippingPlanesWidth( spinBox->value( ) );
  }

  void MainWindow::showInactive( bool show )
  {
    _openGLWidget->showInactive( show );
  }

  void MainWindow::playAtButtonClicked( void )
  {
    if ( !_openGLWidget || !_openGLWidget->player( ) )
      return;

    bool ok;
    double result = QInputDialog::getDouble(
      this, tr( "Set simulation time to play:" ), tr( "Simulation time" ),
      ( double )_openGLWidget->currentTime( ),
      ( double )_openGLWidget->player( )->data( )->startTime( ),
      ( double )_openGLWidget->player( )->data( )->endTime( ), 3, &ok,
      Qt::Popup );

    if ( ok )
    {
      float percentage = ( result - _openGLWidget->player( )->startTime( ) ) /
                         ( _openGLWidget->player( )->endTime( ) -
                           _openGLWidget->player( )->startTime( ) );

      percentage = std::max( 0.0f, std::min( 1.0f, percentage ) );

      PlayAt( percentage, true );
    }
  }

  bool MainWindow::_showDialog( QColor& current, const QString& message )
  {
    QColor result = QColorDialog::getColor( current, this, QString( message ),
                                            QColorDialog::DontUseNativeDialog );

    if ( result.isValid( ) )
    {
      current = result;
      return true;
    }
    else
      return false;
  }

  void MainWindow::colorSelectionClicked( void )
  {
    QPushButton* button = dynamic_cast< QPushButton* >( sender( ) );

    if ( button == _frameClippingColor )
    {
      QColor current = _openGLWidget->clippingPlanesColor( );
      if ( _showDialog( current, "Select planes color" ) )
      {
        _openGLWidget->clippingPlanesColor( current );
        button->setStyleSheet( "background-color: " + current.name( ) );
      }
    }
  }

  void MainWindow::selectionFromPlanes( void )
  {
    if ( !_openGLWidget )
      return;

    auto ids = _openGLWidget->getPlanesContainedElements( );
    visimpl::GIDUSet selectedSet( ids.begin( ), ids.end( ) );

    if ( selectedSet.empty( ) )
      return;

    setSelection( selectedSet, SRC_PLANES );
  }

  void MainWindow::selectionManagerChanged( void )
  {
    setSelection( _selectionManager->selected( ), SRC_WIDGET );
  }

  void MainWindow::_updateSelectionGUI( void )
  {
    auto selection = _domainManager->selection( );

    _buttonAddGroup->setEnabled( true );
    _buttonClearSelection->setEnabled( true );
    _selectionSizeLabel->setText( QString::number( selection.size( ) ) );
    _selectionSizeLabel->update( );
  }

  void MainWindow::setSelection( const GIDUSet& selectedSet,
                                 TSelectionSource source_ )
  {
    if ( source_ == SRC_UNDEFINED )
      return;

    _domainManager->selection( selectedSet );
    _openGLWidget->setSelectedGIDs( selectedSet );

    if ( source_ != SRC_WIDGET )
      _selectionManager->setSelected( selectedSet );

    _updateSelectionGUI( );
  }

  void MainWindow::clearSelection( void )
  {
    if ( _openGLWidget )
    {
      _domainManager->clearSelection( );
      _openGLWidget->clearSelection( );
      _selectionManager->clearSelection( );

      _buttonAddGroup->setEnabled( false );
      _buttonClearSelection->setEnabled( false );
      _selectionSizeLabel->setText( "0" );
    }
  }

#ifdef VISIMPL_USE_ZEROEQ

#ifdef VISIMPL_USE_GMRVLEX

  void MainWindow::ApplyPlaybackOperation( unsigned int playbackOp )
  {
    zeroeq::gmrv::PlaybackOperation operation =
      ( zeroeq::gmrv::PlaybackOperation )playbackOp;

    switch ( operation )
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
        PreviousStep( false );
        break;
      case zeroeq::gmrv::END:
        //        std::cout << "Received end" << std::endl;
        NextStep( false );
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
    Repeat( false );
  }

#endif

  void MainWindow::_setZeqUri( const std::string& uri_ )
  {
    _zeqConnection = true;
    _uri = uri_.empty( ) ? zeroeq::DEFAULT_SESSION : uri_;

    _subscriber = new zeroeq::Subscriber( _uri );

    _subscriber->subscribe(
      lexis::data::SelectedIDs::ZEROBUF_TYPE_IDENTIFIER( ),
      [&]( const void* data_, const size_t size_ ) {
        _onSelectionEvent( lexis::data::SelectedIDs::create( data_, size_ ) );
      } );

    _thread = new std::thread( [&]( ) {
      while ( true )
        _subscriber->receive( 10000 );
    } );
  }

  // void* MainWindow::_Subscriber( void* subs )
  //{
  //  zeroeq::Subscriber* subscriber = static_cast< zeroeq::Subscriber* >( subs
  //  ); while ( true )
  //  {
  //    subscriber->receive( 10000 );
  //  }
  //  pthread_exit( NULL );
  //}

  void
    MainWindow::_onSelectionEvent( lexis::data::ConstSelectedIDsPtr selected )
  {
    std::cout << "Received selection" << std::endl;
    if ( _openGLWidget )
    {
      //    std::vector< unsigned int > selected =
      //        zeq::hbp::deserializeSelectedIDs( selected );

      std::vector< uint32_t > ids = selected->getIdsVector( );

      visimpl::GIDUSet selectedSet( ids.begin( ), ids.end( ) );

      setSelection( selectedSet, SRC_EXTERNAL );
    }
  }

#endif

  void MainWindow::addGroupControls( const std::string& name,
                                     unsigned int currentIndex,
                                     unsigned int size )
  {
    QFrame* frame = new QFrame( );
    auto colors = _openGLWidget->colorPalette( ).colors( );

    frame->setStyleSheet( "background-color: " +
                          colors[ currentIndex ].name( ) );
    frame->setMinimumSize( 20, 20 );
    frame->setMaximumSize( 20, 20 );

    QCheckBox* buttonVisibility = new QCheckBox( "active" );
    buttonVisibility->setChecked( true );

    connect( buttonVisibility, SIGNAL( clicked( ) ), this,
             SLOT( checkGroupsVisibility( ) ) );

    QWidget* container = new QWidget( );
    QGridLayout* layout = new QGridLayout( );
    container->setLayout( layout );

    QString numberText = QString( "# " ).append( QString::number( size ) );

    layout->addWidget( frame, 0, 0, 1, 1 );
    layout->addWidget( new QLabel( name.c_str( ) ), 0, 1, 1, 1 );
    layout->addWidget( buttonVisibility, 0, 2, 1, 1 );
    layout->addWidget( new QLabel( numberText ), 0, 3, 1, 1 );

    _groupsVisButtons.push_back(
      std::make_tuple( container, buttonVisibility ) );

    _groupLayout->addWidget( container );

    _buttonClearGroups->setEnabled( true );
  }

  void MainWindow::removeGroupControls( unsigned int )
  {
  }

  void MainWindow::clearGroups( void )
  {
    for ( auto row : _groupsVisButtons )
    {
      auto container = std::get< gr_container >( row );

      _groupLayout->removeWidget( container );

      delete container;

      _domainManager->removeVisualGroup( 0 );
    }

    _groupsVisButtons.clear( );

    _buttonImportGroups->setEnabled( true );
    _buttonClearGroups->setEnabled( false );
  }

  void MainWindow::importVisualGroups( void )
  {
    const auto& groups = _subsetImporter->selectedSubsets( );

    std::cout << "Importing " << groups.size( ) << " groups..." << std::endl;

    const auto& allGIDs = _domainManager->gids( );

    for ( auto groupName : groups )
    {
      auto subset = _subsetEvents->getSubset( groupName );
      TGIDUSet gids( subset.begin( ), subset.end( ) );

      GIDUSet filteredGIDs;
      for ( auto gid : gids )
        if ( allGIDs.find( gid ) != allGIDs.end( ) )
          filteredGIDs.insert( gid );

      addGroupControls( groupName, _domainManager->groups( ).size( ),
                        filteredGIDs.size( ) );
      _domainManager->addVisualGroup( filteredGIDs, groupName );
    }

    _openGLWidget->setUpdateGroups( );

    _buttonImportGroups->setEnabled( false );
  }

  void MainWindow::addGroupFromSelection( void )
  {
    unsigned int currentIndex = _domainManager->groups( ).size( );

    QString groupName( "Group " + QString::number( currentIndex ) );

    if ( !_autoNameGroups )
    {
      bool ok;
      groupName = QInputDialog::getText( this, tr( "Group Name" ),
                                         tr( "Please, introduce group name: " ),
                                         QLineEdit::Normal, groupName, &ok );

      if ( !ok )
        return;
    }

    addGroupControls( groupName.toStdString( ),
                      _domainManager->groups( ).size( ),
                      _domainManager->selection( ).size( ) );

    _domainManager->addVisualGroupFromSelection( groupName.toStdString( ) );

    _openGLWidget->setUpdateGroups( );
    _openGLWidget->update( );
  }

  void MainWindow::checkGroupsVisibility( void )
  {
    unsigned int counter = 0;
    auto group = _openGLWidget->domainManager( )->groups( ).begin( );
    for ( auto button : _groupsVisButtons )
    {
      auto checkBox = std::get< gr_checkbox >( button );

      if ( checkBox->isChecked( ) != ( *group )->active( ) )
      {
        _domainManager->setVisualGroupState( counter, checkBox->isChecked( ) );
      }

      ++group;
      ++counter;
    }

    _openGLWidget->setUpdateGroups( );
    _openGLWidget->update( );
  }

  void MainWindow::checkAttributeGroupsVisibility( void )
  {
    unsigned int counter = 0;
    auto group = _openGLWidget->domainManager( )->attributeGroups( ).begin( );
    for ( auto button : _attribGroupsVisButtons )
    {
      if ( button->isChecked( ) != ( *group )->active( ) )
      {
        _openGLWidget->domainManager( )->setVisualGroupState(
          counter, button->isChecked( ), true );
      }
      ++group;
      ++counter;
    }

    _openGLWidget->update( );
  }

  void MainWindow::PlayPause( bool notify )
  {
    if ( !_openGLWidget || !_openGLWidget->player( ) )
      return;

    if ( !_openGLWidget->player( )->isPlaying( ) )
      Play( notify );
    else
      Pause( notify );
  }

  void MainWindow::Play( bool notify )
  {
    if ( !_openGLWidget || !_openGLWidget->player( ) )
      return;

    if ( _openGLWidget )
    {
      _openGLWidget->Play( );

      _playButton->setIcon( _pauseIcon );

      if ( _openGLWidget->playbackMode( ) == TPlaybackMode::STEP_BY_STEP &&
           _openGLWidget->completedStep( ) )
        _openGLWidget->playbackMode( TPlaybackMode::CONTINUOUS );

      if ( notify )
      {
#ifdef SIMIL_USE_ZEROEQ
        _openGLWidget->player( )->zeqEvents( )->sendPlaybackOp(
          zeroeq::gmrv::PLAY );
#endif
      }
    }
  }

  void MainWindow::Pause( bool notify )
  {
    if ( !_openGLWidget || !_openGLWidget->player( ) )
      return;

    if ( _openGLWidget )
    {
      _openGLWidget->Pause( );
      _playButton->setIcon( _playIcon );

      if ( notify )
      {
#ifdef SIMIL_USE_ZEROEQ
        _openGLWidget->player( )->zeqEvents( )->sendPlaybackOp(
          zeroeq::gmrv::PAUSE );
#endif
      }
    }
  }

  void MainWindow::Stop( bool notify )
  {
    if ( !_openGLWidget || !_openGLWidget->player( ) )
      return;

    if ( _openGLWidget )
    {
      _openGLWidget->Stop( );
      _playButton->setIcon( _playIcon );
      _startTimeLabel->setText(
        QString::number( ( double )_openGLWidget->player( )->startTime( ) ) );

      _openGLWidget->playbackMode( TPlaybackMode::CONTINUOUS );

      if ( notify )
      {
#ifdef SIMIL_USE_ZEROEQ
        _openGLWidget->player( )->zeqEvents( )->sendPlaybackOp(
          zeroeq::gmrv::STOP );
#endif
      }
    }
  }

  void MainWindow::Repeat( bool notify )
  {
    if ( !_openGLWidget || !_openGLWidget->player( ) )
      return;

    if ( _openGLWidget )
    {
      bool repeat = _repeatButton->isChecked( );
      //    std::cout << "Repeat " << boolalpha << repeat << std::endl;
      _openGLWidget->Repeat( repeat );

      if ( notify )
      {
#ifdef SIMIL_USE_ZEROEQ
        _openGLWidget->player( )->zeqEvents( )->sendPlaybackOp(
          repeat ? zeroeq::gmrv::ENABLE_LOOP : zeroeq::gmrv::DISABLE_LOOP );
#endif
      }
    }
  }

  void MainWindow::requestPlayAt( float percentage )
  {
    if ( !_openGLWidget || !_openGLWidget->player( ) )
      return;

    PlayAt( percentage, false );
  }

  void MainWindow::PlayAt( bool notify )
  {
    if ( !_openGLWidget || !_openGLWidget->player( ) )
      return;

    if ( _openGLWidget )
    {
      PlayAt( _simSlider->sliderPosition( ), notify );
    }
  }

  void MainWindow::PlayAt( float percentage, bool notify )
  {
    if ( !_openGLWidget || !_openGLWidget->player( ) )
      return;

    if ( _openGLWidget )
    {
      int sliderPosition =
        percentage * ( _simSlider->maximum( ) - _simSlider->minimum( ) ) +
        _simSlider->minimum( );

      _simSlider->setSliderPosition( sliderPosition );

      _playButton->setIcon( _pauseIcon );

      _openGLWidget->PlayAt( percentage );

      _openGLWidget->playbackMode( TPlaybackMode::CONTINUOUS );

      if ( notify )
      {
#ifdef SIMIL_USE_ZEROEQ
        // Send event
        _openGLWidget->player( )->zeqEvents( )->sendFrame(
          _simSlider->minimum( ), _simSlider->maximum( ), sliderPosition );

        _openGLWidget->player( )->zeqEvents( )->sendPlaybackOp(
          zeroeq::gmrv::PLAY );
#endif
      }
    }
  }

  void MainWindow::PlayAt( int sliderPosition, bool notify )
  {
    if ( !_openGLWidget || !_openGLWidget->player( ) )
      return;

    if ( _openGLWidget )
    {
      int value = _simSlider->value( );
      float percentage =
        float( value - _simSlider->minimum( ) ) /
        float( _simSlider->maximum( ) - _simSlider->minimum( ) );

      _simSlider->setSliderPosition( sliderPosition );

      _playButton->setIcon( _pauseIcon );

      _openGLWidget->PlayAt( percentage );

      _openGLWidget->playbackMode( TPlaybackMode::CONTINUOUS );

      if ( notify )
      {
#ifdef SIMIL_USE_ZEROEQ
        // Send event
        _openGLWidget->player( )->zeqEvents( )->sendFrame(
          _simSlider->minimum( ), _simSlider->maximum( ), sliderPosition );

        _openGLWidget->player( )->zeqEvents( )->sendPlaybackOp(
          zeroeq::gmrv::PLAY );
#endif
      }
    }
  }

  void MainWindow::PreviousStep( bool /*notify*/ )
  {
    if ( !_openGLWidget || !_openGLWidget->player( ) )
      return;

    if ( _openGLWidget )
    {
      _playButton->setIcon( _pauseIcon );
      _openGLWidget->player( )->Play( );
      _openGLWidget->PreviousStep( );
    }
  }

  void MainWindow::NextStep( bool /*notify*/ )
  {
    if ( !_openGLWidget || !_openGLWidget->player( ) )
      return;

    if ( _openGLWidget )
    {
      _playButton->setIcon( _pauseIcon );
      _openGLWidget->player( )->Play( );
      _openGLWidget->NextStep( );
    }
  }

  void MainWindow::completedStep( void )
  {
    if ( _openGLWidget )
    {
      _playButton->setIcon( _playIcon );
    }
  }

} // namespace visimpl
