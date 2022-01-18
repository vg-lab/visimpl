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
#include <visimpl/version.h>

#include "MainWindow.h"

#include <QFileDialog>
#include <QInputDialog>
#include <QGridLayout>
#include <QShortcut>
#include <QMessageBox>
#include <QColorDialog>
#include <QDateTime>
#include <QComboBox>
#include <QDockWidget>
#include <QPushButton>
#include <QSlider>
#include <QTimer>
#include <QRadioButton>
#include <QGroupBox>
#include <QPushButton>
#include <QToolBox>
#include <QtGlobal>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

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
    , _playIcon(":/icons/play.svg")
    , _pauseIcon(":/icons/pause.svg")
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
    , _objectInspectorGB{nullptr}
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
    , _buttonLoadGroups{nullptr}
    , _buttonSaveGroups{nullptr}
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
    , m_type{simil::TDataType::TDataUndefined}
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
    _openGLWidget = new OpenGLWidget( this, Qt::WindowFlags(), zeqUri );
    connect(_openGLWidget, SIGNAL(dataLoaded()), this, SLOT(onDataLoaded()));

    this->setCentralWidget( _openGLWidget );

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

    connect( _ui->actionOpenH5Files, SIGNAL( triggered( void ) ), this,
             SLOT( openHDF5ThroughDialog( void ) ) );

    connect( _ui->actionOpenSubsetEventsFile, SIGNAL( triggered( void ) ), this,
             SLOT( openSubsetEventsFileThroughDialog( void ) ) );

    connect( _ui->actionCloseData, SIGNAL( triggered( void ) ), this,
             SLOT( closeData( void ) ) );

    connect( _ui->actionQuit, SIGNAL( triggered( void ) ), this,
             SLOT( close( ) ) );

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

    _ui->toolBar->setContextMenuPolicy(Qt::PreventContextMenu);
    _ui->menubar->setContextMenuPolicy(Qt::PreventContextMenu);

#ifdef VISIMPL_USE_ZEROEQ

    _setZeqUri( zeqUri );

#endif
  }

  MainWindow::~MainWindow( void )
  {
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
    }

#endif

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

    if(_openGLWidget)
    {
      auto player = _openGLWidget->player();
      if(player)
      {
        const auto tBegin = player->startTime();
        const auto tEnd = player->endTime();
        const auto tCurrent = player->currentTime();

        _startTimeLabel->setText(QString::number(tCurrent, 'f', 3));
        _endTimeLabel->setText(QString::number(tEnd, 'f', 3));

        const auto percentage = (tCurrent - tBegin) / (tEnd - tBegin);
        UpdateSimulationSlider(percentage);
      }
    }
  }

  void MainWindow::openBlueConfigThroughDialog( void )
  {
#ifdef SIMIL_USE_BRION

    QString path = QFileDialog::getOpenFileName(
      this, tr( "Open BlueConfig" ), _lastOpenedNetworkFileName,
      tr( "BlueConfig ( BlueConfig CircuitConfig);; All files (*)" ), nullptr,
      QFileDialog::DontUseNativeDialog );

    if ( !path.isEmpty() )
    {
      bool ok;

      QString text = QInputDialog::getText( this, tr( "Please type a target" ),
                                            tr( "Target name:" ),
                                            QLineEdit::Normal, "Mosaic", &ok );

      if ( ok && !text.isEmpty( ) )
      {
        std::string reportLabel = text.toStdString( );
        _lastOpenedNetworkFileName = QFileInfo( path ).path( );
        std::string fileName = path.toStdString( );

        loadData(simil::TBlueConfig, fileName, reportLabel, simil::TSimSpikes);
      }
    }
#else
    const QString title = tr("Open BlueConfig");
    const QString message = tr("BlueConfig loading is not supported in this version.");
    QMessageBox::critical(this, title, message, QMessageBox::Button::Ok);
#endif
  }

  void MainWindow::openCSVFilesThroughDialog( void )
  {
    QString pathNetwork = QFileDialog::getOpenFileName(
      this, tr( "Open CSV Network description file" ),
      _lastOpenedNetworkFileName, tr( "CSV (*.csv);; All files (*)" ), nullptr,
      QFileDialog::DontUseNativeDialog );

    if ( !pathNetwork.isEmpty() )
    {
      QString pathActivity = QFileDialog::getOpenFileName(
        this, tr( "Open CSV Activity file" ), _lastOpenedNetworkFileName,
        tr( "CSV (*.csv);; All files (*)" ), nullptr,
        QFileDialog::DontUseNativeDialog );

      if ( !pathActivity.isEmpty( ) )
      {
        _lastOpenedNetworkFileName = QFileInfo( pathNetwork ).path( );
        _lastOpenedActivityFileName = QFileInfo( pathActivity ).path( );
        std::string networkFile = pathNetwork.toStdString( );
        std::string activityFile = pathActivity.toStdString( );

        loadData(simil::TCSV, networkFile, activityFile, simil::TSimSpikes);
      }
    }
  }

  void MainWindow::openHDF5ThroughDialog( void )
  {
    auto path = QFileDialog::getOpenFileName(
      this, tr( "Open a H5 network file" ), _lastOpenedNetworkFileName,
      tr( "hdf5 ( *.h5);; All files (*)" ), nullptr,
      QFileDialog::DontUseNativeDialog );

    if ( path.isEmpty() ) return;

    const auto networkFile = path.toStdString( );

    path =
      QFileDialog::getOpenFileName( this, tr( "Open a H5 activity file" ), path,
                                    tr( "hdf5 ( *.h5);; All files (*)" ),
                                    nullptr, QFileDialog::DontUseNativeDialog );

    if ( path.isEmpty() ) return;

    const auto activityFile = path.toStdString( );

    loadData(simil::THDF5, networkFile, activityFile, simil::TSimSpikes);
  }

  void MainWindow::openSubsetEventFile( const std::string& filePath,
                                        bool append )
  {
    if ( filePath.empty( ) || !_subsetEvents )
      return;

    if ( !append )
      _subsetEvents->clear( );

    QFileInfo eventsFile{QString::fromStdString(filePath)};
    if(!eventsFile.exists())
      return;

    QString errorText;
    try
    {
      if(eventsFile.suffix().toLower().compare("json") == 0)
      {
        _subsetEvents->loadJSON( filePath );
      }
      else if(eventsFile.suffix().toLower().compare("h5") == 0)
      {
        _subsetEvents->loadH5( filePath );
      }
      else
      {
        errorText = tr("Events file not supported: %1").arg(eventsFile.absoluteFilePath());
      }
    }
    catch(const std::exception &e)
    {
      errorText = QString::fromLocal8Bit(e.what());
    }

    if(!errorText.isEmpty())
    {
      QMessageBox::warning(this, tr("Error loading Events file"), errorText, QMessageBox::Ok);
      return;
    }

    _subsetImporter->reload(_subsetEvents);

    _openGLWidget->subsetEventsManager(_subsetEvents);
    _openGLWidget->showEventsActivityLabels(_ui->actionShowEventsActivity->isChecked());
  }

  void MainWindow::openSubsetEventsFileThroughDialog( void )
  {
    QString filePath = QFileDialog::getOpenFileName(
      this, tr( "Open file containing subsets/events data" ),
      _lastOpenedSubsetsFileName, tr( "JSON (*.json);; hdf5 ( *.h5);; All files (*)" ),
      nullptr, QFileDialog::DontUseNativeDialog );

    if ( !filePath.isEmpty( ) )
    {
      QFileInfo eventsFile{ filePath };
      if(eventsFile.exists())
      {
        _lastOpenedSubsetsFileName = eventsFile.path();

        openSubsetEventFile( filePath.toStdString( ), false );
      }
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
      "VG-Lab / URJC / UPM"
      "<br><a href='https://vg-lab.es/'>https://vg-lab.es/</a>"
      "<br>(C) 2015-" + QString::number(QDateTime::currentDateTime().date().year()) + "<br><br>"
      "<a href='https://vg-lab.es'><img src=':/icons/logoVGLab.png'/></a>"
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
    if(QDialog::Accepted == _subsetImporter->exec())
    {
      importVisualGroups();
    }
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

    _objectInspectorGB->setSimPlayer(_openGLWidget->player( ));
    _subsetEvents = _openGLWidget->subsetEventsManager( );

    _startTimeLabel->setText(
      QString::number(_openGLWidget->player()->startTime(), 'f', 3));

    _endTimeLabel->setText(
      QString::number(_openGLWidget->player()->endTime(), 'f', 3));

    _simSlider->setEnabled(true);

#ifdef SIMIL_USE_ZEROEQ
    try
    {
      const auto eventMgr = _openGLWidget->player( )->zeqEvents( );
      if(eventMgr)
      {
        eventMgr->playbackOpReceived.connect( boost::bind( &MainWindow::ApplyPlaybackOperation, this, _1 ) );
        eventMgr->frameReceived.connect( boost::bind( &MainWindow::requestPlayAt, this, _1 ) );
      }
    }
    catch(std::exception &e)
    {
      std::cerr << "Exception when initializing player events. ";
      std::cerr << e.what() << " " << __FILE__ << ":" << __LINE__ << std::endl;
    }
    catch(...)
    {
      std::cerr << "Unknown exception when initializing player events. " << __FILE__ << ":" << __LINE__ << std::endl;
    }
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

    auto content = new QWidget( );
    auto dockLayout = new QGridLayout( );
    content->setLayout( dockLayout );

    _simSlider = new CustomSlider( Qt::Horizontal );
    _simSlider->setMinimum( 0 );
    _simSlider->setMaximum( 1000 );
    _simSlider->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );
    _simSlider->setEnabled(false);

    _playButton = new QPushButton(_playIcon, tr(""));
    _playButton->setSizePolicy( QSizePolicy::MinimumExpanding,
                                QSizePolicy::MinimumExpanding );
    auto stopButton = new QPushButton(QIcon(":/icons/stop.svg"), tr(""));
    auto nextButton = new QPushButton(QIcon(":/icons/next.svg"), tr(""));
    auto prevButton = new QPushButton(QIcon(":/icons/previous.svg"), tr(""));

    _repeatButton = new QPushButton(QIcon(":/icons/repeat.svg"), tr(""));
    _repeatButton->setCheckable( true );
    _repeatButton->setChecked( false );

    _goToButton = new QPushButton( tr("Play at...") );

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

    connect( _simSlider, SIGNAL( sliderPressed( ) ), this, SLOT( PlayAtPosition( ) ) );

    connect( _goToButton, SIGNAL( clicked( ) ), this, SLOT( playAtButtonClicked( ) ) );

    _summary = new visimpl::Summary( nullptr, visimpl::T_STACK_FIXED );
    _summary->setMinimumHeight( 50 );

    dockLayout->addWidget( _summary, 0, 1, 2, totalHSpan - 3 );

    _simulationDock->setWidget( content );
    this->addDockWidget(Qt::BottomDockWidgetArea, _simulationDock );

    connect( _summary, SIGNAL( histogramClicked( float ) ), this,
             SLOT( PlayAtPercentage( float ) ) );
  }

  void MainWindow::_initSimControlDock( void )
  {
    _simConfigurationDock = new QDockWidget( );
    _simConfigurationDock->setMinimumHeight( 100 );
    _simConfigurationDock->setMinimumWidth( 400 );
    _simConfigurationDock->setSizePolicy( QSizePolicy::MinimumExpanding,
                                          QSizePolicy::MinimumExpanding );

    _tfWidget = new TransferFunctionWidget( );
    _tfWidget->setMinimumHeight( 100 );
    _tfWidget->setDialogIcon(QIcon(":/visimpl.png"));

    _subsetImporter = new SubsetImporter( this );
    _subsetImporter->setWindowModality( Qt::WindowModal );
    _subsetImporter->setMinimumHeight( 300 );
    _subsetImporter->setMinimumWidth( 500 );

    _selectionManager = new SelectionManagerWidget( );
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

    _objectInspectorGB = new DataInspector( "Object inspector" );
    _objectInspectorGB->addWidget( new QLabel( "GID:" ), 2, 0, 1, 1 );
    _objectInspectorGB->addWidget( _labelGID, 2, 1, 1, 3 );
    _objectInspectorGB->addWidget( new QLabel( "Position: " ), 3, 0, 1, 1 );
    _objectInspectorGB->addWidget( _labelPosition, 3, 1, 1, 3 );

    QGroupBox* groupBoxGroups = new QGroupBox( "Current visualization groups" );
    _groupLayout = new QVBoxLayout( );
    _groupLayout->setAlignment( Qt::AlignTop );

    _buttonImportGroups = new QPushButton( "Import from..." );
    _buttonClearGroups = new QPushButton( "Clear" );
    _buttonClearGroups->setEnabled( false );
    _buttonLoadGroups = new QPushButton("Load");
    _buttonLoadGroups->setToolTip(tr("Load Groups from disk"));
    _buttonSaveGroups = new QPushButton("Save");
    _buttonSaveGroups->setToolTip(tr("Save Groups to disk"));
    _buttonSaveGroups->setEnabled(false);

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
      groupOuterLayout->addWidget( _buttonLoadGroups, 1, 0, 1, 1 );
      groupOuterLayout->addWidget( _buttonSaveGroups, 1, 1, 1, 1 );

      groupOuterLayout->addWidget( scrollGroups, 2, 0, 1, 2 );

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


    _toolBoxOptions->addItem( _objectInspectorGB, tr( "Inspector" ));

    connect( _objectInspectorGB, SIGNAL( simDataChanged()),
             _openGLWidget,      SLOT( updateData()));

    connect( _objectInspectorGB, SIGNAL( simDataChanged()),
             _summary,           SLOT( UpdateHistograms()));

    connect (_objectInspectorGB, SIGNAL( simDataChanged()),
             this,               SLOT(configureComponents()));

    verticalLayout->setAlignment( Qt::AlignTop );
    verticalLayout->addWidget( _modeSelectionWidget );
    verticalLayout->addWidget( _toolBoxOptions );

    topContainer->setLayout( verticalLayout );
    _simConfigurationDock->setWidget( topContainer );

    this->addDockWidget(Qt::RightDockWidgetArea, _simConfigurationDock );

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

    connect( _buttonLoadGroups, SIGNAL( clicked(void)), this, SLOT(loadGroups()));
    connect( _buttonSaveGroups, SIGNAL( clicked(void)), this, SLOT(saveGroups()));

    _alphaNormalButton->setChecked( true );
  }

  void MainWindow::_initSummaryWidget( void )
  {
    const auto simType = _openGLWidget->player( )->simulationType( );

    if ( simType == simil::TSimSpikes )
    {
      simil::SpikesPlayer* spikesPlayer =
        dynamic_cast< simil::SpikesPlayer* >( _openGLWidget->player( ) );

      _summary->Init( spikesPlayer->data( ) );

      _summary->simulationPlayer( _openGLWidget->player( ) );
    }
  }

  void MainWindow::UpdateSimulationSlider( float percentage )
  {
    if ( !_openGLWidget || !_openGLWidget->player( ) )
      return;

    _startTimeLabel->setText(QString::number(_openGLWidget->currentTime(), 'f', 3));
    _endTimeLabel->setText(QString::number(_openGLWidget->player()->endTime(), 'f', 3));

    const int total = _simSlider->maximum( ) - _simSlider->minimum( );
    const int position = (percentage * total) + _simSlider->minimum();

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
    if ( _alphaNormalButton->isChecked( ) )
    {
      _openGLWidget->SetAlphaBlendingAccumulative( false );
    }
    else
    {
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

    const auto stats = _domainManager->attributeStatistics( );

    auto insertAttribStats = [this](const tStatsGroup &stat)
    {
      const auto name = std::get< T_TYPE_NAME >( stat );
      const auto number = std::get< T_TYPE_STATS >( stat );
      const auto text = ( QString( name.c_str( ) ) + ": " + QString::number( number ) );

      QLabel* textLabel = new QLabel( text );
      _layoutAttribStats->addWidget( textLabel );
    };
    std::for_each(stats.cbegin(), stats.cend(), insertAttribStats);

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

      PlayAtPercentage( percentage, true );
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
        Play( false );
        break;
      case zeroeq::gmrv::PAUSE:
        Pause( false );
        break;
      case zeroeq::gmrv::STOP:
        Stop( false );
        break;
      case zeroeq::gmrv::BEGIN:
        PreviousStep( false );
        break;
      case zeroeq::gmrv::END:
        NextStep( false );
        break;
      case zeroeq::gmrv::ENABLE_LOOP:
        _zeqEventRepeat( true );
        break;
      case zeroeq::gmrv::DISABLE_LOOP:
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
    if(!uri_.empty())
    {
      bool failed = false;
      try
      {
        _zeqUri = uri_;
        _zeqConnection = true;
        _subscriber = new zeroeq::Subscriber( _zeqUri );

        _subscriber->subscribe(lexis::data::SelectedIDs::ZEROBUF_TYPE_IDENTIFIER( ),
          [&]( const void* data_, unsigned long long size_ )
          { _onSelectionEvent( lexis::data::SelectedIDs::create( data_, size_ ));});

        _thread = new std::thread( [this]( )
        { while ( _zeqConnection ) _subscriber->receive( 10000 ); } );
      }
      catch(std::exception &e)
      {
        std::cerr << "Exception when initializing ZeroEQ. ";
        std::cerr << e.what() << " " << __FILE__ << ":" << __LINE__ << std::endl;
        failed = true;
      }
      catch(...)
      {
        std::cerr << "Unknown exception when initializing ZeroEQ. " << __FILE__ << ":" << __LINE__ << std::endl;
        failed = true;
      }

      if(failed)
      {
        _zeqUri.clear();
        _zeqConnection = false;
        _subscriber = nullptr;
        _thread = nullptr;
      }
    }
  }

  void MainWindow::_onSelectionEvent( lexis::data::ConstSelectedIDsPtr selected )
  {
    if ( _openGLWidget )
    {
      std::vector< uint32_t > ids = selected->getIdsVector( );

      visimpl::GIDUSet selectedSet( ids.begin( ), ids.end( ) );

      setSelection( selectedSet, SRC_EXTERNAL );
    }
  }

#endif

  void MainWindow::addGroupControls( VisualGroup *group,
                                     unsigned int currentIndex,
                                     unsigned int size )
  {
    QWidget* container = new QWidget( );
    auto itemLayout = new QHBoxLayout(container);
    container->setLayout( itemLayout );

    const auto colors = _openGLWidget->colorPalette( ).colors( );
    currentIndex = currentIndex % colors.size();
    auto color = colors[currentIndex].toRgb();
    const auto variations = DomainManager::generateColorPair(color);

    TTransferFunction colorVariation;
    colorVariation.push_back( std::make_pair( 0.0f, variations.first ));
    colorVariation.push_back( std::make_pair( 1.0f, variations.second ));
    group->colorMapping(colorVariation);

    auto tfWidget = new TransferFunctionWidget(container);
    tfWidget->setColorPoints(group->colorMapping());
    tfWidget->setSizeFunction(group->sizeFunction());
    tfWidget->setDialogIcon(QIcon(":/visimpl.png"));
    tfWidget->setProperty("groupNum", static_cast<unsigned int>(currentIndex));

    itemLayout->addWidget(tfWidget);

    const auto presetName = QString("Group selection %1").arg(currentIndex);
    QGradientStops stops;
    stops << qMakePair( 0.0,  variations.first)
          << qMakePair( 1.0,  variations.second);
    tfWidget->addPreset(TransferFunctionWidget::Preset(presetName, stops));

    connect( tfWidget, SIGNAL(colorChanged()),
             this,     SLOT(onGroupColorChanged()));
    connect( tfWidget, SIGNAL(previewColor()),
             this,     SLOT(onGroupPreview()));

    QCheckBox* visibilityCheckbox = new QCheckBox( "active" );
    visibilityCheckbox->setChecked( group->active() );

    connect( visibilityCheckbox, SIGNAL( clicked( ) ), this,
             SLOT( checkGroupsVisibility( ) ) );

    QString numberText = QString( "# " ).append( QString::number( size ) );

    auto nameButton = new QPushButton(group->name().c_str());
    nameButton->setFlat(true);
    nameButton->setProperty("groupNum", static_cast<unsigned int>(currentIndex));

    connect(nameButton, SIGNAL(clicked()), this, SLOT(onGroupNameClicked()));

    auto layout = new QVBoxLayout();
    layout->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    layout->addWidget( nameButton );
    layout->addWidget( visibilityCheckbox );
    layout->addWidget( new QLabel( numberText ) );

    itemLayout->insertLayout(1, layout, 0);
    itemLayout->setSizeConstraint(QLayout::SizeConstraint::SetMinimumSize);

    _groupsVisButtons.push_back(
      std::make_tuple( container, visibilityCheckbox ) );

    _groupLayout->addWidget( container );

    _buttonClearGroups->setEnabled( true );
    _buttonSaveGroups->setEnabled(true);
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
        QString::number(_openGLWidget->player()->startTime(),'f',3));

      _openGLWidget->playbackMode( TPlaybackMode::CONTINUOUS );

      if ( notify )
      {
#ifdef VISIMPL_USE_GMRVLEX
        sendZeroEQPlaybackOperation( zeroeq::gmrv::STOP );
#endif
      }
    }
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
    _buttonSaveGroups->setEnabled(false);
  }

  void MainWindow::importVisualGroups( void )
  {
    const auto& groups = _subsetImporter->selectedSubsets( );

    const auto& allGIDs = _domainManager->gids( );

    auto addGroup = [this, &allGIDs](const std::string &groupName)
    {
      const auto subset = _subsetEvents->getSubset( groupName );

      GIDUSet filteredGIDs;
      auto filterGIDs = [&filteredGIDs, &allGIDs](const uint32_t gid)
      {
        if ( allGIDs.find( gid ) != allGIDs.end( ) )
          filteredGIDs.insert( gid );
      };
      std::for_each(subset.cbegin(), subset.cend(), filterGIDs);

      const auto group = _domainManager->addVisualGroup( filteredGIDs, groupName );

      addGroupControls( group, _domainManager->groups( ).size( ) - 1,
                        filteredGIDs.size( ) );
    };
    std::for_each(groups.cbegin(), groups.cend(), addGroup);

    _openGLWidget->setUpdateGroups( );
    _openGLWidget->subsetEventsManager(_subsetEvents);
    _openGLWidget->showEventsActivityLabels(_ui->actionShowEventsActivity->isChecked());

    _buttonImportGroups->setEnabled( groups.empty() );
    _buttonClearGroups->setEnabled( !groups.empty() );
  }

  void MainWindow::addGroupFromSelection( void )
  {
    const unsigned int currentIndex = _domainManager->groups( ).size( );

    QString groupName = QString( "Group " + QString::number( currentIndex ) );
    if ( !_autoNameGroups )
    {
      bool ok;
      groupName = QInputDialog::getText( this, tr( "Group Name" ),
                                         tr( "Please, introduce group name: " ),
                                         QLineEdit::Normal, groupName, &ok );

      if ( !ok ) return;
    }

    const auto group = _domainManager->addVisualGroupFromSelection( groupName.toStdString( ) );

    addGroupControls( group,
                      _domainManager->groups( ).size( ) - 1,
                      _domainManager->selection( ).size( ) );

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
#ifdef VISIMPL_USE_GMRVLEX
        sendZeroEQPlaybackOperation( zeroeq::gmrv::PLAY );
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
      _startTimeLabel->setText(
          QString::number(_openGLWidget->player()->currentTime(), 'f',3));

      if ( notify )
      {
#ifdef VISIMPL_USE_GMRVLEX
        sendZeroEQPlaybackOperation( zeroeq::gmrv::PAUSE );
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
      _openGLWidget->Repeat( repeat );

      if ( notify )
      {
#ifdef VISIMPL_USE_GMRVLEX
        const auto op = repeat ? zeroeq::gmrv::ENABLE_LOOP : zeroeq::gmrv::DISABLE_LOOP;
        sendZeroEQPlaybackOperation( op );
#endif
      }
    }
  }

  void MainWindow::requestPlayAt( float timePos )
  {
    if ( !_openGLWidget || !_openGLWidget->player( ) )
      return;

    const auto tBegin = _openGLWidget->player()->startTime();
    const auto tEnd   = _openGLWidget->player()->endTime();
    const auto newPosition = std::min(tEnd, std::max(tBegin, timePos));
    const auto isOverflow = timePos != newPosition;

    PlayAtTime( newPosition, isOverflow );

    if(isOverflow)
      Pause(true);
  }

  void MainWindow::PlayAtPosition( bool notify )
  {
    if ( !_openGLWidget || !_openGLWidget->player( ) )
      return;

    PlayAtPosition( _simSlider->sliderPosition( ), notify );
  }

  void MainWindow::PlayAtPosition( int sliderPosition, bool notify )
  {
    if ( !_openGLWidget || !_openGLWidget->player( ) )
      return;

    const int sMin = _simSlider->minimum();
    const int sMax = _simSlider->maximum();
    const float percentage = static_cast<float>(sliderPosition - sMin) * (sMax - sMin);

    PlayAtPercentage(percentage, notify);
  }

  void MainWindow::PlayAtPercentage( float percentage, bool notify )
  {
    if ( !_openGLWidget || !_openGLWidget->player( ) )
      return;

    const auto tBegin = _openGLWidget->player()->startTime();
    const auto tEnd = _openGLWidget->player()->endTime();
    const auto timePos = (percentage * (tEnd - tBegin)) + tBegin;

    PlayAtTime(timePos, notify);
  }

  void MainWindow::PlayAtTime(float timePos, bool notify)
  {
    if(!_openGLWidget || !_openGLWidget->player())
      return;

    const auto tBegin = _openGLWidget->player()->startTime();
    const auto tEnd = _openGLWidget->player()->endTime();
    const auto newPosition = std::max(tBegin, std::min(tEnd, timePos));
    const auto percentage = (newPosition - tBegin) / (tEnd - tBegin);

    const auto sMin = _simSlider->minimum();
    const auto sMax = _simSlider->maximum();
    const int sliderPosition = (percentage * (sMax - sMin)) + sMin;

    _simSlider->setSliderPosition( sliderPosition );

    _playButton->setIcon( _pauseIcon );

    _openGLWidget->PlayAt( newPosition );
    _openGLWidget->playbackMode( TPlaybackMode::CONTINUOUS );
    _startTimeLabel->setText(
        QString::number(_openGLWidget->player()->currentTime(), 'f',3));

    if ( notify )
    {
#ifdef SIMIL_USE_ZEROEQ
      try
      {
        // Send event
        auto player   = _openGLWidget->player();
        auto eventMgr = player->zeqEvents( );
        if(eventMgr)
        {
          eventMgr->sendFrame( player->startTime(), player->endTime(), player->currentTime());
        }
      }
      catch(const std::exception &e)
      {
        std::cerr << "Exception when sending frame. " << e.what() << __FILE__ << ":" << __LINE__ << std::endl;
      }
      catch(...)
      {
        std::cerr << "Unknown exception when sending frame. " << __FILE__ << ":" << __LINE__  << std::endl;
      }
#endif
#ifdef VISIMPL_USE_GMRVLEX
      sendZeroEQPlaybackOperation(zeroeq::gmrv::PLAY);
#endif
    }

  }
  void MainWindow::PreviousStep( bool /*notify*/ )
  {
    if ( !_openGLWidget || !_openGLWidget->player( ) )
      return;

    _playButton->setIcon( _pauseIcon );
    _openGLWidget->player( )->Play( );
    _openGLWidget->PreviousStep( );
  }

  void MainWindow::NextStep( bool /*notify*/ )
  {
    if ( !_openGLWidget || !_openGLWidget->player( ) )
      return;

    _playButton->setIcon( _pauseIcon );
    _openGLWidget->player( )->Play( );
    _openGLWidget->NextStep( );
  }

  void MainWindow::completedStep( void )
  {
    if ( _openGLWidget )
    {
      _playButton->setIcon( _playIcon );
    }
  }

  void MainWindow::onGroupColorChanged()
  {
    auto tfw = qobject_cast<TransferFunctionWidget *>(sender());
    if(tfw)
    {
      bool ok = false;
      size_t groupNum = tfw->property("groupNum").toUInt(&ok);

      if(!ok) return;
      updateGroupColors(groupNum, tfw->getColors(), tfw->getSizeFunction());
    }
  }

  void MainWindow::onGroupPreview()
  {
    auto tfw = qobject_cast<TransferFunctionWidget *>(sender());
    if(tfw)
    {
      bool ok = false;
      size_t groupNum = tfw->property("groupNum").toUInt(&ok);

      if(!ok) return;
      updateGroupColors(groupNum, tfw->getPreviewColors(), tfw->getSizePreview());
    }
  }

  void MainWindow::updateGroupColors(size_t idx, const TTransferFunction &t,
      const TSizeFunction &s)
  {
    const auto groups = _domainManager->groups();
    const auto groupNum = std::min(idx, groups.size() - 1);
    groups[groupNum]->colorMapping(t);
    groups[groupNum]->sizeFunction(s);
  }

  void MainWindow::loadData(const simil::TDataType type,
                            const std::string arg_1, const std::string arg_2,
                            const simil::TSimulationType simType, const std::string &subsetEventFile)
  {
    QApplication::setOverrideCursor(Qt::WaitCursor);

    try
    {
      m_type = type;
      m_subsetEventFile = subsetEventFile;
      _openGLWidget->loadData( arg_1, type, simType, arg_2 );
      _lastOpenedNetworkFileName = QString::fromStdString(arg_1);
    }
    catch(const std::exception &e)
    {
      QApplication::restoreOverrideCursor();
      const auto errorText = QString::fromLocal8Bit(e.what());
      QMessageBox::critical(this, tr("Error loading data"), errorText, QMessageBox::Ok);
      return;
    }
  }

  void MainWindow::onDataLoaded()
  {
    configureComponents( );

    openSubsetEventFile( m_subsetEventFile, false );

    _configurePlayer( );

    _comboAttribSelection->clear();

    switch(m_type)
    {
      case simil::TDataType::TBlueConfig:
        {
          const QStringList attributes = {"Morphological type", "Functional type"};
          _comboAttribSelection->addItems( attributes );
        }
        break;
      case simil::TDataType::TREST:
        {
#ifdef SIMIL_WITH_REST_API
          auto timer = new QTimer( this );
          connect( timer,              SIGNAL( timeout()),
                   _objectInspectorGB, SLOT( updateInfo()) );

          timer->start( 4000 );
#endif
        }
        break;
      case simil::TDataType::TCONE:
      case simil::TDataType::TCSV:
      case simil::TDataType::THDF5:
      default:
        break;
    }

    _openGLWidget->closeLoadingDialog();

    QApplication::restoreOverrideCursor();
  }

  void MainWindow::onGroupNameClicked()
  {
    auto button = qobject_cast<QPushButton *>(sender());
    if(button)
    {
      bool ok = false;
      size_t groupNum = button->property("groupNum").toUInt(&ok);

      if(!ok) return;
      auto group = _domainManager->groups().at(groupNum);

      auto groupName = QString::fromStdString(group->name());
      groupName = QInputDialog::getText( this, tr( "Group Name" ),
                                         tr( "Please, introduce group name: " ),
                                         QLineEdit::Normal, groupName, &ok );

      if (!ok) return;
      group->name(groupName.toStdString());
      button->setText(groupName);
    }
  }

  void MainWindow::loadGroups()
  {
    const auto title = tr("Load Groups");

    if(!_domainManager->groups().empty())
    {
      const auto message = tr("Loading groups from disk will erase the "
                              "current groups. Do you want to continue?");

      QMessageBox msgbox(this);
      msgbox.setWindowTitle(title);
      msgbox.setText(message);
      msgbox.setIcon(QMessageBox::Icon::Question);
      msgbox.setWindowIcon(QIcon(":/visimpl.png"));
      msgbox.setStandardButtons(QMessageBox::Cancel|QMessageBox::Ok);
      msgbox.setDefaultButton(QMessageBox::Button::Ok);

      if(QMessageBox::Ok != msgbox.exec())
        return;
    }

    QFileInfo lastFile{_lastOpenedNetworkFileName};
    const auto fileName = QFileDialog::getOpenFileName(this, title, lastFile.path(),
                                                       tr("Json files (*.json)"),
                                                       nullptr, QFileDialog::ReadOnly|QFileDialog::DontUseNativeDialog);
    if(fileName.isEmpty()) return;

    QFile file{fileName};
    if(!file.open(QIODevice::ReadOnly))
    {
      const auto message = tr("Couldn't open file %1").arg(fileName);

      QMessageBox msgbox(this);
      msgbox.setWindowTitle(title);
      msgbox.setIcon(QMessageBox::Icon::Critical);
      msgbox.setText(message);
      msgbox.setWindowIcon(QIcon(":/visimpl.png"));
      msgbox.setStandardButtons(QMessageBox::Ok);
      msgbox.exec();
      return;
    }

    const auto contents = file.readAll();
    QJsonParseError jsonError;
    const auto  jsonDoc = QJsonDocument::fromJson(contents, &jsonError);;
    if(jsonDoc.isNull() || !jsonDoc.isObject())
    {
      const auto message = tr("Couldn't read the contents of %1 or error at parsing.").arg(fileName);

      QMessageBox msgbox{this};
      msgbox.setWindowTitle(title);
      msgbox.setIcon(QMessageBox::Icon::Critical);
      msgbox.setText(message);
      msgbox.setWindowIcon(QIcon(":/visimpl.png"));
      msgbox.setStandardButtons(QMessageBox::Ok);
      msgbox.setDetailedText(jsonError.errorString());
      msgbox.exec();
      return;
    }

    const auto jsonObj = jsonDoc.object();
    if(jsonObj.isEmpty())
    {
      const auto message = tr("Error at parsing.").arg(fileName);

      QMessageBox msgbox{this};
      msgbox.setWindowTitle(title);
      msgbox.setIcon(QMessageBox::Icon::Critical);
      msgbox.setText(message);
      msgbox.setWindowIcon(QIcon(":/visimpl.png"));
      msgbox.setStandardButtons(QMessageBox::Ok);
      msgbox.exec();
      return;
    }

    const QFileInfo currentFile{_lastOpenedNetworkFileName};
    const QString jsonGroupsFile = jsonObj.value("filename").toString();
    if(jsonGroupsFile.compare(currentFile.fileName(), Qt::CaseInsensitive) != 0)
    {
      const auto message = tr("This groups definitions are from file %1. Current file"
                              " is %2. Do you want to continue?").arg(jsonGroupsFile).arg(currentFile.fileName());

      QMessageBox msgbox{this};
      msgbox.setWindowTitle(title);
      msgbox.setIcon(QMessageBox::Icon::Question);
      msgbox.setText(message);
      msgbox.setWindowIcon(QIcon(":/visimpl.png"));
      msgbox.setStandardButtons(QMessageBox::Cancel|QMessageBox::Ok);
      msgbox.setDefaultButton(QMessageBox::Ok);

      if(QMessageBox::Ok != msgbox.exec())
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    clearGroups();
    const auto jsonGroups = jsonObj.value("groups").toArray();

    std::vector<VisualGroup *> groupsList;
    const auto createGroup = [&groupsList, this](const QJsonValue &v)
    {
      const auto o = v.toObject();

      const auto name = o.value("name").toString();
      const auto overrideGIDS = o.value("override").toBool(false);
      const auto gidsStrings = o.value("gids").toString().split(",");

      GIDUSet gids;
      auto addGids = [&gids](const QString s)
      {
        if(s.contains(":"))
        {
         auto limits = s.split(":");
         for(unsigned int id = limits.first().toUInt(); id <= limits.last().toUInt(); ++id)
           gids.insert(id);
        }
        else
        {
          gids.insert(s.toUInt());
        }
      };
      std::for_each(gidsStrings.cbegin(), gidsStrings.cend(), addGids);

      auto group = _domainManager->addVisualGroup(gids, name.toStdString(), overrideGIDS);
      auto idx = _domainManager->groups().size()-1;
      addGroupControls(group, idx, gids.size());
      const auto active = o.value("active").toBool(true);
      auto checkbox = std::get< gr_checkbox >(_groupsVisButtons.at(idx));
      checkbox->setChecked(active);

      _domainManager->setVisualGroupState( idx, active );

      const auto functionPairs = o.value("function").toString().split(";");
      TTransferFunction function;
      auto addFunctionPair = [&function](const QString &s)
      {
        const auto parts = s.split(",");
        Q_ASSERT(parts.size() == 2);
        const auto value = parts.first().toFloat();
        const auto color = QColor(parts.last());
        function.emplace_back(value, color);
      };
      std::for_each(functionPairs.cbegin(), functionPairs.cend(), addFunctionPair);

      const auto sizePairs = o.value("sizes").toString().split(";");
      TSizeFunction sizes;
      auto addSizes = [&sizes](const QString &s)
      {
        const auto parts = s.split(",");
        Q_ASSERT(parts.size() == 2);
        const auto a = parts.first().toFloat();
        const auto b = parts.last().toFloat();
        sizes.emplace_back(a, b);
      };
      std::for_each(sizePairs.cbegin(), sizePairs.cend(), addSizes);

      updateGroupColors(idx, function, sizes);
      auto container = std::get< gr_container >(_groupsVisButtons.at(idx));
      auto tfw = qobject_cast<TransferFunctionWidget*>(container->layout()->itemAt(0)->widget());
      if(tfw)
      {
        tfw->setColorPoints(function, true);
        tfw->setSizeFunction(sizes);
        tfw->colorChanged();
        tfw->sizeChanged();
      }
    };
    std::for_each(jsonGroups.constBegin(), jsonGroups.constEnd(), createGroup);

    _groupLayout->update();
    checkGroupsVisibility();

    const auto groups = _domainManager->groups();
    _buttonClearGroups->setEnabled( !groups.empty() );
    _buttonSaveGroups->setEnabled( !groups.empty() );

    QApplication::restoreOverrideCursor();
  }

  void MainWindow::saveGroups()
  {
    const auto &groups = _domainManager->groups();
    if(groups.empty()) return;

    const auto dateTime = QDateTime::currentDateTime();
    QFileInfo lastFile{_lastOpenedNetworkFileName};
    QString filename = lastFile.dir().absoluteFilePath(lastFile.baseName() + "_groups_" + dateTime.toString("yyyy-MM-dd-hh-mm") + ".json");
    filename = QFileDialog::getSaveFileName(this, tr("Save Groups"), filename, tr("Json files (*.json)"), nullptr, QFileDialog::DontUseNativeDialog);

    if(filename.isEmpty()) return;

    QFile wFile{filename};
    if(!wFile.open(QIODevice::WriteOnly|QIODevice::Text|QIODevice::Truncate))
    {
      const auto message = tr("Unable to open file %1 for writing.").arg(filename);

      QMessageBox msgbox{this};
      msgbox.setWindowTitle(tr("Save Groups"));
      msgbox.setIcon(QMessageBox::Icon::Critical);
      msgbox.setText(message);
      msgbox.setWindowIcon(QIcon(":/visimpl.png"));
      msgbox.setDefaultButton(QMessageBox::Ok);
      msgbox.exec();
      return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    QJsonObject obj;
    obj.insert("filename", QFileInfo{_lastOpenedNetworkFileName}.fileName());
    obj.insert("date", dateTime.toString());

    QJsonArray groupsObjs;

    auto insertGroup = [&groupsObjs, this](const VisualGroup *g)
    {
      QJsonObject groupObj;
      groupObj.insert("name", QString::fromStdString(g->name()));
      groupObj.insert("active", g->active());

      QStringList tfList;
      const auto tf = g->colorMapping();
      auto addColors = [&tfList](const TTFColor &c)
      {
        tfList << QString("%1,%2").arg(c.first).arg(c.second.name(QColor::HexArgb));
      };
      std::for_each(tf.cbegin(), tf.cend(), addColors);
      groupObj.insert("function", tfList.join(";"));

      QStringList sizesList;
      const auto sizes = g->sizeFunction();
      auto addSizes = [&sizesList](const TSize &s)
      {
        sizesList << QString("%1,%2").arg(s.first).arg(s.second);
      };
      std::for_each(sizes.cbegin(), sizes.cend(), addSizes);
      groupObj.insert("sizes", sizesList.join(";"));

      const auto &gids = g->gids();
      std::vector<unsigned int> gidsVec;
      std::for_each(gids.cbegin(), gids.cend(), [&gidsVec](unsigned int v){ gidsVec.push_back(v); });
      std::sort(gidsVec.begin(), gidsVec.end());
      QStringList gidsStrings;
      std::pair<unsigned int, unsigned int> range = std::make_pair(std::numeric_limits<unsigned int>::max() - 1,std::numeric_limits<unsigned int>::max() - 1);
      auto enterNumber = [&range, &gidsStrings]()
      {
          if(range.first == range.second)
            gidsStrings << QString::number(range.first);
          else
            gidsStrings << QString("%1:%2").arg(range.first).arg(range.second);
      };

      for(auto i = gidsVec.begin(); i != gidsVec.end(); ++i)
      {
        auto num = *i;
        if(num != range.second + 1)
        {
          if(range.first != std::numeric_limits<unsigned int>::max() - 1)
          {
            enterNumber();
          }
          range.first = num;
        }

        range.second = num;
      }
      enterNumber();

      groupObj.insert("gids", gidsStrings.join(","));

      groupsObjs << groupObj;
    };
    std::for_each(groups.cbegin(), groups.cend(), insertGroup);

    obj.insert("groups", groupsObjs);

    QJsonDocument doc{obj};
    const auto temp = doc.toJson().toStdString();
    wFile.write(doc.toJson());

    QApplication::restoreOverrideCursor();

    if(wFile.error() != QFile::NoError)
    {
      const auto message = tr("Error saving file %1.").arg(filename);

      QMessageBox msgbox{this};
      msgbox.setWindowTitle(tr("Save Groups"));
      msgbox.setIcon(QMessageBox::Icon::Critical);
      msgbox.setText(message);
      msgbox.setDetailedText(wFile.errorString());
      msgbox.setWindowIcon(QIcon(":/visimpl.png"));
      msgbox.setDefaultButton(QMessageBox::Ok);
      msgbox.exec();
    }

    wFile.flush();
    wFile.close();
  }

  void MainWindow::sendZeroEQPlaybackOperation(const unsigned int op)
  {
#ifdef SIMIL_USE_ZEROEQ
    try
    {
      auto eventMgr = _openGLWidget->player( )->zeqEvents( );
      if(eventMgr)
      {
        eventMgr->sendPlaybackOp( static_cast<zeroeq::gmrv::PlaybackOperation>(op) );
      }
    }
    catch(const std::exception &e)
    {
      std::cerr << "Exception when sending play operation. " << e.what() << __FILE__ << ":" << __LINE__ << std::endl;
    }
    catch(...)
    {
      std::cerr << "Unknown exception when sending play operation. " << __FILE__ << ":" << __LINE__  << std::endl;
    }
#else
    __attribute__((unused)) const auto unused = op; // c++17 [[maybe_unused]]
#endif
  }

} // namespace visimpl
