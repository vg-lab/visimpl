/*
 * @file  MainWindow.cpp
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
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

  MainWindow::MainWindow( QWidget* parent_,
                          bool updateOnIdle )
  : QMainWindow( parent_ )
  #ifdef VISIMPL_USE_GMRVLEX
  , _zeqConnection( false )
  , _subscriber( nullptr )
  , _thread( nullptr )
  #endif
  , _ui( new Ui::MainWindow )
  , _lastOpenedFileName( "" )
  , _openGLWidget( nullptr )
  , _domainManager( nullptr )
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
  , _tfEditor( nullptr )
  , _tfWidget( nullptr )
  , _autoNameGroups( false )
  , _groupBoxGroups( nullptr )
  , _groupLayout( nullptr )
  , _decayBox( nullptr )
  , _deltaTimeBox( nullptr )
  , _timeStepsPSBox( nullptr )
  , _stepByStepDurationBox( nullptr )
  , _addGroupButton( nullptr )
  , _clearSelectionButton( nullptr )
  , _selectionSizeLabel( nullptr )
  , _alphaNormalButton( nullptr )
  , _alphaAccumulativeButton( nullptr )
  , _labelGID( nullptr )
  , _labelPosition( nullptr )
  , _groupBoxAttrib( nullptr )
  , _comboAttribSelection( nullptr )
  , _layoutAttribStats( nullptr )
  , _layoutAttribGroups( nullptr )
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

    _openGLWidget->idleUpdate( _ui->actionUpdateOnIdle->isChecked( ));

    connect( _ui->actionUpdateOnIdle, SIGNAL( triggered( )),
             _openGLWidget, SLOT( toggleUpdateOnIdle( )));

    connect( _ui->actionBackgroundColor, SIGNAL( triggered( )),
             _openGLWidget, SLOT( changeClearColor( )));

    connect( _ui->actionShowFPSOnIdleUpdate, SIGNAL( triggered( )),
             _openGLWidget, SLOT( toggleShowFPS( )));

    connect( _ui->actionShowEventsActivity, SIGNAL( triggered( bool )),
             _openGLWidget, SLOT( showEventsActivityLabels( bool )));

    connect( _ui->actionShowCurrentTime, SIGNAL( triggered( bool )),
             _openGLWidget, SLOT( showCurrentTimeLabel( bool )));

    connect( _ui->actionOpenBlueConfig, SIGNAL( triggered( )),
             this, SLOT( openBlueConfigThroughDialog( )));

    connect( _ui->actionQuit, SIGNAL( triggered( )),
             QApplication::instance(), SLOT( quit( )));

    // Connect about dialog
    connect( _ui->actionAbout, SIGNAL( triggered( )),
             this, SLOT( aboutDialog( )));


    connect( _ui->actionHome, SIGNAL( triggered( )),
             _openGLWidget, SLOT( home( )));

    connect( _openGLWidget, SIGNAL( stepCompleted( void )),
             this, SLOT( completedStep( void )));

    connect( _openGLWidget, SIGNAL( pickedSingle( unsigned int )),
             this, SLOT( updateSelectedStatsPickingSingle( unsigned int )));

    QAction* actionTogglePause = new QAction(this);
    actionTogglePause->setShortcut( Qt::Key_Space );

    connect( actionTogglePause, SIGNAL( triggered( )), this, SLOT( PlayPause( )));
    addAction( actionTogglePause );


  #ifdef VISIMPL_USE_ZEROEQ
    _ui->actionShowInactive->setEnabled( true );
    _ui->actionShowInactive->setChecked( true );

    _openGLWidget->showInactive( true );

    connect( _ui->actionShowInactive, SIGNAL( toggled( bool )),
             this, SLOT( showInactive( bool )));

  #else
    _ui->actionShowSelection->setEnabled( false );
  #endif

    _initPlaybackDock( );
    _initSimControlDock( );

    connect( _simulationDock->toggleViewAction( ), SIGNAL( toggled( bool )),
               _ui->actionTogglePlaybackDock, SLOT( setChecked( bool )));

    connect( _ui->actionTogglePlaybackDock, SIGNAL( triggered( )),
             this, SLOT( togglePlaybackDock( )));

    connect( _simConfigurationDock->toggleViewAction( ), SIGNAL( toggled( bool )),
               _ui->actionToggleSimConfigDock, SLOT( setChecked( bool )));

    connect( _ui->actionToggleSimConfigDock, SIGNAL( triggered( )),
             this, SLOT( toggleSimConfigDock( )));



    #ifdef VISIMPL_USE_ZEROEQ

    _setZeqUri( zeqUri );

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
                                   simil::TSimulationType simulationType,
                                   const std::string& reportLabel,
                                   const std::string& subsetEventFile )
  {
    _openGLWidget->loadData( fileName,
                             simil::TDataType::TBlueConfig,
                             simulationType, reportLabel );

    _domainManager = _openGLWidget->domainManager( );

    openSubsetEventFile( subsetEventFile, true );

    _configurePlayer( );


    QStringList attributes = { "Morphological type", "Functional type" };

    _comboAttribSelection->addItems( attributes );
  }

  void MainWindow::openBlueConfigThroughDialog( void )
  {
  #ifdef SIMIL_USE_BRION

    QString path = QFileDialog::getOpenFileName(
      this, tr( "Open BlueConfig" ), _lastOpenedFileName,
      tr( "BlueConfig ( BlueConfig CircuitConfig);; All files (*)" ),
      nullptr, QFileDialog::DontUseNativeDialog );

    if( path != QString( "" ))
    {

      bool ok;

      simil::TSimulationType simType = simil::TSimSpikes;
      QString text = QInputDialog::getText(
            this, tr( "Please type a target" ),
            tr( "Target name:" ), QLineEdit::Normal,
            "Mosaic", &ok );

      if ( ok && !text.isEmpty( ))
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
                                 const std::string& activityFile,
                                 const std::string& subsetEventFile )
  {
    _openGLWidget->loadData( networkFile,
                             simil::TDataType::THDF5,
                             simulationType,
                             activityFile );

    openSubsetEventFile( subsetEventFile, true );

    _configurePlayer( );
  }


  void MainWindow::openHDF5ThroughDialog( void )
  {
    QString path = QFileDialog::getOpenFileName(
      this, tr( "Open a H5 network file" ), _lastOpenedFileName,
      tr( "hdf5 ( *.h5);; All files (*)" ), nullptr,
      QFileDialog::DontUseNativeDialog );

    std::string networkFile;
    std::string activityFile;

    if ( path == QString( "" ))
      return;

    networkFile = path.toStdString( );

    path = QFileDialog::getOpenFileName(
    this, tr( "Open a H5 activity file" ), path,
    tr( "hdf5 ( *.h5);; All files (*)" ), nullptr,
    QFileDialog::DontUseNativeDialog );

    if( path == QString( "" ))
    {
      return;
    }

    activityFile = path.toStdString( );

    openHDF5File( networkFile, simil::TSimSpikes, activityFile );
  }

  void MainWindow::openSubsetEventFile( const std::string& filePath,
                                        bool append )
  {
    if( filePath.empty( ))
      return;

    if( !append )
      _openGLWidget->player( )->data( )->subsetsEvents( )->clear( );


    if( filePath.find( "json" ) != std::string::npos )
    {
      std::cout << "Loading JSON file: " << filePath << std::endl;
      _openGLWidget->player( )->data( )->subsetsEvents( )->loadJSON( filePath );
    }
    else if( filePath.find( "h5" ) != std::string::npos )
    {
      std::cout << "Loading H5 file: " << filePath << std::endl;
      _openGLWidget->player( )->data( )->subsetsEvents( )->loadH5( filePath );
    }
    else
    {
      std::cout << "Subset Events file not found: " << filePath << std::endl;
    }
  }


  void MainWindow::aboutDialog( void )
  {

    QString msj = 
      QString( "<h2>ViSimpl</h2>" ) +
      tr( "A multi-view visual analyzer of brain simulation data. " ) + 
      "<br>" + 
      tr( "Version " ) + visimpl::Version::getString( ).c_str( ) +
      tr( " rev (%1)<br>").arg(visimpl::Version::getRevision( )) +
      "<a href='https://gmrv.es/visimpl/'>https://gmrv.es/visimpl</a>" + 
      "<h4>" + tr( "Build info:" ) + "</h4>" +
      "<ul>"
    
#ifdef VISIMPL_USE_DEFLECT
    "</li><li>Deflect " + DEFLECT_REV_STRING +
#else
    "</li><li>Deflect " + tr ("support not built.") +
#endif

#ifdef VISIMPL_USE_FIRES
    "</li><li>FiReS " + FIRES_REV_STRING +
#else
    "</li><li>FiReS " + tr ("support not built.") +
#endif

#ifdef VISIMPL_USE_GMRVLEX
    "</li><li>GmrvLex " + GMRVLEX_REV_STRING +
#else
    "</li><li>GmrvLex " + tr ("support not built.") +
#endif

#ifdef VISIMPL_USE_NSOL
    "</li><li>Nsol " + NSOL_REV_STRING +
#else
    "</li><li>Nsol " + tr ("support not built.") +
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
#else
    "</li><li>SimIL " + tr ("support not built.") +
#endif

#ifdef VISIMPL_USE_ZEROEQ
    "</li><li>ZeroEQ " + ZEROEQ_REV_STRING +
#else
    "</li><li>ZeroEQ " + tr ("support not built.") +
#endif

    "</li></ul>" +
    "<h4>" + tr( "Developed by:" ) + "</h4>" +
    "GMRV / URJC / UPM"
    "<br><a href='https://gmrv.es/gmrvvis'>https://gmrv.es/gmrvvis</a>"
    //"<br><a href='mailto:gmrv@gmrv.es'>gmrv@gmrv.es</a><br><br>"
    "<br>(C) 2015-2017<br><br>"
    "<a href='https://gmrv.es/gmrvvis'><img src=':/icons/logoGMRV.png'/></a>"
    "&nbsp;&nbsp;&nbsp;&nbsp;"
    "<a href='https://www.urjc.es'><img src=':/icons/logoURJC.png' /></a>"
    "&nbsp;&nbsp;&nbsp;&nbsp;"
    "<a href='https://www.upm.es'><img src=':/icons/logoUPM.png' /></a>";
    
    QMessageBox::about(this, tr( "About ViSimpl" ), msj );
  }

  void MainWindow::togglePlaybackDock( void )
  {
    if( _ui->actionTogglePlaybackDock->isChecked( ))
      _simulationDock->show( );
    else
      _simulationDock->close( );

    update( );
  }

  void MainWindow::toggleSimConfigDock( void )
  {
    if( _ui->actionToggleSimConfigDock->isChecked( ))
      _simConfigurationDock->show( );
    else
      _simConfigurationDock->close( );

    update( );
  }

  void MainWindow::_configurePlayer( void )
  {
    connect( _openGLWidget, SIGNAL( updateSlider( float )),
               this, SLOT( UpdateSimulationSlider( float )));


    _startTimeLabel->setText(
        QString::number( (double)_openGLWidget->player( )->startTime( )));

    _endTimeLabel->setText(
          QString::number( (double)_openGLWidget->player( )->endTime( )));

  #ifdef SIMIL_USE_ZEROEQ
    _openGLWidget->player( )->zeqEvents( )->playbackOpReceived.connect(
        boost::bind( &MainWindow::ApplyPlaybackOperation, this, _1 ));

    _openGLWidget->player( )->zeqEvents( )->frameReceived.connect(
        boost::bind( &MainWindow::requestPlayAt, this, _1 ));

  #endif

    changeEditorColorMapping( );
    changeEditorSimDeltaTime( );
    changeEditorSimTimestepsPS( );
    changeEditorSizeFunction( );
    changeEditorDecayValue( );
    changeEditorStepByStepDuration( );
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
    _simSlider->setSizePolicy( QSizePolicy::Preferred,
                               QSizePolicy::Preferred );

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
    _goToButton->setText( QString( "Play at..." ));

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
    dockLayout->addWidget( _goToButton, row, 13, 1, 1 );

    connect( _playButton, SIGNAL( clicked( )),
             this, SLOT( PlayPause( )));

    connect( stopButton, SIGNAL( clicked( )),
               this, SLOT( Stop( )));

    connect( nextButton, SIGNAL( clicked( )),
               this, SLOT( NextStep( )));

    connect( prevButton, SIGNAL( clicked( )),
               this, SLOT( PreviousStep( )));

    connect( _repeatButton, SIGNAL( clicked( )),
               this, SLOT( Repeat( )));

    connect( _simSlider, SIGNAL( sliderPressed( )),
             this, SLOT( PlayAt( )));

    connect( _goToButton, SIGNAL( clicked( )),
             this, SLOT( playAtButtonClicked( )));

  //  connect( _simSlider, SIGNAL( sliderMoved( )),
  //             this, SLOT( PlayAt( )));

    _summary = new visimpl::Summary( nullptr, visimpl::T_STACK_FIXED );
  //  _summary->setVisible( false );
    _summary->setMinimumHeight( 50 );

    dockLayout->addWidget( _summary, 0, 1, 2, totalHSpan - 3 );

    _simulationDock->setWidget( content );
    this->addDockWidget( Qt::/*DockWidgetAreas::enum_type::*/BottomDockWidgetArea,
                         _simulationDock );

    connect( _summary, SIGNAL( histogramClicked( float )),
             this, SLOT( PlayAt( float )));
  }

  void MainWindow::_initSimControlDock( void )
  {
    _simConfigurationDock = new QDockWidget( );
    _simConfigurationDock->setMinimumHeight( 100 );
    _simConfigurationDock->setMinimumWidth( 300 );
  //  _simConfigurationDock->setMaximumHeight( 400 );
    _simConfigurationDock->setSizePolicy( QSizePolicy::MinimumExpanding,
                                    QSizePolicy::MinimumExpanding );

  //  _tfEditor = new TransferFunctionEditor( );
    _tfWidget = new TransferFunctionWidget( );
    _tfWidget->setMinimumHeight( 150 );

  //  _psWidget = new ParticleSizeWidget( );
  //  _psWidget->setMinimumHeight( 150 );

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

    _clearSelectionButton = new QPushButton( "Discard" );
    _clearSelectionButton->setEnabled( false );
    _selectionSizeLabel = new QLabel( "0" );

    _addGroupButton = new QPushButton( "Add group" );
    _addGroupButton->setEnabled( false );
    _addGroupButton->setToolTip( "Click to create a group from current selection.");

    _labelGID = new QLabel( "" );
    _labelPosition = new QLabel( "" );

    QWidget* topContainer = new QWidget( );
    QVBoxLayout* verticalLayout = new QVBoxLayout( );
  //  QPushButton* applyColorButton = new QPushButton( QString( "Apply" ));

    _groupBoxTransferFunction = new QGroupBox( "Color and Size transfer function" );
    QVBoxLayout* tfLayout = new QVBoxLayout( );
    tfLayout->addWidget( _tfWidget );
    _groupBoxTransferFunction->setLayout( tfLayout );
//    _groupBoxTransferFunction->setMaximumHeight( 250 );

    QGroupBox* tSpeedGB = new QGroupBox( "Simulation playback Configuration" );
    QGridLayout* sfLayout = new QGridLayout( );
    sfLayout->setAlignment( Qt::AlignTop );
    sfLayout->addWidget( new QLabel( "Simulation timestep:" ), 0, 0, 1, 1 );
    sfLayout->addWidget( _deltaTimeBox, 0, 1, 1, 1  );
    sfLayout->addWidget( new QLabel( "Timesteps per second:" ), 1, 0, 1, 1 );
    sfLayout->addWidget( _timeStepsPSBox, 1, 1, 1, 1  );
    sfLayout->addWidget( new QLabel( "Step playback duration (s):"), 2, 0, 1, 1);
    sfLayout->addWidget( _stepByStepDurationBox, 2, 1, 1, 1 );
    tSpeedGB->setLayout( sfLayout );
//    tSpeedGB->setMaximumHeight( 200 );

    QComboBox* comboShader = new QComboBox( );
    comboShader->addItems( { "Default", "Solid" });
    comboShader->setCurrentIndex( 0 );

    QGroupBox* shaderGB = new QGroupBox( "Shader Configuration" );
    QHBoxLayout* shaderLayout = new QHBoxLayout( );
    shaderLayout->addWidget( new QLabel( "Current shader: "));
    shaderLayout->addWidget( comboShader );
    shaderGB->setLayout( shaderLayout );

    QGroupBox* dFunctionGB = new QGroupBox( "Decay function" );
    QHBoxLayout* dfLayout = new QHBoxLayout( );
    dfLayout->setAlignment( Qt::AlignTop );
    dfLayout->addWidget( new QLabel( "Decay (simulation time): " ));
    dfLayout->addWidget( _decayBox );
    dFunctionGB->setLayout( dfLayout );
//    dFunctionGB->setMaximumHeight( 200 );

    QGroupBox* rFunctionGB = new QGroupBox( "Alpha blending function" );
    QHBoxLayout* rfLayout = new QHBoxLayout( );
    rfLayout->setAlignment( Qt::AlignTop );
    rfLayout->addWidget( new QLabel( "Alpha Blending: " ));
    rfLayout->addWidget( _alphaNormalButton );
    rfLayout->addWidget( _alphaAccumulativeButton );
    rFunctionGB->setLayout( rfLayout );
//    rFunctionGB->setMaximumHeight( 200 );

    // Visual Configuration Container
    QWidget* vcContainer = new QWidget( );
    QVBoxLayout* vcLayout = new QVBoxLayout( );
    vcLayout->setAlignment( Qt::AlignTop );
    vcLayout->addWidget( shaderGB );
    vcLayout->addWidget( dFunctionGB );
    vcLayout->addWidget( rFunctionGB );
    vcContainer->setLayout( vcLayout );


    QGroupBox* selFunctionGB = new QGroupBox( "Current selection");
    QHBoxLayout* selLayout = new QHBoxLayout( );
    selLayout->setAlignment( Qt::AlignTop );
    selLayout->addWidget( new QLabel( "Selection size: " ));
    selLayout->addWidget( _selectionSizeLabel );
    selLayout->addWidget( _addGroupButton );
    selLayout->addWidget( _clearSelectionButton );
    selFunctionGB->setLayout( selLayout );
//    selFunctionGB->setMaximumHeight( 200 );

    QGroupBox* objectInspectoGB = new QGroupBox( "Object inspector" );
    QGridLayout* oiLayout = new QGridLayout( );
    oiLayout->setAlignment( Qt::AlignTop );
    oiLayout->addWidget( new QLabel( "GID:" ), 0, 0, 1, 1 );
    oiLayout->addWidget( _labelGID, 0, 1, 1, 3 );
    oiLayout->addWidget( new QLabel( "Position: " ), 1, 0, 1, 1 );
    oiLayout->addWidget( _labelPosition, 1, 1, 1, 3 );
    objectInspectoGB->setLayout( oiLayout );

    _groupBoxGroups = new QGroupBox( "Current visualization groups" );
//    _groupBoxGroups->setMaximumHeight( 200 );
    _groupLayout = new QGridLayout( );
    _groupLayout->setAlignment( Qt::AlignTop );


    {
      QWidget* groupContainer = new QWidget( );
      groupContainer->setLayout( _groupLayout );

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

      _groupBoxGroups->setLayout( groupOuterLayout );
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
    tabGroupsLayout->addWidget( _groupBoxGroups );

    QWidget* containerTabAttrib = new QWidget( );
    QVBoxLayout* tabAttribLayout = new QVBoxLayout( );
    containerTabAttrib->setLayout( tabAttribLayout );
    tabAttribLayout->addWidget( _groupBoxAttrib );

//TODO
    _modeSelectionWidget = new QTabWidget( );
    _modeSelectionWidget->addTab( containerTabSelection, tr( "Selection" ));
    _modeSelectionWidget->addTab( containerTabGroups, tr( "Groups" ));
    _modeSelectionWidget->addTab( containerTabAttrib, tr( "Attribute" ));

    _toolBoxOptions = new QToolBox( );
    _toolBoxOptions->addItem( tSpeedGB, tr( "Playback Configuration" ));
    _toolBoxOptions->addItem( vcContainer, tr( "Visual Configuration" ));
    _toolBoxOptions->addItem( selFunctionGB, tr( "Selection" ));
    _toolBoxOptions->addItem( objectInspectoGB, tr( "Inspector" ));

    verticalLayout->setAlignment( Qt::AlignTop );
//    verticalLayout->addWidget( _groupBoxTransferFunction );
//    verticalLayout->addWidget( _groupBoxGroups );
    verticalLayout->addWidget( _modeSelectionWidget );
    verticalLayout->addWidget( _toolBoxOptions );
//    verticalLayout->addWidget( tSpeedGB );
//    verticalLayout->addWidget( dFunctionGB );
//    verticalLayout->addWidget( rFunctionGB );
//    verticalLayout->addWidget( selFunctionGB  );

    topContainer->setLayout( verticalLayout );
    _simConfigurationDock->setWidget( topContainer );

    this->addDockWidget( Qt::/*DockWidgetAreas::enum_type::*/RightDockWidgetArea,
                         _simConfigurationDock );

  //  connect( applyColorButton, SIGNAL( clicked( void )),
  //             this, SLOT( UpdateSimulationColorMapping( void )));

    connect( _modeSelectionWidget, SIGNAL( currentChanged( int )),
             _openGLWidget, SLOT( setMode( int )));

    connect( _openGLWidget, SIGNAL( attributeStatsComputed( void )),
             this, SLOT( updateAttributeStats( void )));

    connect( _comboAttribSelection, SIGNAL( currentIndexChanged( int )),
             _openGLWidget, SLOT( selectAttrib( int )));

    connect( comboShader, SIGNAL( currentIndexChanged( int )),
             _openGLWidget, SLOT( changeShader( int )));

    connect( _tfWidget, SIGNAL( colorChanged( void )),
             this, SLOT( UpdateSimulationColorMapping( void )));
    connect( _tfWidget, SIGNAL( colorChanged( void )),
             this, SLOT( UpdateSimulationSizeFunction( void )));
    connect( _tfWidget, SIGNAL( previewColor( void )),
             this, SLOT( PreviewSimulationColorMapping( void )));
    connect( _tfWidget, SIGNAL( previewColor( void )),
             this, SLOT( PreviewSimulationSizeFunction( void )));

    connect( _deltaTimeBox, SIGNAL( valueChanged( double )),
               this, SLOT( updateSimDeltaTime( void )));

    connect( _timeStepsPSBox, SIGNAL( valueChanged( double )),
               this, SLOT( updateSimTimestepsPS( void )));

    connect( _decayBox, SIGNAL( valueChanged( double )),
             this, SLOT( updateSimulationDecayValue( void )));

    connect( _stepByStepDurationBox, SIGNAL( valueChanged( double )),
             this, SLOT( updateSimStepByStepDuration( void )));

    connect( _alphaNormalButton, SIGNAL( toggled( bool )),
             this, SLOT( AlphaBlendingToggled( void ) ));

#ifdef VISIMPL_USE_ZEROEQ
#ifdef VISIMPL_USE_GMRVLEX
    connect( _clearSelectionButton, SIGNAL( clicked( void )),
             this, SLOT( ClearSelection( void )));
#endif
#endif

    connect( _addGroupButton, SIGNAL( clicked( void )),
             this, SLOT( addGroupFromSelection( )));

  //  connect( _alphaAccumulativeButton, SIGNAL( toggled( bool )),
  //           this, SLOT( AlphaBlendingToggled( void ) ));

    _alphaNormalButton->setChecked( true );
  }

  void MainWindow::_initSummaryWidget( void )
  {
    simil::TSimulationType simType =
        _openGLWidget->player( )->simulationType( );

    if( simType == simil::TSimSpikes )
    {
      simil::SpikesPlayer* spikesPlayer =
          dynamic_cast< simil::SpikesPlayer* >( _openGLWidget->player( ));

      std::cout << "Creating summary..." << std::endl;

      _summary->Init( spikesPlayer->data( ));

      _summary->simulationPlayer( _openGLWidget->player( ));

      _openGLWidget->subsetEventsManager( spikesPlayer->data( )->subsetsEvents( ));

      _openGLWidget->showEventsActivityLabels( _ui->actionShowEventsActivity->isChecked( ));
    }

  }

  void MainWindow::UpdateSimulationSlider( float percentage )
  {
    if( !_openGLWidget || !_openGLWidget->player( ))
      return;

    _startTimeLabel->setText(
          QString::number( (double)_openGLWidget->currentTime( )));

    int total = _simSlider->maximum( ) - _simSlider->minimum( );

    int position = percentage * total;

    _simSlider->setSliderPosition( position );

    if( _summary )
      _summary->repaintHistograms( );
  }

  void MainWindow::UpdateSimulationColorMapping( void )
  {
    _openGLWidget->changeSimulationColorMapping( _tfWidget->getColors( ));
  }

  void MainWindow::PreviewSimulationColorMapping( void )
  {
    _openGLWidget->changeSimulationColorMapping( _tfWidget->getPreviewColors( ));
  }

  void MainWindow::changeEditorColorMapping( void )
  {
    _tfWidget->setColorPoints( _openGLWidget->getSimulationColorMapping( ));
  }

  void MainWindow::changeEditorSizeFunction( void )
  {
    _tfWidget->setSizeFunction( _openGLWidget->getSimulationSizeFunction( ));
  }

  void MainWindow::UpdateSimulationSizeFunction( void )
  {
    _openGLWidget->changeSimulationSizeFunction( _tfWidget->getSizeFunction( ));
  }

  void MainWindow::PreviewSimulationSizeFunction( void )
  {
    _openGLWidget->changeSimulationSizeFunction( _tfWidget->getSizePreview( ));
  }

  void MainWindow::changeEditorSimDeltaTime( void )
  {
    _deltaTimeBox->setValue( _openGLWidget->simulationDeltaTime( ));
  }

  void MainWindow::updateSimDeltaTime( void )
  {
    _openGLWidget->simulationDeltaTime( _deltaTimeBox->value( ));
  }

  void MainWindow::changeEditorSimTimestepsPS( void )
  {
    _timeStepsPSBox->setValue( _openGLWidget->simulationStepsPerSecond( ));
  }

  void MainWindow::updateSimTimestepsPS( void )
  {
    _openGLWidget->simulationStepsPerSecond( _timeStepsPSBox->value( ));
  }

  void MainWindow::changeEditorDecayValue( void )
  {
    _decayBox->setValue( _openGLWidget->getSimulationDecayValue( ));
  }

  void MainWindow::updateSimulationDecayValue( void )
  {
    _openGLWidget->changeSimulationDecayValue( _decayBox->value( ));
  }


  void MainWindow::changeEditorStepByStepDuration( void )
  {
    _stepByStepDurationBox->setValue( _openGLWidget->simulationStepByStepDuration( ));
  }

  void MainWindow::updateSimStepByStepDuration( void )
  {
    _openGLWidget->simulationStepByStepDuration( _stepByStepDurationBox->value( ));
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

  void MainWindow::updateAttributeStats( void )
  {

    if( !_domainManager )
      return;

    QLayoutItem* item;
    while(( item = _layoutAttribStats->takeAt( 0 )))
    {
      _layoutAttribStats->removeWidget( item->widget( ));
      delete item->widget( );
    }

    std::cout << "Updating stats..." << std::endl;

//    int attribNumber = _comboAttribSelection->currentIndex( );

//    auto values = _domainManager->attributeValues( attribNumber );
//    auto names = _domainManager->attributeNames( attribNumber );
//    auto labels = _domainManager->attributeNames( attribNumber, true );

    auto stats = _domainManager->attributeStatistics( );

//    for( unsigned int i = 0; i < stats.size( ); ++i )
    for( auto stat : stats )
    {
      auto name = std::get< T_TYPE_NAME >( stat );//names[ i ];
      auto label = std::get< T_TYPE_LABEL >( stat ); //labels[ i ];
      auto number = std::get< T_TYPE_STATS >( stat );//stats.find( i )->second;

      QString text = ( QString( name.c_str( )) //+ " - " + QString( label.c_str( ))
          + ": " + QString::number( number ) );

      QLabel* textLabel = new QLabel( text );
//      std::cout << "Text " << text.toStdString( ) << std::endl;
      _layoutAttribStats->addWidget( textLabel );
    }

    while(( item = _layoutAttribGroups->takeAt( 0 )))
    {
      _layoutAttribGroups->removeWidget( item->widget( ));
      delete item->widget( );
    }

    _attribGroupsVisButtons.clear( );

//    auto colors = _openGLWidget->domainManager( )->paletteColors( );
    unsigned int currentIndex = 0;
    for( auto group : _openGLWidget->domainManager( )->attributeGroups( ))
    {
      QFrame* frame = new QFrame( );
//      auto colors = _openGLWidget->colorPalette( ).colors( );
//colors[ currentIndex % colors.size( )].first
      frame->setStyleSheet( "background-color: " + group->color( ).name( ) );
      frame->setMinimumSize( 20, 20 );
      frame->setMaximumSize( 20, 20 );

      group->name( std::get< T_TYPE_NAME >( stats[ currentIndex ])); //names[ currentIndex] );

      //    QIcon* eye = new QIcon( ":/icons/show.png" );
      QCheckBox* buttonVisibility = new QCheckBox( group->name( ).c_str( ));
      buttonVisibility->setChecked( true );
      //    buttonVisibility->setMinimumSize( 60, 50 );
      //    buttonVisibility->setMaximumSize( 60, 50 );

      _attribGroupsVisButtons.push_back( buttonVisibility );

      connect( buttonVisibility, SIGNAL( clicked( )),
              this, SLOT( checkAttributeGroupsVisibility( )));

      _layoutAttribGroups->addWidget( frame, currentIndex, 0, 1, 1 );
      _layoutAttribGroups->addWidget( buttonVisibility, currentIndex, 2, 1, 1 );

//      _openGLWidget->setUpdateGroups( );

      ++currentIndex;
    }


    _layoutAttribGroups->update( );
  }

  void MainWindow::updateSelectedStatsPickingSingle( unsigned int selected )
  {
    auto pickingInfo = _domainManager->pickingInfoSimple( selected );

    _labelGID->setText( QString::number( std::get< T_PART_GID >( pickingInfo )));

    auto position = std::get< T_PART_POSITION >( pickingInfo );

    QString posText = "( " + QString::number( position.x )
                    + ", " + QString::number( position.y )
                    + ", " + QString::number( position.z )
                    + " )";



    _labelPosition->setText( posText );

    _toolBoxOptions->setCurrentIndex( ( unsigned int ) T_TOOL_Inpector );

  }


  void MainWindow::showInactive( bool show )
  {
    _openGLWidget->showInactive( show );
  }


  void MainWindow::playAtButtonClicked( void )
  {
    if( !_openGLWidget || !_openGLWidget->player( ))
      return;

    bool ok;
    double result =
        QInputDialog::getDouble( this, tr( "Set simulation time to play:"),
                                 tr( "Simulation time" ),
                                 ( double )_openGLWidget->currentTime( ),
                                 ( double )_openGLWidget->player( )->data( )->startTime( ),
                                 ( double )_openGLWidget->player( )->data( )->endTime( ),
                                 3, &ok, Qt::Popup );

    if( ok )
    {
      float percentage = ( result - _openGLWidget->player( )->startTime( )) /
          ( _openGLWidget->player( )->endTime( ) -
              _openGLWidget->player( )->startTime( ));

      percentage = std::max( 0.0f, std::min( 1.0f, percentage ));

      PlayAt( percentage, true );
    }
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
        [&]( const void* data_, const size_t size_ )
        { _onSelectionEvent( lexis::data::SelectedIDs::create( data_, size_ ));});

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

  void MainWindow::ClearSelection( void )
  {
    if( _openGLWidget )
    {
      _openGLWidget->clearSelection( );

      _addGroupButton->setEnabled( false );
      _clearSelectionButton->setEnabled( false );
      _selectionSizeLabel->setText( "0" );
    }
  }

  void MainWindow::_onSelectionEvent( lexis::data::ConstSelectedIDsPtr selected )
  {

    std::cout << "Received selection" << std::endl;
    if( _openGLWidget )
    {
  //    std::vector< unsigned int > selected =
  //        zeq::hbp::deserializeSelectedIDs( selected );

      std::vector< uint32_t > ids = selected->getIdsVector( );



      visimpl::GIDUSet selectedSet( ids.begin( ), ids.end( ));

      if( selectedSet.size( ) == 0 )
        return;

      _openGLWidget->setSelectedGIDs( selectedSet );

      _addGroupButton->setEnabled( true );
      _clearSelectionButton->setEnabled( true );
      _selectionSizeLabel->setText( QString::number( selectedSet.size( )));
    }

  }

#endif

  void MainWindow::addGroupFromSelection( void )
  {
    DomainManager* inputMux = _openGLWidget->domainManager( );

    unsigned int currentIndex = inputMux->groups( ).size( );

    QString groupName( "Group " + QString::number( currentIndex ));

    if( !_autoNameGroups )
    {
      bool ok;
      groupName =
          QInputDialog::getText( this, tr( "Group Name" ),
                                 tr( "Please, introduce group name: "),
                                 QLineEdit::Normal,
                                 groupName,
                                 &ok );

      if( !ok )
        return;
    }

    _openGLWidget->addGroupFromSelection( groupName.toStdString( ));

    QFrame* frame = new QFrame( );
    auto colors = _openGLWidget->colorPalette( ).colors( );

    frame->setStyleSheet( "background-color: " + colors[ currentIndex ].name( ) );
    frame->setMinimumSize( 20, 20 );
    frame->setMaximumSize( 20, 20 );

//    QIcon* eye = new QIcon( ":/icons/show.png" );
    QCheckBox* buttonVisibility = new QCheckBox( "active" );
    buttonVisibility->setChecked( true );
//    buttonVisibility->setMinimumSize( 60, 50 );
//    buttonVisibility->setMaximumSize( 60, 50 );

    _groupsVisButtons.push_back( buttonVisibility );

    connect( buttonVisibility, SIGNAL( clicked( )),
             this, SLOT( checkGroupsVisibility( )));

    _groupLayout->addWidget( frame, currentIndex, 0, 1, 1 );
    _groupLayout->addWidget( new QLabel( groupName ), currentIndex, 1, 1, 1 );
    _groupLayout->addWidget( buttonVisibility, currentIndex, 2, 1, 1 );

    _openGLWidget->setUpdateGroups( );
    _openGLWidget->update( );
  }

  void MainWindow::checkGroupsVisibility( void )
  {
    unsigned int counter = 0;
    auto group = _openGLWidget->domainManager( )->groups( ).begin( );
    for( auto button : _groupsVisButtons )
    {
      if( button->isChecked( ) != ( *group)->active( ) )
      {
        _openGLWidget->domainManager( )->setVisualGroupState( counter, button->isChecked( ));

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
    for( auto button : _attribGroupsVisButtons )
    {
      if( button->isChecked( ) != ( *group)->active( ) )
      {
        _openGLWidget->domainManager( )->setVisualGroupState( counter, button->isChecked( ), true);

      }
      ++group;
      ++counter;
    }

//    _openGLWidget->setUpdateGroups( );
    _openGLWidget->update( );
  }



  void MainWindow::PlayPause( bool notify )
  {
    if( !_openGLWidget || !_openGLWidget->player( ))
      return;

    if( !_openGLWidget->player( )->isPlaying( ))
      Play( notify );
    else
      Pause( notify );
  }

  void MainWindow::Play( bool notify )
  {
  //  playIcon.swap( pauseIcon );
    if( !_openGLWidget || !_openGLWidget->player( ))
      return;

    if( _openGLWidget )
    {
      _openGLWidget->Play( );

      _playButton->setIcon( _pauseIcon );

      if( _openGLWidget->playbackMode( ) == TPlaybackMode::STEP_BY_STEP &&
          _openGLWidget->completedStep( ))
        _openGLWidget->playbackMode( TPlaybackMode::CONTINUOUS );

      if( notify )
      {
  #ifdef SIMIL_USE_ZEROEQ
        _openGLWidget->player( )->zeqEvents( )->sendPlaybackOp( zeroeq::gmrv::PLAY );
  #endif
      }
    }
  }

  void MainWindow::Pause( bool notify )
  {
    if( !_openGLWidget || !_openGLWidget->player( ))
      return;

    if( _openGLWidget )
    {
      _openGLWidget->Pause( );
      _playButton->setIcon( _playIcon );

      if( notify )
      {
  #ifdef SIMIL_USE_ZEROEQ
      _openGLWidget->player( )->zeqEvents( )->sendPlaybackOp( zeroeq::gmrv::PAUSE );
  #endif
      }
    }
  }

  void MainWindow::Stop( bool notify )
  {
    if( !_openGLWidget || !_openGLWidget->player( ))
      return;

    if( _openGLWidget )
    {
      _openGLWidget->Stop( );
      _playButton->setIcon( _playIcon );
      _startTimeLabel->setText(
            QString::number( (double)_openGLWidget->player( )->startTime( )));

      _openGLWidget->playbackMode( TPlaybackMode::CONTINUOUS );

      if( notify )
      {
  #ifdef SIMIL_USE_ZEROEQ
        _openGLWidget->player( )->zeqEvents( )->sendPlaybackOp( zeroeq::gmrv::STOP );
  #endif
      }

    }
  }

  void MainWindow::Repeat( bool notify )
  {
    if( !_openGLWidget || !_openGLWidget->player( ))
      return;

    if( _openGLWidget )
    {
      bool repeat = _repeatButton->isChecked( );
  //    std::cout << "Repeat " << boolalpha << repeat << std::endl;
      _openGLWidget->Repeat( repeat );

      if( notify )
      {
  #ifdef SIMIL_USE_ZEROEQ
        _openGLWidget->player( )->zeqEvents( )->sendPlaybackOp( repeat ?
                                    zeroeq::gmrv::ENABLE_LOOP :
                                    zeroeq::gmrv::DISABLE_LOOP );
  #endif
      }

    }
  }

  void MainWindow::requestPlayAt( float percentage )
  {
    if( !_openGLWidget || !_openGLWidget->player( ))
      return;

    PlayAt( percentage, false );
  }

  void MainWindow::PlayAt( bool notify )
  {
    if( !_openGLWidget || !_openGLWidget->player( ))
      return;

    if( _openGLWidget )
    {
      PlayAt( _simSlider->sliderPosition( ), notify );
    }
  }

  void MainWindow::PlayAt( float percentage, bool notify )
  {
    if( !_openGLWidget || !_openGLWidget->player( ))
      return;

    if( _openGLWidget )
    {
      int sliderPosition = percentage *
                      ( _simSlider->maximum( ) - _simSlider->minimum( )) +
                      _simSlider->minimum( );

      _simSlider->setSliderPosition( sliderPosition );

  //    _openGLWidget->resetParticles( );

      _playButton->setIcon( _pauseIcon );

      _openGLWidget->PlayAt( percentage );

      _openGLWidget->playbackMode( TPlaybackMode::CONTINUOUS );

      if( notify )
      {
  #ifdef SIMIL_USE_ZEROEQ
      // Send event
      _openGLWidget->player( )->zeqEvents( )->sendFrame( _simSlider->minimum( ),
                             _simSlider->maximum( ),
                             sliderPosition );

      _openGLWidget->player( )->zeqEvents( )->sendPlaybackOp( zeroeq::gmrv::PLAY );
  #endif
      }

    }
  }

  void MainWindow::PlayAt( int sliderPosition, bool notify )
  {
    if( !_openGLWidget || !_openGLWidget->player( ))
      return;

    if( _openGLWidget )
    {

  //    _openGLWidget->Pause( );


      int value = _simSlider->value( );
      float percentage = float( value - _simSlider->minimum( )) /
                         float( _simSlider->maximum( ) - _simSlider->minimum( ));

      _simSlider->setSliderPosition( sliderPosition );

  //    _openGLWidget->resetParticles( );

      _playButton->setIcon( _pauseIcon );

      _openGLWidget->PlayAt( percentage );

      _openGLWidget->playbackMode( TPlaybackMode::CONTINUOUS );

      if( notify )
      {
  #ifdef SIMIL_USE_ZEROEQ
      // Send event
      _openGLWidget->player( )->zeqEvents( )->sendFrame( _simSlider->minimum( ),
                             _simSlider->maximum( ),
                             sliderPosition );

      _openGLWidget->player( )->zeqEvents( )->sendPlaybackOp( zeroeq::gmrv::PLAY );
  #endif
      }
    }
  }

  void MainWindow::PreviousStep( bool /*notify*/ )
  {
    if( !_openGLWidget || !_openGLWidget->player( ))
      return;

    if( _openGLWidget )
    {
      _playButton->setIcon( _pauseIcon );
      _openGLWidget->player( )->Play( );
      _openGLWidget->PreviousStep( );
    }
  }

  void MainWindow::NextStep( bool /*notify*/ )
  {
    if( !_openGLWidget || !_openGLWidget->player( ))
      return;

    if( _openGLWidget )
    {
      _playButton->setIcon( _pauseIcon );
      _openGLWidget->player( )->Play( );
      _openGLWidget->NextStep( );

    }
  }

  void MainWindow::completedStep( void )
  {
    if( _openGLWidget )
    {
      _playButton->setIcon( _playIcon );
    }
  }

} // namespace visimpl
