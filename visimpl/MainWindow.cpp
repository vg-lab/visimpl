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

#include "simil/Network.h"
#include "sumrice/ReconnectRESTDialog.h"
#include <memory>

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

#include <acuterecorder/acuterecorder.h>

#include "MainWindow.h"
#include "SaveScreenshotDialog.h"

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
#include <QRadioButton>
#include <QGroupBox>
#include <QPushButton>
#include <QToolBox>
#include <QtGlobal>
#include <QWindow>
#include <QScreen>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMenu>

#include <thread>
#include <iterator>

#include <sumrice/sumrice.h>

template< class T >
void ignore( const T& )
{ }

constexpr const char* POSITION_KEY_ = "positionData";
constexpr const char* PLANES_COLOR_KEY_ = "clippingPlanesColor";
constexpr const char* GROUP_NAME_ = "groupName";

namespace visimpl
{
  enum toolIndex
  {
    T_TOOL_Playback = 0 ,
    T_TOOL_Visual ,
    T_TOOL_Selection ,
    T_TOOL_Inpector
  };

  MainWindow::MainWindow( QWidget* parent_ , bool updateOnIdle )
    : QMainWindow( parent_ )
    , _ui( new Ui::MainWindow )
    , _lastOpenedNetworkFileName( "" )
    , _playIcon( ":/icons/play.svg" )
    , _pauseIcon( ":/icons/pause.svg" )
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
    , _stackVizDock( nullptr )
    , _stackViz( nullptr )
    , _modeSelectionWidget( nullptr )
    , _toolBoxOptions( nullptr )
    , _objectInspectorGB{ nullptr }
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
    , _buttonLoadGroups{ nullptr }
    , _buttonSaveGroups{ nullptr }
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
    , _recorder( nullptr )
    , m_loader{ nullptr }
    , m_loaderDialog{ nullptr }
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

    _ui->actionUpdateOnIdle->setChecked( updateOnIdle );
    _ui->actionShowFPSOnIdleUpdate->setChecked( false );

    _ui->actionShowEventsActivity->setChecked( false );

#ifdef SIMIL_USE_BRION
    _ui->actionOpenBlueConfig->setEnabled( true );
#else
    _ui->actionOpenBlueConfig->setEnabled( false );
#endif

    auto positionsMenu = new QMenu( );
    positionsMenu->setTitle( "Camera positions" );
    _ui->actionCamera_Positions->setMenu( positionsMenu );
  }

  void MainWindow::init( const std::string& zeqUri )
  {
    _openGLWidget = new OpenGLWidget( this , Qt::WindowFlags( ) , zeqUri );

    this->setCentralWidget( _openGLWidget );

#ifdef VISIMPL_USE_ZEROEQ

    try
    {
      auto& zInstance = ZeroEQConfig::instance( );
      if ( !zInstance.isConnected( ))
      {
        zInstance.connect( zeqUri );
      }

      zInstance.subscriber( )->subscribe(
        lexis::data::SelectedIDs::ZEROBUF_TYPE_IDENTIFIER( ) ,
        [ & ]( const void* data_ , unsigned long long size_ )
        {
          _onSelectionEvent( lexis::data::SelectedIDs::create( data_ , size_ ));
        } );

      // receive loop will be started by OpenGLWidget after loading data.
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

    _openGLWidget->idleUpdate( _ui->actionUpdateOnIdle->isChecked( ));

    connect( _ui->actionUpdateOnIdle , SIGNAL( triggered( void )) ,
             _openGLWidget , SLOT( toggleUpdateOnIdle( void )));

    connect( _ui->actionBackgroundColor , SIGNAL( triggered( void )) ,
             _openGLWidget , SLOT( changeClearColor( void )));

    connect( _openGLWidget , SIGNAL( planesColorChanged(
                                       const QColor & )) ,
             this , SLOT( changePlanesColor(
                            const QColor & )));

    connect( _ui->actionShowFPSOnIdleUpdate , SIGNAL( triggered( void )) ,
             _openGLWidget , SLOT( toggleShowFPS( void )));

    connect( _ui->actionShowEventsActivity , SIGNAL( triggered( bool )) ,
             _openGLWidget , SLOT( showEventsActivityLabels( bool )));

    connect( _ui->actionShowCurrentTime , SIGNAL( triggered( bool )) ,
             _openGLWidget , SLOT( showCurrentTimeLabel( bool )));

    connect( _ui->actionOpenBlueConfig , SIGNAL( triggered( void )) , this ,
             SLOT( openBlueConfigThroughDialog( void )));

    connect( _ui->actionOpenCSVFiles , SIGNAL( triggered( void )) , this ,
             SLOT( openCSVFilesThroughDialog( void )));

    connect( _ui->actionOpenH5Files , SIGNAL( triggered( void )) , this ,
             SLOT( openHDF5ThroughDialog( void )));

    connect( _ui->actionConnectREST , SIGNAL( triggered( void )) , this ,
             SLOT( openRESTThroughDialog( )));

    connect( _ui->actionConfigureREST , SIGNAL( triggered( )) , this ,
             SLOT( configureREST( )));

    connect( _ui->actionOpenSubsetEventsFile , SIGNAL( triggered( void )) ,
             this , SLOT( openSubsetEventsFileThroughDialog( void )));

    connect(_ui->actionTake_screenshot, SIGNAL(triggered()),
            this, SLOT(saveScreenshot()));

    _ui->actionOpenSubsetEventsFile->setEnabled( false );

#ifdef SIMIL_WITH_REST_API
    _ui->actionConnectREST->setEnabled( true );
#endif

    connect( _ui->actionCloseData , SIGNAL( triggered( void )) , this ,
             SLOT( closeData( void )));

    connect( _ui->actionQuit , SIGNAL( triggered( void )) , this ,
             SLOT( close( )));

    connect( _ui->actionAbout , SIGNAL( triggered( void )) , this ,
             SLOT( dialogAbout( void )));

    connect( _ui->actionHome , SIGNAL( triggered( void )) , _openGLWidget ,
             SLOT( home( void )));

    connect( _openGLWidget , SIGNAL( stepCompleted( void )) , this ,
             SLOT( completedStep( void )));

    connect( _openGLWidget , SIGNAL( pickedSingle( unsigned int )) , this ,
             SLOT( updateSelectedStatsPickingSingle( unsigned int )));

    connect( _ui->actionAdd_camera_position , SIGNAL( triggered( bool )) ,
             this ,
             SLOT( addCameraPosition( )));

    connect( _ui->actionRemove_camera_position , SIGNAL( triggered( bool )) ,
             this ,
             SLOT( removeCameraPosition( )));

    connect( _ui->actionLoad_camera_positions , SIGNAL( triggered( bool )) ,
             this ,
             SLOT( loadCameraPositions( )));

    connect( _ui->actionSave_camera_positions , SIGNAL( triggered( bool )) ,
             this ,
             SLOT( saveCameraPositions( )));

    QAction* actionTogglePause = new QAction( this );
    actionTogglePause->setShortcut( Qt::Key_Space );

    connect( actionTogglePause , SIGNAL( triggered( )) , this ,
             SLOT( PlayPause( )));
    addAction( actionTogglePause );

#ifdef VISIMPL_USE_ZEROEQ
    _ui->actionShowInactive->setEnabled( true );
    _ui->actionShowInactive->setChecked( true );

    _openGLWidget->showInactive( true );

    connect( _ui->actionShowInactive , SIGNAL( toggled( bool )) , this ,
             SLOT( showInactive( bool )) );
#endif

    _initPlaybackDock( );
    _initSimControlDock( );
    _initStackVizDock( );
    _ui->actionToggleStackVizDock->setEnabled( false );

    connect(
      _simulationDock->toggleViewAction( ) , SIGNAL( toggled( bool )) ,
      _ui->actionTogglePlaybackDock , SLOT( setChecked( bool ))
    );

    connect(
      _ui->actionTogglePlaybackDock , SIGNAL( triggered( )) ,
      this , SLOT( togglePlaybackDock( ))
    );

    connect(
      _simConfigurationDock->toggleViewAction( ) , SIGNAL( toggled( bool )) ,
      _ui->actionToggleSimConfigDock , SLOT( setChecked( bool ))
    );

    connect(
      _ui->actionToggleSimConfigDock , SIGNAL( triggered( )) ,
      this , SLOT( toggleSimConfigDock( ))
    );

    connect(
      _stackVizDock->toggleViewAction( ) , SIGNAL( toggled( bool )) ,
      _ui->actionToggleStackVizDock , SLOT( setChecked( bool ))
    );

    connect(
      _stackVizDock->toggleViewAction( ) , SIGNAL( toggled( bool )) ,
      this , SLOT( changeStackVizToolbarStatus( bool ))
    );
    changeStackVizToolbarStatus( false );

    connect(
      _ui->actionToggleStackVizDock , SIGNAL( triggered( )) ,
      this , SLOT( toggleStackVizDock( ))
    );

    _ui->toolBar->setContextMenuPolicy( Qt::PreventContextMenu );
    _ui->menubar->setContextMenuPolicy( Qt::PreventContextMenu );

    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_F), this, SLOT(showMaximized()));
    new QShortcut(QKeySequence(Qt::CTRL + +Qt::SHIFT + Qt::Key_F), this, SLOT(presentationMode()));
  }

  MainWindow::~MainWindow( void )
  {
#ifdef VISIMPL_USE_ZEROEQ

    auto& instance = ZeroEQConfig::instance( );
    if ( instance.isConnected( ))
      instance.disconnect( );

#endif
    delete _ui;

    m_loader = nullptr;
    closeLoadingDialog( );
  }

  void MainWindow::showStatusBarMessage( const QString& message )
  {
    _ui->statusbar->showMessage( message );
  }

  void MainWindow::configureComponents( void )
  {
    _domainManager = _openGLWidget->domainManager( );

    _selectionManager->setGIDs( _openGLWidget->player( )->gids( ));

    _subsetEvents = _openGLWidget->subsetEventsManager( );

    _ui->actionToggleStackVizDock->setEnabled( true );
    _ui->actionOpenSubsetEventsFile->setEnabled( true );
    _ui->actionCloseData->setEnabled( true );

    _buttonImportGroups->setEnabled(
      _subsetEvents && _subsetEvents->numSubsets( ) > 0 );

    if ( _openGLWidget )
    {
      auto player = _openGLWidget->player( );
      if ( player )
      {
        const auto tBegin = player->startTime( );
        const auto tEnd = player->endTime( );
        const auto tCurrent = player->currentTime( );

        _startTimeLabel->setText( QString::number( tCurrent , 'f' , 3 ));
        _endTimeLabel->setText( QString::number( tEnd , 'f' , 3 ));

        const auto percentage = ( tCurrent - tBegin ) / ( tEnd - tBegin );
        UpdateSimulationSlider( percentage );
      }
    }

    _simulationDock->setEnabled( true );
    _simConfigurationDock->setEnabled( true );
  }

  void MainWindow::openBlueConfigThroughDialog( void )
  {
#ifdef SIMIL_USE_BRION

    QString path = QFileDialog::getOpenFileName(
      this , tr( "Open BlueConfig" ) , _lastOpenedNetworkFileName ,
      tr( "BlueConfig ( BlueConfig CircuitConfig);; All files (*)" ) , nullptr ,
      QFileDialog::DontUseNativeDialog );

    if ( !path.isEmpty( ))
    {
      bool ok;

      QString text = QInputDialog::getText( this ,
                                            tr( "Please type a target" ) ,
                                            tr( "Target name:" ) ,
                                            QLineEdit::Normal , "Mosaic" ,
                                            &ok );

      if ( ok && !text.isEmpty( ))
      {
        std::string reportLabel = text.toStdString( );
        _lastOpenedNetworkFileName = QFileInfo( path ).path( );
        std::string fileName = path.toStdString( );

        loadData( simil::TBlueConfig , fileName , reportLabel ,
                  simil::TSimSpikes );
      }
    }
#else
    const QString title = tr( "Open BlueConfig" );
    const QString message = tr(
      "BlueConfig loading is not supported in this version." );
    QMessageBox::critical( this , title , message , QMessageBox::Button::Ok );
#endif
  }

  void MainWindow::openCSVFilesThroughDialog( void )
  {
    QString pathNetwork = QFileDialog::getOpenFileName(
      this , tr( "Open CSV Network description file" ) ,
      _lastOpenedNetworkFileName , tr( "CSV (*.csv);; All files (*)" ) ,
      nullptr ,
      QFileDialog::DontUseNativeDialog );

    if ( !pathNetwork.isEmpty( ))
    {
      QString pathActivity = QFileDialog::getOpenFileName(
        this , tr( "Open CSV Activity file" ) , _lastOpenedNetworkFileName ,
        tr( "CSV (*.csv);; All files (*)" ) , nullptr ,
        QFileDialog::DontUseNativeDialog );

      if ( !pathActivity.isEmpty( ))
      {
        _lastOpenedNetworkFileName = QFileInfo( pathNetwork ).path( );
        _lastOpenedActivityFileName = QFileInfo( pathActivity ).path( );
        const auto networkFile = pathNetwork.toStdString( );
        const auto activityFile = pathActivity.toStdString( );

        loadData( simil::TCSV , networkFile , activityFile ,
                  simil::TSimSpikes );
      }
    }
  }

  void MainWindow::openHDF5ThroughDialog( void )
  {
    auto path = QFileDialog::getOpenFileName(
      this , tr( "Open a H5 network file" ) , _lastOpenedNetworkFileName ,
      tr( "hdf5 (*.h5 *.hdf5);; All files (*)" ) , nullptr ,
      QFileDialog::DontUseNativeDialog );

    if ( path.isEmpty( )) return;

    const auto networkFile = path.toStdString( );

    path =
      QFileDialog::getOpenFileName( this , tr( "Open a H5 activity file" ) ,
                                    path ,
                                    tr( "hdf5 (*.h5 *.hdf5);; All files (*)" ) ,
                                    nullptr ,
                                    QFileDialog::DontUseNativeDialog );

    if ( path.isEmpty( )) return;

    const auto activityFile = path.toStdString( );

    loadData( simil::THDF5 , networkFile , activityFile , simil::TSimSpikes );
  }

  void MainWindow::openSubsetEventFile( const std::string& filePath ,
                                        bool append )
  {
    if ( filePath.empty( ) || !_subsetEvents )
      return;

    if ( !append )
      _subsetEvents->clear( );

    QFileInfo eventsFile{ QString::fromStdString( filePath ) };
    if ( !eventsFile.exists( ))
      return;

    bool h5 = false;
    QString errorText;
    try
    {
      if ( eventsFile.suffix( ).toLower( ).compare( "json" ) == 0 )
      {
        _subsetEvents->loadJSON( filePath );
      }
      else if ( eventsFile.suffix( ).toLower( ).compare( "h5" ) == 0 ||
                eventsFile.suffix( ).toLower( ).compare( "hdf5" ) == 0 )
      {
        _subsetEvents->loadH5( filePath );
        h5 = true;
      }
      else
      {
        errorText = tr( "Events file not supported: %1" ).arg(
          eventsFile.absoluteFilePath( ));
      }
    }
    catch ( const std::exception& e )
    {
      errorText = QString::fromLocal8Bit( e.what( ));
    }

    if ( !errorText.isEmpty( ))
    {
      QMessageBox::warning( this , tr( "Error loading Events file" ) ,
                            errorText , QMessageBox::Ok );
      return;
    }

    _subsetImporter->reload( _subsetEvents );

    _openGLWidget->subsetEventsManager( _subsetEvents );
    _openGLWidget->showEventsActivityLabels(
      _ui->actionShowEventsActivity->isChecked( ));
    _stackViz->openSubsetEventsFile( h5 );
  }

  void MainWindow::openSubsetEventsFileThroughDialog( void )
  {
    QString filePath = QFileDialog::getOpenFileName(
      this , tr( "Open file containing subsets/events data" ) ,
      _lastOpenedSubsetsFileName ,
      tr( "JSON (*.json);; hdf5 (*.h5 *.hdf5);; All files (*)" ) ,
      nullptr , QFileDialog::DontUseNativeDialog );

    if ( !filePath.isEmpty( ))
    {
      QFileInfo eventsFile{ filePath };
      if ( eventsFile.exists( ))
      {
        _lastOpenedSubsetsFileName = eventsFile.path( );
        openSubsetEventFile( filePath.toStdString( ) , false );
      }
    }
  }

  void MainWindow::openRecorder( void )
  {
    auto action = qobject_cast< QAction* >( sender( ));

    // The button stops the recorder if found.
    if ( _recorder != nullptr )
    {
      if ( action ) action->setDisabled( true );
      const bool currentlyPlaying = ( _openGLWidget &&
                                      _openGLWidget->player( ) &&
                                      _openGLWidget->player( )->isPlaying( ));
      if ( currentlyPlaying ) Pause( );

      RecorderUtils::stopAndWait( _recorder , this );

      if ( currentlyPlaying ) Play( );

      // Recorder will be deleted after finishing.
      _recorder = nullptr;
      return;
    }

    RSWParameters params;
    params.widgetsToRecord.emplace_back( "Viewport" , _openGLWidget );
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

    RecorderDialog dialog( nullptr , params , false );
    dialog.setWindowIcon( QIcon( ":/visimpl.png" ));
    dialog.setFixedSize( 800 , 600 );
    if ( dialog.exec( ) == QDialog::Accepted )
    {
      _recorder = dialog.getRecorder( );
      connect( _recorder , SIGNAL( finished( )) ,
               _recorder , SLOT( deleteLater( )));
      connect( _recorder , SIGNAL( finished( )) ,
               this , SLOT( finishRecording( )));
      connect( _openGLWidget , SIGNAL( frameSwapped( )) ,
               _recorder , SLOT( takeFrame( )));
      if ( action ) action->setChecked( true );
    }
    else
    {
      if ( action ) action->setChecked( false );
    }
  }

  void MainWindow::closeData( void )
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

        _objectInspectorGB->setCheckUpdates( false );

        _summary->UpdateHistograms( );
        _stackViz->updateHistograms( );

        _objectInspectorGB->update( );
        _objectInspectorGB->setCheckUpdates( true );

        _openGLWidget->resetParticles( );
        _simSlider->setSliderPosition( 0 );

        repaint( );

        QApplication::processEvents( );

        QApplication::restoreOverrideCursor( );

        return;
      }
    }
#endif
    QApplication::setOverrideCursor( Qt::WaitCursor );

    Stop( );

    clearGroups( );
    clearSelection( );
    _openGLWidget->setPlayer( nullptr , simil::TDataType::TDataUndefined );
    _stackViz->closeData( );
    _selectionManager->setGIDs( TGIDSet( ));
    _domainManager->setSelection( TGIDSet( ) , tGidPosMap( ));
    _ui->actionToggleStackVizDock->setChecked( false );
    _ui->actionToggleStackVizDock->setEnabled( false );
    _ui->actionCloseData->setEnabled( false );

    _objectInspectorGB->setCheckUpdates( false );
    _objectInspectorGB->setSimPlayer( nullptr );

    _simSlider->setSliderPosition( 0 );
    _summary->clear( );
    _simulationDock->setEnabled( false );
    _simConfigurationDock->setEnabled( false );

    m_loader = nullptr;

    _tfWidget->setColorPoints( visimpl::TTransferFunction( ));
    _tfWidget->setSizeFunction( visimpl::TSizeFunction( ));

    QApplication::restoreOverrideCursor( );
  }

  void MainWindow::dialogAbout( void )
  {
    QString msj =
      QString( "<h2>ViSimpl</h2>" ) +
      tr( "A multi-view visual analyzer of brain simulation data. " ) + "<br>" +
      tr( "Version " ) + visimpl::Version::getString( ).c_str( ) +
      tr( " rev (%1)<br>" ).arg( visimpl::Version::getRevision( )) +
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
      "</li><li>ZeroEQ " + zeroeq::Version::getRevString().c_str() +
      #else
      "</li><li>ZeroEQ " + tr( "support not built." ) +
      #endif

      "</li><li>AcuteRecorder " + ACUTERECORDER_REV_STRING +

      "</li></ul>" + "<h4>" + tr( "Developed by:" ) + "</h4>" +
      "VG-Lab / URJC / UPM"
      "<br><a href='https://vg-lab.es/'>https://vg-lab.es/</a>"
      "<br>(C) 2015-" +
      QString::number( QDateTime::currentDateTime( ).date( ).year( )) +
      "<br><br>"
      "<a href='https://vg-lab.es'><img src=':/icons/logoVGLab.png'/></a>"
      "&nbsp;&nbsp;&nbsp;&nbsp;"
      "<a href='https://www.urjc.es'><img src=':/icons/logoURJC.png' /></a>"
      "&nbsp;&nbsp;&nbsp;&nbsp;"
      "<a href='https://www.upm.es'><img src=':/icons/logoUPM.png' /></a>" +
      "<br><br><a href='https://visimpl-documentation.readthedocs.io/en/latest/'>Online documentation</a>";

    QMessageBox::about( this , tr( "About ViSimpl" ) , msj );
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
    if ( QDialog::Accepted == _subsetImporter->exec( ))
    {
      importVisualGroups( );
    }
  }

  void MainWindow::togglePlaybackDock( void )
  {
    if ( _ui->actionTogglePlaybackDock->isChecked( ))
      _simulationDock->show( );
    else
      _simulationDock->close( );

    update( );
  }

  void MainWindow::toggleSimConfigDock( void )
  {
    if ( _ui->actionToggleSimConfigDock->isChecked( ))
      _simConfigurationDock->show( );
    else
      _simConfigurationDock->close( );

    update( );
  }

  void MainWindow::toggleStackVizDock( void )
  {
    if ( _ui->actionToggleStackVizDock->isChecked( ))
      _stackVizDock->show( );
    else
      _stackVizDock->close( );

    update( );
  }

  void MainWindow::_configurePlayer( void )
  {
    connect( _openGLWidget , SIGNAL( updateSlider( float )) , this ,
             SLOT( UpdateSimulationSlider( float )));

    _objectInspectorGB->setSimPlayer( _openGLWidget->player( ));

    _startTimeLabel->setText(
      QString::number( _openGLWidget->player( )->startTime( ) , 'f' , 3 ));

    _endTimeLabel->setText(
      QString::number( _openGLWidget->player( )->endTime( ) , 'f' , 3 ));

    _simSlider->setEnabled( true );

#ifdef SIMIL_USE_ZEROEQ
    try
    {
      const auto eventMgr = _openGLWidget->player( )->zeqEvents( );
      if ( eventMgr )
      {
        eventMgr->playbackOpReceived.connect(
          boost::bind( &MainWindow::ApplyPlaybackOperation , this , _1 ));
        eventMgr->frameReceived.connect(
          boost::bind( &MainWindow::requestPlayAt , this , _1 ));
      }
    }
    catch ( std::exception& e )
    {
      std::cerr << "Exception when initializing player events. ";
      std::cerr << e.what( ) << " " << __FILE__ << ":" << __LINE__ << std::endl;
    }
    catch ( ... )
    {
      std::cerr << "Unknown exception when initializing player events. "
                << __FILE__ << ":" << __LINE__ << std::endl;
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

  void MainWindow::_initStackVizDock( void )
  {
    _stackVizDock = new QDockWidget( );
    _stackVizDock->setObjectName( "stackvizDock" );
    _stackVizDock->setMinimumHeight( 100 );
    _stackVizDock->setSizePolicy( QSizePolicy::MinimumExpanding ,
                                  QSizePolicy::MinimumExpanding );
    _stackVizDock->setVisible( false );

    _stackViz = new StackViz( this );

    if ( _openGLWidget && _openGLWidget->player( ))
    {
      _stackViz->init( _openGLWidget->player( ) ,
                       _openGLWidget->subsetEventsManager( ));
    }

    _stackVizDock->setWidget( _stackViz );
    this->addDockWidget( Qt::LeftDockWidgetArea , _stackVizDock );

    connect( _objectInspectorGB , SIGNAL( simDataChanged( )) ,
             _stackViz , SLOT( updateHistograms( )));

    connect( _stackViz , SIGNAL( changedBins(
                                   const unsigned int)) ,
             _summary , SLOT( bins( unsigned int )));

    connect( _ui->actionStackVizShowDataManager , SIGNAL( triggered( bool )) ,
             _stackViz , SLOT( showDisplayManagerWidget( )));

    connect( _ui->actionStackVizShowPanels , SIGNAL( triggered( bool )) ,
             _stackViz , SLOT( showStackVizPanels( bool )));

    connect( _ui->actionStackVizShowDataManager , SIGNAL( triggered( bool )) ,
             _stackViz , SLOT( showDisplayManagerWidget( )));

    connect( _ui->actionStackVizAutoNamingSelections , SIGNAL( triggered( )) ,
             _stackViz , SLOT( toggleAutoNameSelections( )));

    connect( _ui->actionStackVizFillPlots , SIGNAL( triggered( bool )) ,
             _stackViz , SLOT( fillPlots( bool )));

    connect( _ui->actionStackVizFocusOnPlayhead , SIGNAL( triggered( )) ,
             _stackViz , SLOT( focusPlayback( )));

    connect( _ui->actionStackVizFollowPlayHead , SIGNAL( triggered( bool )) ,
             _stackViz , SLOT( followPlayhead( bool )));

    // this avoids making the dock smaller when the stackviz config panels
    // hide.
    QObject::connect( _ui->actionStackVizShowPanels , &QAction::triggered ,
                      [ = ]( bool )
                      {
                        this->resizeDocks( { _stackVizDock } ,
                                           { _stackVizDock->width( ) } ,
                                           Qt::Horizontal );
                      } );
  }

  void MainWindow::_initPlaybackDock( void )
  {
    _simulationDock = new QDockWidget( );
    _simulationDock->setMinimumHeight( 100 );
    _simulationDock->setSizePolicy( QSizePolicy::MinimumExpanding ,
                                    QSizePolicy::MinimumExpanding );

    unsigned int totalHSpan = 20;

    auto content = new QWidget( );
    auto dockLayout = new QGridLayout( );
    content->setLayout( dockLayout );

    _simSlider = new CustomSlider( Qt::Horizontal );
    _simSlider->setMinimum( 0 );
    _simSlider->setMaximum( 1000 );
    _simSlider->setSizePolicy( QSizePolicy::Preferred ,
                               QSizePolicy::Preferred );
    _simSlider->setEnabled( false );

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

    _goToButton = new QPushButton( tr( "Play at..." ));

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

    connect( _playButton , SIGNAL( clicked( )) , this , SLOT( PlayPause( )));

    connect( stopButton , SIGNAL( clicked( )) , this , SLOT( Stop( )));

    connect( nextButton , SIGNAL( clicked( )) , this , SLOT( NextStep( )));

    connect( prevButton , SIGNAL( clicked( )) , this , SLOT( PreviousStep( )));

    connect( _repeatButton , SIGNAL( clicked( )) , this , SLOT( Repeat( )));

    connect( _simSlider , SIGNAL( sliderPressed( )) , this ,
             SLOT( PlayAtPosition( )));

    connect( _goToButton , SIGNAL( clicked( )) , this ,
             SLOT( playAtButtonClicked( )));

    _summary = new visimpl::Summary( nullptr , visimpl::T_STACK_FIXED );
    _summary->setMinimumHeight( 50 );

    dockLayout->addWidget( _summary , 0 , 1 , 2 , totalHSpan - 3 );

    _simulationDock->setWidget( content );
    this->addDockWidget( Qt::BottomDockWidgetArea , _simulationDock );

    connect( _summary , SIGNAL( histogramClicked( float )) , this ,
             SLOT( PlayAtPercentage( float )));

    _simulationDock->setEnabled( false );
  }

  void MainWindow::saveScreenshot()
  {
    QPixmap pixmap(_openGLWidget->size());
    _openGLWidget->render(&pixmap);

    const QIcon icon(":/icons/screenshot.svg");

    SaveScreenshotDialog dialog(pixmap.width(), pixmap.height(), pixmap.toImage(), this);
    dialog.setWindowIcon(icon);
    
    if (dialog.exec() == QDialog::Rejected)
      return;

    auto outputSize = dialog.getSize();

    const auto dialogTitle = tr("Save screenshot");
    const auto problemText = tr("Unable to save screenshot.");
    auto suggestion = tr("ViSimpl-screenshot-%1.png").arg(QDateTime::currentDateTime().toString("yyyy.MM.dd-hh.mm.ss"));
    QString formats = "PNG image (*.png);;BMP image (*.bmp);;JPG image (*.jpg);;JPEG image (*.jpeg)";

    auto fileName = QFileDialog::getSaveFileName(this, dialogTitle, QDir::home().absoluteFilePath(suggestion), formats, nullptr, QFileDialog::DontUseNativeDialog);

    if (!fileName.isEmpty())
    {
      auto nameParts = fileName.split(".");
      auto extension = nameParts.last().toUpper();

      const QStringList validFileExtensions{"BMP", "JPG", "JPEG", "PNG"};

      if (validFileExtensions.contains(extension))
      {
        if (pixmap.size() != outputSize)
          pixmap = pixmap.scaled(outputSize.width(), outputSize.height(), Qt::AspectRatioMode::KeepAspectRatio, Qt::TransformationMode::SmoothTransformation);

        auto saved = pixmap.save(fileName, extension.toUtf8(), 100);

        // check for successful file write
        QFileInfo fileInfo{fileName};
        if (!saved || !fileInfo.exists() || fileInfo.size() == 0)
        {
          auto message = tr("Couln't save screenshot file '%1'. Problem writing format '%2'.").arg(fileInfo.fileName()).arg(extension);

          QMessageBox msgBox(this);
          msgBox.setWindowIcon(icon);
          msgBox.setWindowTitle(dialogTitle);
          msgBox.setText(problemText);
          msgBox.setDetailedText(message);
          msgBox.exec();
        }
      }
      else
      {
        auto message = tr("Couln't save screenshot file '%1'. Unrecognized extension '%1'.").arg(fileName.split('/').last()).arg(extension);

        QMessageBox msgBox(this);
        msgBox.setWindowIcon(icon);
        msgBox.setWindowTitle(dialogTitle);
        msgBox.setText(problemText);
        msgBox.setDetailedText(message);
        msgBox.exec();
      }
    }
  }

void MainWindow::presentationMode()
{
  if (_openGLWidget->parent() != this)
  {
    auto parent = qobject_cast<QDialog*>(_openGLWidget->parent());
    if(parent)
      parent->removeEventFilter(this);

    setCentralWidget(_openGLWidget);
    _openGLWidget->setParent(this);

    if(parent)
    {
      parent->close();
      parent->deleteLater();
    }
  }
  else
  {
    auto presentationDialog = new QDialog(this, Qt::Window | Qt::FramelessWindowHint);
    presentationDialog->setModal(false);
    presentationDialog->installEventFilter(this);
    auto layout = new QHBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSizeConstraint(QLayout::SetNoConstraint);
    presentationDialog->setLayout(layout);
    layout->addWidget(_openGLWidget);
    _openGLWidget->setParent(presentationDialog);
    presentationDialog->showNormal();
    const auto screen = presentationDialog->windowHandle()->screen();
    const auto geom = screen->geometry();
    presentationDialog->move(geom.topLeft());
    presentationDialog->resize(geom.width(), geom.height());
  }
  _openGLWidget->update();
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
  auto presentationDialog = qobject_cast<QDialog *>(obj);

  if (presentationDialog)
  {
    _openGLWidget->camera()->rotate(Eigen::Vector3f(M_PI / 360, 0, 0));
    if (event->type() == QEvent::KeyPress || event->type() == QEvent::ShortcutOverride)
    {
      QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
      const auto modifiers = static_cast<int>(Qt::KeyboardModifiers(Qt::ShiftModifier | Qt::ControlModifier));
      const auto key = keyEvent->key();

      if(key == Qt::Key_Escape) // avoid closing the widget dialog.
      {
        event->accept();
        return true;
      }

      if ((keyEvent->modifiers() & (Qt::ShiftModifier | Qt::ControlModifier)) == modifiers)
      {
        if (key == Qt::Key_F)
        {
          this->presentationMode();
          event->accept();
          return true;
        }

        if (key == Qt::Key_Left || key == Qt::Key_Right || key == Qt::Key_Up)
        {
          const auto screen = presentationDialog->windowHandle()->screen();
          const auto geom = screen->geometry();

          switch (key)
          {
            default:
            case Qt::Key_Up:
              presentationDialog->setMaximumSize(QSize{geom.size()});
              presentationDialog->resize(geom.width(), geom.height());
              presentationDialog->move(geom.topLeft());
              break;
            case Qt::Key_Left:
              presentationDialog->setMaximumSize(QSize{geom.width()/2, geom.height()});
              presentationDialog->resize(geom.width()/2, geom.height());
              presentationDialog->move(geom.topLeft());
              break;
            case Qt::Key_Right:
              presentationDialog->setMaximumSize(QSize{geom.width() / 2, geom.height()});
              presentationDialog->resize(geom.width() / 2, geom.height());
              presentationDialog->move(geom.topLeft() + QPoint(geom.width() / 2, 0));
              break;
            }
            event->accept();
            return true;
        }
      }
    }
  }
  return false;
}

void MainWindow::_initSimControlDock(void)
{
  _simConfigurationDock = new QDockWidget();
  _simConfigurationDock->setMinimumHeight(100);
  _simConfigurationDock->setMinimumWidth(400);
  _simConfigurationDock->setSizePolicy(QSizePolicy::MinimumExpanding,
                                       QSizePolicy::MinimumExpanding);

  _tfWidget = new TransferFunctionWidget();
  _tfWidget->setMinimumHeight(100);
  _tfWidget->setDialogIcon(QIcon(":/visimpl.png"));

  _subsetImporter = new SubsetImporter(this);
  _subsetImporter->setWindowModality(Qt::WindowModal);
  _subsetImporter->setMinimumHeight(300);
  _subsetImporter->setMinimumWidth(500);

  _selectionManager = new SelectionManagerWidget();
  _selectionManager->setWindowModality(Qt::WindowModal);
  _selectionManager->setMinimumHeight(300);
  _selectionManager->setMinimumWidth(500);
  _selectionManager->setWindowIcon(QIcon(":/visimpl.png"));

  connect(_selectionManager, SIGNAL(selectionChanged(void)), this,
          SLOT(selectionManagerChanged(void)));

  _deltaTimeBox = new QDoubleSpinBox();
  _deltaTimeBox->setMinimum(0.00000001);
  _deltaTimeBox->setMaximum(50);
  _deltaTimeBox->setSingleStep(0.05);
  _deltaTimeBox->setDecimals(5);
  _deltaTimeBox->setMaximumWidth(100);

  _timeStepsPSBox = new QDoubleSpinBox();
  _timeStepsPSBox->setMinimum(0.00000001);
  _timeStepsPSBox->setMaximum(50);
  _timeStepsPSBox->setSingleStep(1.0);
  _timeStepsPSBox->setDecimals(5);
  _timeStepsPSBox->setMaximumWidth(100);

  _stepByStepDurationBox = new QDoubleSpinBox();
  _stepByStepDurationBox->setMinimum(0.5);
  _stepByStepDurationBox->setMaximum(50);
  _stepByStepDurationBox->setSingleStep(1.0);
  _stepByStepDurationBox->setDecimals(3);
  _stepByStepDurationBox->setMaximumWidth(100);

  _decayBox = new QDoubleSpinBox();
  _decayBox->setMinimum(0.01);
  _decayBox->setMaximum(600.0);
  _decayBox->setDecimals(5);
  _decayBox->setMaximumWidth(100);

  _alphaNormalButton = new QRadioButton("Normal");
  _alphaAccumulativeButton = new QRadioButton("Accumulative");
  _openGLWidget->SetAlphaBlendingAccumulative(false);

  _buttonClearSelection = new QPushButton("Discard");
  _buttonClearSelection->setEnabled(false);
  _selectionSizeLabel = new QLabel("0");

  _buttonAddGroup = new QPushButton("Add group");
  _buttonAddGroup->setEnabled(false);
  _buttonAddGroup->setToolTip(
      "Click to create a group from current selection.");

  _labelGID = new QLabel("");
  _labelPosition = new QLabel("");

  _checkClipping = new QCheckBox(tr("Clipping"));
  _checkShowPlanes = new QCheckBox("Show planes");
  _buttonResetPlanes = new QPushButton("Reset");
  _spinBoxClippingHeight = new QDoubleSpinBox();
  _spinBoxClippingHeight->setDecimals(2);
  _spinBoxClippingHeight->setMinimum(1);
  _spinBoxClippingHeight->setMaximum(99999);

  _spinBoxClippingWidth = new QDoubleSpinBox();
  _spinBoxClippingWidth->setDecimals(2);
  _spinBoxClippingWidth->setMinimum(1);
  _spinBoxClippingWidth->setMaximum(99999);

  _spinBoxClippingDist = new QDoubleSpinBox();
  _spinBoxClippingDist->setDecimals(2);
  _spinBoxClippingDist->setMinimum(1);
  _spinBoxClippingDist->setMaximum(99999);

  QColor clippingColor(255, 255, 255, 255);
  _frameClippingColor = new QPushButton();
  _frameClippingColor->setStyleSheet("background-color: " +
                                     clippingColor.name());
  _frameClippingColor->setMinimumSize(20, 20);
  _frameClippingColor->setMaximumSize(20, 20);
  _frameClippingColor->setProperty(PLANES_COLOR_KEY_,
                                   clippingColor.name());

  _buttonSelectionFromClippingPlanes = new QPushButton("To selection");
  _buttonSelectionFromClippingPlanes->setToolTip(
      tr("Create a selection set from elements between planes"));

  _circuitScaleX = new QDoubleSpinBox();
  _circuitScaleX->setDecimals(2);
  _circuitScaleX->setMinimum(0.01);
  _circuitScaleX->setMaximum(9999999);
  _circuitScaleX->setSingleStep(0.5);
  _circuitScaleX->setMaximumWidth(100);

  _circuitScaleY = new QDoubleSpinBox();
  _circuitScaleY->setDecimals(2);
  _circuitScaleY->setMinimum(0.01);
  _circuitScaleY->setMaximum(9999999);
  _circuitScaleY->setSingleStep(0.5);
  _circuitScaleY->setMaximumWidth(100);

  _circuitScaleZ = new QDoubleSpinBox();
  _circuitScaleZ->setDecimals(2);
  _circuitScaleZ->setMinimum(0.01);
  _circuitScaleZ->setMaximum(9999999);
  _circuitScaleZ->setSingleStep(0.5);
  _circuitScaleZ->setMaximumWidth(100);

  QWidget *topContainer = new QWidget();
  QVBoxLayout *verticalLayout = new QVBoxLayout();

  _groupBoxTransferFunction =
      new QGroupBox("Color and Size transfer function");
  QVBoxLayout *tfLayout = new QVBoxLayout();
  tfLayout->addWidget(_tfWidget);
  _groupBoxTransferFunction->setLayout(tfLayout);

  QGroupBox *tSpeedGB = new QGroupBox("Simulation playback Configuration");
  QGridLayout *sfLayout = new QGridLayout();
  sfLayout->setAlignment(Qt::AlignTop);
  sfLayout->addWidget(new QLabel("Simulation timestep:"), 0, 0, 1, 1);
  sfLayout->addWidget(_deltaTimeBox, 0, 1, 1, 1);
  sfLayout->addWidget(new QLabel("Timesteps per second:"), 1, 0, 1,
                      1);
  sfLayout->addWidget(_timeStepsPSBox, 1, 1, 1, 1);
  sfLayout->addWidget(new QLabel("Step playback duration (s):"), 2, 0,
                      1,
                      1);
  sfLayout->addWidget(_stepByStepDurationBox, 2, 1, 1, 1);
  tSpeedGB->setLayout(sfLayout);

  QGroupBox *scaleGB = new QGroupBox("Scale factor (X,Y,Z)");
  QHBoxLayout *layoutScale = new QHBoxLayout();
  scaleGB->setLayout(layoutScale);
  layoutScale->addWidget(_circuitScaleX);
  layoutScale->addWidget(_circuitScaleY);
  layoutScale->addWidget(_circuitScaleZ);

  QComboBox *comboShader = new QComboBox();
  comboShader->addItems({"Default", "Solid"});
  comboShader->setCurrentIndex(0);

  QGroupBox *shaderGB = new QGroupBox("Shader Configuration");
  QHBoxLayout *shaderLayout = new QHBoxLayout();
  shaderLayout->addWidget(new QLabel("Current shader: "));
  shaderLayout->addWidget(comboShader);
  shaderGB->setLayout(shaderLayout);

  QGroupBox *dFunctionGB = new QGroupBox("Decay function");
  QHBoxLayout *dfLayout = new QHBoxLayout();
  dfLayout->setAlignment(Qt::AlignTop);
  dfLayout->addWidget(new QLabel("Decay (simulation time): "));
  dfLayout->addWidget(_decayBox);
  dFunctionGB->setLayout(dfLayout);

  QGroupBox *rFunctionGB = new QGroupBox("Alpha blending function");
  QHBoxLayout *rfLayout = new QHBoxLayout();
  rfLayout->setAlignment(Qt::AlignTop);
  rfLayout->addWidget(new QLabel("Alpha Blending: "));
  rfLayout->addWidget(_alphaNormalButton);
  rfLayout->addWidget(_alphaAccumulativeButton);
  rFunctionGB->setLayout(rfLayout);

  // Visual Configuration Container
  QWidget *vcContainer = new QWidget();
  QVBoxLayout *vcLayout = new QVBoxLayout();
  vcLayout->setAlignment(Qt::AlignTop);
  vcLayout->addWidget(scaleGB);
  vcLayout->addWidget(shaderGB);
  vcLayout->addWidget(dFunctionGB);
  vcLayout->addWidget(rFunctionGB);
  vcContainer->setLayout(vcLayout);

  QPushButton *buttonSelectionManager = new QPushButton("...");
  buttonSelectionManager->setToolTip(
      tr("Show the selection management dialog"));
  buttonSelectionManager->setMaximumWidth(30);

  QGroupBox *selFunctionGB = new QGroupBox("Current selection");
  QHBoxLayout *selLayout = new QHBoxLayout();
  selLayout->setAlignment(Qt::AlignTop);
  selLayout->addWidget(new QLabel("Size: "));
  selLayout->addWidget(_selectionSizeLabel);
  selLayout->addWidget(buttonSelectionManager);
  selLayout->addWidget(_buttonAddGroup);
  selLayout->addWidget(_buttonClearSelection);
  selFunctionGB->setLayout(selLayout);

  QFrame *line = new QFrame();
  line->setFrameShape(QFrame::HLine);
  line->setFrameShadow(QFrame::Sunken);

  QGroupBox *gbClippingPlanes = new QGroupBox("Clipping planes");
  QGridLayout *layoutClippingPlanes = new QGridLayout();
  gbClippingPlanes->setLayout(layoutClippingPlanes);
  layoutClippingPlanes->addWidget(_checkClipping, 0, 0, 1, 1);
  layoutClippingPlanes->addWidget(_checkShowPlanes, 0, 1, 1, 2);
  layoutClippingPlanes->addWidget(_buttonSelectionFromClippingPlanes, 0,
                                  3,
                                  1, 1);
  layoutClippingPlanes->addWidget(line, 1, 0, 1, 4);
  layoutClippingPlanes->addWidget(_frameClippingColor, 2, 0, 1, 1);
  layoutClippingPlanes->addWidget(_buttonResetPlanes, 2, 1, 1, 1);
  layoutClippingPlanes->addWidget(new QLabel("Height"), 2, 2, 1, 1);
  layoutClippingPlanes->addWidget(_spinBoxClippingHeight, 2, 3, 1, 1);
  layoutClippingPlanes->addWidget(new QLabel("Distance"), 3, 0, 1, 1);
  layoutClippingPlanes->addWidget(_spinBoxClippingDist, 3, 1, 1, 1);
  layoutClippingPlanes->addWidget(new QLabel("Width"), 3, 2, 1, 1);
  layoutClippingPlanes->addWidget(_spinBoxClippingWidth, 3, 3, 1, 1);

  QWidget *containerSelectionTools = new QWidget();
  QVBoxLayout *layoutContainerSelection = new QVBoxLayout();
  containerSelectionTools->setLayout(layoutContainerSelection);

  layoutContainerSelection->addWidget(selFunctionGB);
  layoutContainerSelection->addWidget(gbClippingPlanes);

  _objectInspectorGB = new DataInspector("Object inspector");
  _objectInspectorGB->addWidget(new QLabel("GID:"), 2, 0, 1, 1);
  _objectInspectorGB->addWidget(_labelGID, 2, 1, 1, 3);
  _objectInspectorGB->addWidget(new QLabel("Position: "), 3, 0, 1, 1);
  _objectInspectorGB->addWidget(_labelPosition, 3, 1, 1, 3);

  QGroupBox *groupBoxGroups = new QGroupBox("Current visualization groups");
  _groupLayout = new QVBoxLayout();
  _groupLayout->setAlignment(Qt::AlignTop);

  _buttonImportGroups = new QPushButton("Import from...");
  _buttonClearGroups = new QPushButton("Clear");
  _buttonClearGroups->setEnabled(false);
  _buttonLoadGroups = new QPushButton("Load");
  _buttonLoadGroups->setToolTip(tr("Load Groups from disk"));
  _buttonSaveGroups = new QPushButton("Save");
  _buttonSaveGroups->setToolTip(tr("Save Groups to disk"));
  _buttonSaveGroups->setEnabled(false);

  {
    QWidget *groupContainer = new QWidget();
    groupContainer->setLayout(_groupLayout);

    QScrollArea *scrollGroups = new QScrollArea();
    scrollGroups->setWidget(groupContainer);
    scrollGroups->setWidgetResizable(true);
    scrollGroups->setFrameShape(QFrame::Shape::NoFrame);
    scrollGroups->setFrameShadow(QFrame::Shadow::Plain);
    scrollGroups->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollGroups->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    QGridLayout *groupOuterLayout = new QGridLayout();
    groupOuterLayout->setMargin(0);
    groupOuterLayout->addWidget(_buttonImportGroups, 0, 0, 1, 1);
    groupOuterLayout->addWidget(_buttonClearGroups, 0, 1, 1, 1);
    groupOuterLayout->addWidget(_buttonLoadGroups, 1, 0, 1, 1);
    groupOuterLayout->addWidget(_buttonSaveGroups, 1, 1, 1, 1);

    groupOuterLayout->addWidget(scrollGroups, 2, 0, 1, 2);

    groupBoxGroups->setLayout(groupOuterLayout);
  }

  _groupBoxAttrib = new QGroupBox("Attribute Mapping");
  QGridLayout *layoutGroupAttrib = new QGridLayout();
  _groupBoxAttrib->setLayout(layoutGroupAttrib);

  _comboAttribSelection = new QComboBox();

  QGroupBox *gbAttribSel = new QGroupBox("Attribute selection");
  QHBoxLayout *lyAttribSel = new QHBoxLayout();
  lyAttribSel->addWidget(_comboAttribSelection);
  gbAttribSel->setLayout(lyAttribSel);

  QGroupBox *gbAttribStats = new QGroupBox("Statistics");
  QScrollArea *attribScroll = new QScrollArea();
  QWidget *attribContainer = new QWidget();
  QVBoxLayout *attribContainerLayout = new QVBoxLayout();
  gbAttribStats->setLayout(attribContainerLayout);
  attribContainerLayout->addWidget(attribScroll);

  attribScroll->setWidget(attribContainer);
  attribScroll->setWidgetResizable(true);
  attribScroll->setFrameShape(QFrame::Shape::NoFrame);
  attribScroll->setFrameShadow(QFrame::Shadow::Plain);
  attribScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  attribScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

  _layoutAttribStats = new QVBoxLayout();
  _layoutAttribStats->setAlignment(Qt::AlignTop);
  attribContainer->setLayout(_layoutAttribStats);

  QGroupBox *gbAttribGroups = new QGroupBox("Groups");
  _layoutAttribGroups = new QGridLayout();
  _layoutAttribGroups->setAlignment(Qt::AlignTop);
  {
    QWidget *groupContainer = new QWidget();
    groupContainer->setLayout(_layoutAttribGroups);

    QScrollArea *groupScroll = new QScrollArea();
    groupScroll->setWidget(groupContainer);
    groupScroll->setWidgetResizable(true);
    groupScroll->setFrameShape(QFrame::Shape::NoFrame);
    groupScroll->setFrameShadow(QFrame::Shadow::Plain);
    groupScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    groupScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    QGridLayout *groupOuterLayout = new QGridLayout();
    groupOuterLayout->setMargin(0);
    groupOuterLayout->addWidget(groupScroll);

    gbAttribGroups->setLayout(groupOuterLayout);
  }

  layoutGroupAttrib->addWidget(gbAttribSel, 0, 0, 1, 2);
  layoutGroupAttrib->addWidget(gbAttribGroups, 0, 2, 3, 2);
  layoutGroupAttrib->addWidget(gbAttribStats, 1, 0, 2, 2);

  QWidget *containerTabSelection = new QWidget();
  QVBoxLayout *tabSelectionLayout = new QVBoxLayout();
  containerTabSelection->setLayout(tabSelectionLayout);
  tabSelectionLayout->addWidget(_groupBoxTransferFunction);

  QWidget *containerTabGroups = new QWidget();
  QVBoxLayout *tabGroupsLayout = new QVBoxLayout();
  containerTabGroups->setLayout(tabGroupsLayout);
  tabGroupsLayout->addWidget(groupBoxGroups);

  QWidget *containerTabAttrib = new QWidget();
  QVBoxLayout *tabAttribLayout = new QVBoxLayout();
  containerTabAttrib->setLayout(tabAttribLayout);
  tabAttribLayout->addWidget(_groupBoxAttrib);

  _modeSelectionWidget = new QTabWidget();
  _modeSelectionWidget->addTab(containerTabSelection, tr("Selection"));
  _modeSelectionWidget->addTab(containerTabGroups, tr("Groups"));
  _modeSelectionWidget->addTab(containerTabAttrib, tr("Attribute"));

  _toolBoxOptions = new QToolBox();
  _toolBoxOptions->addItem(tSpeedGB, tr("Playback Configuration"));
  _toolBoxOptions->addItem(vcContainer, tr("Visual Configuration"));
  _toolBoxOptions->addItem(containerSelectionTools, tr("Selection"));
  _toolBoxOptions->addItem(_objectInspectorGB, tr("Inspector"));

  connect(_objectInspectorGB, SIGNAL(simDataChanged()),
          _openGLWidget, SLOT(updateData()));

  connect(_objectInspectorGB, SIGNAL(simDataChanged()),
          _summary, SLOT(UpdateHistograms()));

  connect(_objectInspectorGB, SIGNAL(simDataChanged()),
          this, SLOT(configureComponents()));

  verticalLayout->setAlignment(Qt::AlignTop);
  verticalLayout->addWidget(_modeSelectionWidget);
  verticalLayout->addWidget(_toolBoxOptions);

  topContainer->setLayout(verticalLayout);
  _simConfigurationDock->setWidget(topContainer);

  this->addDockWidget(Qt::RightDockWidgetArea, _simConfigurationDock);

  connect(_modeSelectionWidget, SIGNAL(currentChanged(int)),
          _openGLWidget, SLOT(setMode(int)));

  connect(_openGLWidget, SIGNAL(attributeStatsComputed(void)), this,
          SLOT(updateAttributeStats(void)));

  connect(_comboAttribSelection, SIGNAL(currentIndexChanged(int)),
          _openGLWidget, SLOT(selectAttrib(int)));

  connect(comboShader, SIGNAL(currentIndexChanged(int)), _openGLWidget,
          SLOT(changeShader(int)));

  connect(_tfWidget, SIGNAL(colorChanged(void)), this,
          SLOT(UpdateSimulationColorMapping(void)));
  connect(_tfWidget, SIGNAL(sizeChanged(void)), this,
          SLOT(UpdateSimulationSizeFunction(void)));
  connect(_tfWidget, SIGNAL(previewColor(void)), this,
          SLOT(PreviewSimulationColorMapping(void)));
  connect(_tfWidget, SIGNAL(previewColor(void)), this,
          SLOT(PreviewSimulationSizeFunction(void)));

  connect(buttonSelectionManager, SIGNAL(clicked(void)), this,
          SLOT(dialogSelectionManagement(void)));

  connect(_circuitScaleX, SIGNAL(valueChanged(double)), this,
          SLOT(updateCircuitScaleValue(void)));

  connect(_circuitScaleY, SIGNAL(valueChanged(double)), this,
          SLOT(updateCircuitScaleValue(void)));

  connect(_circuitScaleZ, SIGNAL(valueChanged(double)), this,
          SLOT(updateCircuitScaleValue(void)));

  connect(_deltaTimeBox, SIGNAL(valueChanged(double)), this,
          SLOT(updateSimDeltaTime(void)));

  connect(_timeStepsPSBox, SIGNAL(valueChanged(double)), this,
          SLOT(updateSimTimestepsPS(void)));

  connect(_decayBox, SIGNAL(valueChanged(double)), this,
          SLOT(updateSimulationDecayValue(void)));

  connect(_stepByStepDurationBox, SIGNAL(valueChanged(double)), this,
          SLOT(updateSimStepByStepDuration(void)));

  connect(_alphaNormalButton, SIGNAL(toggled(bool)), this,
          SLOT(AlphaBlendingToggled(void)));

  // Clipping planes

  _checkClipping->setChecked(true);
  connect(_checkClipping, SIGNAL(stateChanged(int)), this,
          SLOT(clippingPlanesActive(int)));
  _checkClipping->setChecked(false);

  _checkShowPlanes->setChecked(true);
  connect(_checkShowPlanes, SIGNAL(stateChanged(int)), _openGLWidget,
          SLOT(paintClippingPlanes(int)));

  connect(_buttonSelectionFromClippingPlanes, SIGNAL(clicked(void)),
          this, SLOT(selectionFromPlanes(void)));

  connect(_buttonResetPlanes, SIGNAL(clicked(void)), this,
          SLOT(clippingPlanesReset(void)));

  connect(_spinBoxClippingDist, SIGNAL(editingFinished(void)), this,
          SLOT(spinBoxValueChanged(void)));

  connect(_spinBoxClippingHeight, SIGNAL(editingFinished(void)), this,
          SLOT(spinBoxValueChanged(void)));

  connect(_spinBoxClippingWidth, SIGNAL(editingFinished(void)), this,
          SLOT(spinBoxValueChanged(void)));

  connect(_frameClippingColor, SIGNAL(clicked()), this,
          SLOT(colorSelectionClicked()));

  connect(_buttonClearSelection, SIGNAL(clicked(void)), this,
          SLOT(clearSelection(void)));

  connect(_buttonAddGroup, SIGNAL(clicked(void)), this,
          SLOT(addGroupFromSelection()));

  connect(_buttonImportGroups, SIGNAL(clicked(void)), this,
          SLOT(dialogSubsetImporter(void)));

  connect(_buttonClearGroups, SIGNAL(clicked(void)), this,
          SLOT(clearGroups(void)));

  connect(_buttonLoadGroups, SIGNAL(clicked(void)), this,
          SLOT(loadGroups()));
  connect(_buttonSaveGroups, SIGNAL(clicked(void)), this,
          SLOT(saveGroups()));

  _alphaNormalButton->setChecked(true);
  _simConfigurationDock->setEnabled(false);
}

  void MainWindow::_initSummaryWidget( void )
  {
    const auto simType = _openGLWidget->player( )->simulationType( );

    if ( simType == simil::TSimSpikes )
    {
      simil::SpikesPlayer* spikesPlayer =
        dynamic_cast< simil::SpikesPlayer* >( _openGLWidget->player( ));

      _summary->Init( spikesPlayer->data( ));

      _summary->simulationPlayer( _openGLWidget->player( ));
      _summary->show( );
    }
  }

  void MainWindow::UpdateSimulationSlider( float percentage )
  {
    if ( !_openGLWidget || !_openGLWidget->player( ))
      return;

    _startTimeLabel->setText(
      QString::number( _openGLWidget->currentTime( ) , 'f' , 3 ));
    _endTimeLabel->setText(
      QString::number( _openGLWidget->player( )->endTime( ) , 'f' , 3 ));

    const int total = _simSlider->maximum( ) - _simSlider->minimum( );
    const int position = ( percentage * total ) + _simSlider->minimum( );

    _simSlider->setSliderPosition( position );

    if ( _summary )
      _summary->repaintHistograms( );

    _stackViz->repaintHistograms( );
  }

  void MainWindow::UpdateSimulationColorMapping( void )
  {
    _openGLWidget->changeSimulationColorMapping( _tfWidget->getColors( ));
  }

  void MainWindow::PreviewSimulationColorMapping( void )
  {
    _openGLWidget->changeSimulationColorMapping(
      _tfWidget->getPreviewColors( ));
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
    _openGLWidget->changeSimulationSizeFunction(
      _tfWidget->getSizeFunction( ));
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

  void MainWindow::setCircuitSizeScaleFactor( vec3 scaleFactor )
  {
    _circuitScaleX->blockSignals( true );
    _circuitScaleY->blockSignals( true );
    _circuitScaleZ->blockSignals( true );

    _circuitScaleX->setValue( scaleFactor.x );
    _circuitScaleY->setValue( scaleFactor.y );
    _circuitScaleZ->setValue( scaleFactor.z );

    _circuitScaleX->blockSignals( true );
    _circuitScaleY->blockSignals( true );
    _circuitScaleZ->blockSignals( true );

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
    vec3 scale{ _circuitScaleX->value( ) , _circuitScaleY->value( ) ,
                _circuitScaleZ->value( ) };
    _openGLWidget->circuitScaleFactor( scale );
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
    _stepByStepDurationBox->setValue(
      _openGLWidget->simulationStepByStepDuration( ));
  }

  void MainWindow::updateSimStepByStepDuration( void )
  {
    _openGLWidget->simulationStepByStepDuration(
      _stepByStepDurationBox->value( ));
  }

  void MainWindow::AlphaBlendingToggled( void )
  {
    if ( _alphaNormalButton->isChecked( ))
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
    while (( item = _layoutAttribStats->takeAt( 0 )))
    {
      _layoutAttribStats->removeWidget( item->widget( ));
      delete item->widget( );
    }

    const auto stats = _domainManager->getAttributeClusters( );

    for ( const auto& group: stats )
    {
      _layoutAttribStats->addWidget( new QLabel(
        QString::fromStdString( group.first )));
    }

    while (( item = _layoutAttribGroups->takeAt( 0 )))
    {
      _layoutAttribGroups->removeWidget( item->widget( ));
      delete item->widget( );
    }

    _attribGroupsVisButtons.clear( );

    unsigned int currentIndex = 0;
    for ( const auto& entry: stats )
    {
      auto& group = entry.second;

      QFrame* frame = new QFrame( );
      frame->setStyleSheet( "background-color: " +
                            group->colorMapping( ).at( 0 ).second.name( ));
      frame->setMinimumSize( 20 , 20 );
      frame->setMaximumSize( 20 , 20 );

      QCheckBox* buttonVisibility = new QCheckBox( group->name( ).c_str( ));
      buttonVisibility->setChecked( true );

      _attribGroupsVisButtons.push_back( buttonVisibility );

      connect( buttonVisibility , &QCheckBox::clicked ,
               [ group ]( bool active )
               {
                 group->active( active );
               } );

      _layoutAttribGroups->addWidget( frame , currentIndex , 0 , 1 , 1 );
      _layoutAttribGroups->addWidget( buttonVisibility , currentIndex , 2 , 1 ,
                                      1 );

      ++currentIndex;
    }

    _layoutAttribGroups->update( );
  }

  void MainWindow::updateSelectedStatsPickingSingle( unsigned int /*selected*/ )
  {
    // TODO
    /*
    auto pickingInfo = _domainManager->pickingInfoSimple( selected );

    _labelGID->setText(
      QString::number( std::get< T_PART_GID >( pickingInfo )));

    auto position = std::get< T_PART_POSITION >( pickingInfo );

    QString posText = "( " + QString::number( position.x ) + ", " +
                      QString::number( position.y ) + ", " +
                      QString::number( position.z ) + " )";

    _labelPosition->setText( posText );

    _toolBoxOptions->setCurrentIndex(
      static_cast<unsigned int>( T_TOOL_Inpector ));
    */
  }

  void MainWindow::clippingPlanesReset( void )
  {
    _openGLWidget->clippingPlanesReset( );

    _resetClippingParams( );
  }

  void MainWindow::_resetClippingParams( void )
  {
    _spinBoxClippingDist->setValue( _openGLWidget->clippingPlanesDistance( ));
    _spinBoxClippingHeight->setValue( _openGLWidget->clippingPlanesHeight( ));
    _spinBoxClippingWidth->setValue( _openGLWidget->clippingPlanesWidth( ));
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
    auto spinBox = dynamic_cast< QDoubleSpinBox* >( sender( ));

    if ( spinBox == _spinBoxClippingDist )
      _openGLWidget->clippingPlanesDistance( spinBox->value( ));
    else if ( spinBox == _spinBoxClippingHeight )
      _openGLWidget->clippingPlanesHeight( spinBox->value( ));
    else if ( spinBox == _spinBoxClippingWidth )
      _openGLWidget->clippingPlanesWidth( spinBox->value( ));
  }

  void MainWindow::showInactive( bool show )
  {
    _openGLWidget->showInactive( show );
  }

  void MainWindow::playAtButtonClicked( void )
  {
    if ( !_openGLWidget || !_openGLWidget->player( ))
      return;

    bool ok;
    auto player = _openGLWidget->player( );
    double result = QInputDialog::getDouble(
      this , tr( "Set simulation time to play:" ) , tr( "Simulation time" ) ,
      static_cast<double>(_openGLWidget->currentTime( )) ,
      static_cast<double>(player->data( )->startTime( )) ,
      static_cast<double>(player->data( )->endTime( )) , 3 , &ok ,
      Qt::Popup );

    if ( ok )
    {
      float percentage = ( result - player->startTime( )) /
                         ( player->endTime( ) - player->startTime( ));

      percentage = std::max( 0.0f , std::min( 1.0f , percentage ));

      PlayAtPercentage( percentage , true );
    }
  }

  bool MainWindow::_showDialog( QColor& current , const QString& message )
  {
    QColor result = QColorDialog::getColor( current , this ,
                                            QString( message ) ,
                                            QColorDialog::DontUseNativeDialog );

    if ( result.isValid( ))
    {
      current = result;
      return true;
    }
    else
      return false;
  }

  void MainWindow::colorSelectionClicked( void )
  {
    QPushButton* button = dynamic_cast< QPushButton* >( sender( ));

    if ( button == _frameClippingColor )
    {
      QColor current = _openGLWidget->clippingPlanesColor( );
      if ( _showDialog( current , "Select planes color" ))
      {
        _openGLWidget->clippingPlanesColor( current );
      }
    }
  }

  void MainWindow::selectionFromPlanes( void )
  {
    if ( !_openGLWidget )
      return;

    auto ids = _openGLWidget->getPlanesContainedElements( );
    visimpl::GIDUSet selectedSet( ids.begin( ) , ids.end( ));

    if ( selectedSet.empty( ))
      return;

    setSelection( selectedSet , SRC_PLANES );
  }

  void MainWindow::selectionManagerChanged( void )
  {
    setSelection( _selectionManager->selected( ) , SRC_WIDGET );
  }

  void MainWindow::_updateSelectionGUI( void )
  {
    auto selection = _domainManager->getSelectionCluster( );

    _buttonAddGroup->setEnabled( selection->size( ) != 0 );
    _buttonClearSelection->setEnabled( selection->size( ) != 0 );
    _selectionSizeLabel->setText( QString::number( selection->size( )));
    _selectionSizeLabel->update( );
  }

  void MainWindow::setSelection( const GIDUSet& selectedSet ,
                                 TSelectionSource source_ )
  {
    if ( source_ == SRC_UNDEFINED )
      return;

    _domainManager->setSelection( selectedSet ,
                                  _openGLWidget->getGidPositions( ));
    _openGLWidget->setSelectedGIDs( selectedSet );

    if ( source_ != SRC_WIDGET )
      _selectionManager->setSelected( selectedSet );

    _updateSelectionGUI( );
  }

  void MainWindow::clearSelection( void )
  {
    if ( _openGLWidget )
    {
      _domainManager->setSelection( _openGLWidget->player( )->gids( ) ,
                                    _openGLWidget->getGidPositions( ));
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
      static_cast<zeroeq::gmrv::PlaybackOperation>(playbackOp);

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

  void
  MainWindow::_onSelectionEvent( lexis::data::ConstSelectedIDsPtr selected )
  {
    if ( _openGLWidget )
    {
      std::vector< uint32_t > ids = selected->getIdsVector( );

      visimpl::GIDUSet selectedSet( ids.begin( ) , ids.end( ));

      setSelection( selectedSet , SRC_EXTERNAL );

      if ( _ui->actionAddZeroEQhistograms->isChecked( ))
      {
        addGroupFromSelection( );
      }
    }
  }

#endif

  void MainWindow::addGroupControls( std::shared_ptr< VisualGroup > group ,
                                     unsigned int currentIndex ,
                                     unsigned int size ,
                                     QColor groupColor )
  {
    QWidget* container = new QWidget( );
    auto itemLayout = new QHBoxLayout( container );
    container->setLayout( itemLayout );
    container->setProperty( GROUP_NAME_ ,
                            QString::fromStdString( group->name( )));

    if ( groupColor == QColor{ 0 , 0 , 0 } )
    {
      const auto colors = _openGLWidget->colorPalette( ).colors( );
      const auto paletteIdx = currentIndex % colors.size( );
      const auto color = colors[ paletteIdx ].toRgb( );
      const auto variations = generateColorPair( color );

      TTransferFunction colorVariation;
      colorVariation.push_back( std::make_pair( 0.0f , variations.first ));
      colorVariation.push_back( std::make_pair( 1.0f , variations.second ));
      group->colorMapping( colorVariation );
    }
    else
    {
      const auto variations = generateColorPair( groupColor );

      TTransferFunction colorVariation;
      colorVariation.push_back( std::make_pair( 0.0f , variations.first ));
      colorVariation.push_back( std::make_pair( 1.0f , variations.second ));
      group->colorMapping( colorVariation );
    }

    auto tfWidget = new TransferFunctionWidget( container );
    tfWidget->setColorPoints( group->colorMapping( ));
    tfWidget->setSizeFunction( group->sizeFunction( ));
    tfWidget->setDialogIcon( QIcon( ":/visimpl.png" ));
    tfWidget->setProperty( GROUP_NAME_ ,
                           QString::fromStdString( group->name( )));

    itemLayout->addWidget( tfWidget );

    const auto presetName = QString( "Group selection %1" ).arg( currentIndex );
    QGradientStops stops;

    const auto mapping = group->colorMapping( );
    auto addColorMapping = [ &stops ]( const visimpl::TTFColor& c )
    { stops << qMakePair( c.first , c.second ); };
    std::for_each( mapping.cbegin( ) , mapping.cend( ) , addColorMapping );

    tfWidget->addPreset( TransferFunctionWidget::Preset( presetName , stops ));

    connect( tfWidget , SIGNAL( colorChanged( )) ,
             this , SLOT( onGroupColorChanged( )));
    connect( tfWidget , SIGNAL( sizeChanged( )) ,
             this , SLOT( onGroupColorChanged( )));
    connect( tfWidget , SIGNAL( previewColor( )) ,
             this , SLOT( onGroupPreview( )));

    QCheckBox* visibilityCheckbox = new QCheckBox( "active" );
    visibilityCheckbox->setChecked( group->active( ));

    connect( visibilityCheckbox , SIGNAL( clicked( )) , this ,
             SLOT( checkGroupsVisibility( )));

    QString numberText = QString( "# " ).append( QString::number( size ));

    auto nameButton = new QPushButton( group->name( ).c_str( ));
    nameButton->setFlat( true );
    nameButton->setObjectName( "nameButton" );
    nameButton->setProperty( GROUP_NAME_ ,
                             QString::fromStdString( group->name( )));

    connect( nameButton , SIGNAL( clicked( )) , this ,
             SLOT( onGroupNameClicked( )));

    auto deleteButton = new QPushButton( QIcon( ":/icons/close.svg" ) , "" );
    deleteButton->setProperty( GROUP_NAME_ ,
                               QString::fromStdString( group->name( )));

    connect( deleteButton , SIGNAL( clicked( )) , this ,
             SLOT( onGroupDeleteClicked( )));

    auto firstLayout = new QHBoxLayout( );
    firstLayout->setMargin( 0 );
    firstLayout->addWidget( nameButton , 1 );
    firstLayout->addWidget( deleteButton , 0 );

    auto layout = new QVBoxLayout( );
    layout->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
    layout->addLayout( firstLayout );
    layout->addWidget( visibilityCheckbox );
    layout->addWidget( new QLabel( numberText ));

    itemLayout->insertLayout( 1 , layout , 0 );
    itemLayout->setSizeConstraint( QLayout::SizeConstraint::SetMinimumSize );

    _groupsVisButtons.push_back(
      std::make_tuple( container , visibilityCheckbox ));

    _groupLayout->addWidget( container );

    _buttonClearGroups->setEnabled( true );
    _buttonSaveGroups->setEnabled( true );
  }

  void MainWindow::Stop( bool notify )
  {
    if ( !_openGLWidget || !_openGLWidget->player( ))
      return;

    if ( _openGLWidget )
    {
      _openGLWidget->Stop( );
      _playButton->setIcon( _playIcon );
      _startTimeLabel->setText(
        QString::number( _openGLWidget->player( )->startTime( ) , 'f' , 3 ));

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
    while ( !_groupsVisButtons.empty( ))
    {
      auto container = std::get< 0 >( _groupsVisButtons.at( 0 ));
      auto groupName = container->property( GROUP_NAME_ ).toString( );

      removeVisualGroup( groupName );
    }
  }

  void MainWindow::importVisualGroups( void )
  {
    const auto& groups = _subsetImporter->selectedSubsets( );

    const auto& allGIDs = _openGLWidget->player( )->gids( );

    auto addGroup = [ this , &allGIDs ]( const std::string& groupName )
    {
      const auto subset = _subsetEvents->getSubset( groupName );

      GIDUSet filteredGIDs;
      auto filterGIDs = [ &filteredGIDs , &allGIDs ]( const uint32_t gid )
      {
        if ( allGIDs.find( gid ) != allGIDs.end( ))
          filteredGIDs.insert( gid );
      };
      std::for_each( subset.cbegin( ) , subset.cend( ) , filterGIDs );

      _openGLWidget->makeCurrent( );
      const auto group = _domainManager->createGroup(
        filteredGIDs , _openGLWidget->getGidPositions( ) , groupName );

      const auto color = _subsetEvents->getSubsetColor( groupName );
      QColor groupColor = QColor::fromRgbF( color.x( ) , color.y( ) ,
                                            color.z( ));

      addGroupControls( group , _domainManager->getGroupAmount( ) - 1 ,
                        filteredGIDs.size( ) , groupColor );

      visimpl::Selection selection;
      selection.name = groupName;
      selection.gids = filteredGIDs;
      _stackViz->addSelection( selection );
    };
    std::for_each( groups.cbegin( ) , groups.cend( ) , addGroup );

    _openGLWidget->subsetEventsManager( _subsetEvents );
    _openGLWidget->showEventsActivityLabels(
      _ui->actionShowEventsActivity->isChecked( ));

    _buttonImportGroups->setEnabled( groups.empty( ));
    _buttonClearGroups->setEnabled( !groups.empty( ));
  }

  void MainWindow::addGroupFromSelection( void )
  {
    const unsigned int currentIndex = _domainManager->getGroupAmount( );

    QString groupName = QString( "Group " + QString::number( currentIndex ));
    if ( !_autoNameGroups )
    {
      bool ok;
      groupName = QInputDialog::getText( this , tr( "Group Name" ) ,
                                         tr(
                                           "Please, introduce group name: " ) ,
                                         QLineEdit::Normal , groupName , &ok );

      if ( !ok ) return;
    }

    _openGLWidget->makeCurrent( );
    const auto group = _domainManager->createGroupFromSelection(
      _openGLWidget->getGidPositions( ) ,
      groupName.toStdString( ));

    addGroupControls( group ,
                      _domainManager->getGroupAmount( ) - 1 ,
                      _domainManager->getSelectionCluster( )->size( ));

    _openGLWidget->update( );

    visimpl::Selection selection;

    for ( const auto& item: group->getGids( ))
      selection.gids.insert( item );

    selection.name = groupName.toStdString( );

    _stackViz->addSelection( selection );
  }

  void MainWindow::checkGroupsVisibility( void )
  {
    unsigned int counter = 0;
    for ( auto button: _groupsVisButtons )
    {
      auto container = std::get< gr_container >( button );
      auto checkBox = std::get< gr_checkbox >( button );
      auto name = container->property( GROUP_NAME_ )
        .toString( ).toStdString( );
      auto group = _openGLWidget->domainManager( )->getGroup( name );

      if ( checkBox->isChecked( ) != group->active( ))
      {
        const auto state = checkBox->isChecked( );
        group->active( state );
        _stackViz->setHistogramVisible( counter , state );
      }

      ++counter;
    }

    _openGLWidget->update( );
  }

  void MainWindow::PlayPause( bool notify )
  {
    if ( !_openGLWidget || !_openGLWidget->player( ))
      return;

    if ( !_openGLWidget->player( )->isPlaying( ))
      Play( notify );
    else
      Pause( notify );
  }

  void MainWindow::Play( bool notify )
  {
    if ( !_openGLWidget || !_openGLWidget->player( ))
      return;

    if ( _openGLWidget )
    {
      _openGLWidget->Play( );

      _playButton->setIcon( _pauseIcon );

      if ( _openGLWidget->playbackMode( ) == TPlaybackMode::STEP_BY_STEP &&
           _openGLWidget->completedStep( ))
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
    if ( !_openGLWidget || !_openGLWidget->player( ))
      return;

    if ( _openGLWidget )
    {
      _openGLWidget->Pause( );
      _playButton->setIcon( _playIcon );
      _startTimeLabel->setText(
        QString::number( _openGLWidget->player( )->currentTime( ) , 'f' , 3 ));

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
    if ( !_openGLWidget || !_openGLWidget->player( ))
      return;

    if ( _openGLWidget )
    {
      bool repeat = _repeatButton->isChecked( );
      _openGLWidget->Repeat( repeat );

      if ( notify )
      {
#ifdef VISIMPL_USE_GMRVLEX
        const auto op = repeat ? zeroeq::gmrv::ENABLE_LOOP
                               : zeroeq::gmrv::DISABLE_LOOP;
        sendZeroEQPlaybackOperation( op );
#endif
      }
    }
  }

  void MainWindow::requestPlayAt( float timePos )
  {
    if ( !_openGLWidget || !_openGLWidget->player( ))
      return;

    auto player = _openGLWidget->player( );
    const auto tBegin = player->startTime( );
    const auto tEnd = player->endTime( );
    const auto newPosition = std::min( tEnd , std::max( tBegin , timePos ));
    const auto isOverflow = std::abs( timePos - newPosition ) >
                            std::numeric_limits< float >::epsilon( );

    PlayAtTime( newPosition , isOverflow );

    if ( isOverflow )
      Pause( true );
  }

  void MainWindow::PlayAtPosition( bool notify )
  {
    if ( !_openGLWidget || !_openGLWidget->player( ))
      return;

    PlayAtPosition( _simSlider->sliderPosition( ) , notify );
  }

  void MainWindow::PlayAtPosition( int sliderPosition , bool notify )
  {
    if ( !_openGLWidget || !_openGLWidget->player( ))
      return;

    const int sMin = _simSlider->minimum( );
    const int sMax = _simSlider->maximum( );
    const float percentage =
      static_cast<float>(sliderPosition - sMin) * ( sMax - sMin );

    PlayAtPercentage( percentage , notify );
  }

  void MainWindow::PlayAtPercentage( float percentage , bool notify )
  {
    if ( !_openGLWidget || !_openGLWidget->player( ))
      return;

    auto player = _openGLWidget->player( );
    const auto tBegin = player->startTime( );
    const auto tEnd = player->endTime( );
    const auto timePos = ( percentage * ( tEnd - tBegin )) + tBegin;

    PlayAtTime( timePos , notify );
  }

  void MainWindow::PlayAtTime( float timePos , bool notify )
  {
    if ( !_openGLWidget || !_openGLWidget->player( ))
      return;

    auto player = _openGLWidget->player( );
    const auto tBegin = player->startTime( );
    const auto tEnd = player->endTime( );
    const auto newPosition = std::max( tBegin , std::min( tEnd , timePos ));
    const auto percentage = ( newPosition - tBegin ) / ( tEnd - tBegin );

    const auto sMin = _simSlider->minimum( );
    const auto sMax = _simSlider->maximum( );
    const int sliderPosition = ( percentage * ( sMax - sMin )) + sMin;

    _simSlider->setSliderPosition( sliderPosition );

    _playButton->setIcon( _pauseIcon );

    _openGLWidget->PlayAt( newPosition );
    _openGLWidget->playbackMode( TPlaybackMode::CONTINUOUS );
    _startTimeLabel->setText(
      QString::number( player->currentTime( ) , 'f' , 3 ));

    if ( notify )
    {
#ifdef SIMIL_USE_ZEROEQ
      try
      {
        // Send event
        auto eventMgr = player->zeqEvents( );
        if ( eventMgr )
        {
          eventMgr->sendFrame( player->startTime( ) , player->endTime( ) ,
                               player->currentTime( ));
        }
      }
      catch ( const std::exception& e )
      {
        std::cerr << "Exception when sending frame. " << e.what( ) << __FILE__
                  << ":" << __LINE__ << std::endl;
      }
      catch ( ... )
      {
        std::cerr << "Unknown exception when sending frame. " << __FILE__ << ":"
                  << __LINE__ << std::endl;
      }
#endif
#ifdef VISIMPL_USE_GMRVLEX
      sendZeroEQPlaybackOperation( zeroeq::gmrv::PLAY );
#endif
    }

  }

  void MainWindow::PreviousStep( bool /*notify*/ )
  {
    if ( !_openGLWidget || !_openGLWidget->player( ))
      return;

    _playButton->setIcon( _pauseIcon );
    _openGLWidget->player( )->Play( );
    _openGLWidget->PreviousStep( );
  }

  void MainWindow::NextStep( bool /*notify*/ )
  {
    if ( !_openGLWidget || !_openGLWidget->player( ))
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

  void MainWindow::onGroupColorChanged( )
  {
    auto tfw = qobject_cast< TransferFunctionWidget* >( sender( ));
    if ( tfw )
    {
      auto groupName = tfw->property( GROUP_NAME_ ).toString( );
      updateGroup( groupName.toStdString( ) , tfw->getColors( ) ,
                   tfw->getSizeFunction( ));
    }
  }

  void MainWindow::onGroupPreview( )
  {
    auto tfw = qobject_cast< TransferFunctionWidget* >( sender( ));
    if ( tfw )
    {
      bool ok = false;
      auto groupName = tfw->property( GROUP_NAME_ ).toString( );

      if ( !ok ) return;
      updateGroup( groupName.toStdString( ) , tfw->getPreviewColors( ) ,
                   tfw->getSizePreview( ));
    }
  }

  void
  MainWindow::updateGroup( std::string name , const TTransferFunction& t ,
                           const TSizeFunction& s )
  {
    const auto& groups = _domainManager->getGroups( );
    groups.at( name )->colorMapping( t );
    groups.at( name )->sizeFunction( s );
  }

  void MainWindow::loadData( const simil::TDataType type ,
                             const std::string arg_1 , const std::string arg_2 ,
                             const simil::TSimulationType simType ,
                             const std::string& subsetEventFile )
  {
    closeLoadingDialog( );

    Q_ASSERT( type != simil::TDataType::TREST );

    QApplication::setOverrideCursor( Qt::WaitCursor );

    _objectInspectorGB->setCheckUpdates( false );
    _ui->actionConfigureREST->setEnabled( false );

    Q_ASSERT( simType == simil::TSimulationType::TSimSpikes );

    m_loaderDialog = new LoadingDialog( this );
    m_loaderDialog->show( );

    QApplication::processEvents( );

    m_loader = std::make_shared< LoaderThread >( );
    m_loader->setData( type , arg_1 , arg_2 );

    connect( m_loader.get( ) , SIGNAL( finished( )) ,
             this , SLOT( onDataLoaded( )));
    connect( m_loader.get( ) , SIGNAL( progress( int )) ,
             m_loaderDialog , SLOT( setProgress( int )));
    connect( m_loader.get( ) , SIGNAL( network( unsigned int )) ,
             m_loaderDialog , SLOT( setNetwork( unsigned int )));
    connect( m_loader.get( ) , SIGNAL( spikes( unsigned int )) ,
             m_loaderDialog , SLOT( setSpikesValue( unsigned int )));

    _lastOpenedNetworkFileName = QString::fromStdString( arg_1 );
    _lastOpenedSubsetsFileName = QString::fromStdString( subsetEventFile );

    m_loader->start( );
  }

  void MainWindow::closeLoadingDialog( )
  {
    if ( m_loaderDialog )
    {
      m_loaderDialog->close( );
      delete m_loaderDialog;
      m_loaderDialog = nullptr;
    }
  }

  void MainWindow::onDataLoaded( )
  {
    setWindowTitle( "SimPart" );

    if ( !m_loader ) return;

    const auto error = m_loader->errors( );
    const auto filename = QString::fromStdString( m_loader->filename( ));
    if ( !error.empty( ))
    {
      closeLoadingDialog( );
      QApplication::restoreOverrideCursor( );

      const auto message = QString::fromStdString( error );
      QMessageBox::critical( this , tr( "Error loading data" ) , message ,
                             QMessageBox::Ok );

      m_loader = nullptr;
      return;
    }

    simil::SpikesPlayer* player = nullptr;
    const auto dataType = m_loader->type( );

    switch ( dataType )
    {
      case simil::TBlueConfig:
      case simil::TCSV:
      case simil::THDF5:
      {
        const auto spikeData = m_loader->simulationData( );

        player = new simil::SpikesPlayer( );
        player->LoadData( spikeData );

        _subsetEvents = spikeData->subsetsEvents( );

        m_loader = nullptr;
      }
        break;
      case simil::TREST:
      {
        const auto netData = m_loader->network( );
        const auto simData = m_loader->simulationData( );

        player = new simil::SpikesPlayer( );
        player->LoadData( netData , simData );

        _subsetEvents = netData->subsetsEvents( );

        m_loader = nullptr;
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

    if ( !player )
    {
      closeLoadingDialog( );
      QApplication::restoreOverrideCursor( );

      const auto message = tr( "Unable to load data." );
      QMessageBox::critical( this , tr( "Error loading data" ) , message ,
                             QMessageBox::Ok );

      m_loader = nullptr;
      return;
    }

    if ( m_loaderDialog )
    {
      m_loaderDialog->setNetwork( player->gidsSize( ));
      m_loaderDialog->setSpikesValue( player->spikesSize( ));
      m_loaderDialog->repaint( );
    }

    setWindowTitle( "SimPart - " + filename );

    _openGLWidget->setPlayer( player , dataType );
    _modeSelectionWidget->setCurrentIndex( 0 );

    _openGLWidget->subsetEventsManager( _subsetEvents );
    _openGLWidget->showEventsActivityLabels(
      _ui->actionShowEventsActivity->isChecked( ));
    _openGLWidget->idleUpdate( _ui->actionUpdateOnIdle->isChecked( ));

    configureComponents( );

    openSubsetEventFile( m_subsetEventFile , false );

    _configurePlayer( );

    _comboAttribSelection->clear( );

    _stackViz->init( player , _subsetEvents );

    _subsetImporter->reload( _subsetEvents );

    switch ( dataType )
    {
      case simil::TDataType::TBlueConfig:
      {
        const QStringList attributes = { "Morphological type" ,
                                         "Functional type" };
        _comboAttribSelection->addItems( attributes );
      }
        break;
      case simil::TDataType::TREST:
      {
#ifdef SIMIL_WITH_REST_API
        //const auto waitTime = m_loader->RESTLoader( )->getConfiguration( ).waitTime;
        //_objectInspectorGB->setCheckTimer( waitTime );
        //_objectInspectorGB->setCheckUpdates( true );
        //_ui->actionConfigureREST->setEnabled( true );
#endif
      }
        break;
      case simil::TDataType::TCONE:
      case simil::TDataType::TCSV:
      case simil::TDataType::THDF5:
      default:
        break;
    }

    closeLoadingDialog( );

    if ( !_lastOpenedSubsetsFileName.isEmpty( ))
    {
      openSubsetEventFile( _lastOpenedSubsetsFileName.toStdString( ) , false );
    }

    QApplication::restoreOverrideCursor( );
  }

  void MainWindow::onGroupNameClicked( )
  {
    auto button = qobject_cast< QPushButton* >( sender( ));
    if ( button )
    {
      bool ok = false;
      auto groupName = button->property( GROUP_NAME_ ).toString( );
      auto group = _domainManager->getGroup( groupName.toStdString( ));

      groupName = QInputDialog::getText( this , tr( "Group Name" ) ,
                                         tr(
                                           "Please, introduce group name: " ) ,
                                         QLineEdit::Normal , groupName , &ok );

      if ( !ok ) return;
      group->name( groupName.toStdString( ));
      button->setText( groupName );

      _stackViz->changeHistogramName( 0 , groupName );
    }
  }

  void MainWindow::loadGroups( )
  {
    const auto title = tr( "Load Groups" );

    if ( _domainManager->getGroupAmount( ) > 0 )
    {
      const auto message = tr( "Loading groups from disk will erase the "
                               "current groups. Do you want to continue?" );

      QMessageBox msgbox( this );
      msgbox.setWindowTitle( title );
      msgbox.setText( message );
      msgbox.setIcon( QMessageBox::Icon::Question );
      msgbox.setWindowIcon( QIcon( ":/visimpl.png" ));
      msgbox.setStandardButtons( QMessageBox::Cancel | QMessageBox::Ok );
      msgbox.setDefaultButton( QMessageBox::Button::Ok );

      if ( QMessageBox::Ok != msgbox.exec( ))
        return;
    }

    QFileInfo lastFile{ _lastOpenedNetworkFileName };
    const auto fileName = QFileDialog::getOpenFileName( this , title ,
                                                        lastFile.path( ) ,
                                                        tr(
                                                          "Json files (*.json)" ) ,
                                                        nullptr ,
                                                        QFileDialog::ReadOnly |
                                                        QFileDialog::DontUseNativeDialog );
    if ( fileName.isEmpty( )) return;

    QFile file{ fileName };
    if ( !file.open( QIODevice::ReadOnly ))
    {
      const auto message = tr( "Couldn't open file %1" ).arg( fileName );

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
        "Couldn't read the contents of %1 or parsing error." ).arg( fileName );

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
        fileName );

      QMessageBox msgbox{ this };
      msgbox.setWindowTitle( title );
      msgbox.setIcon( QMessageBox::Icon::Critical );
      msgbox.setText( message );
      msgbox.setWindowIcon( QIcon( ":/visimpl.png" ));
      msgbox.setStandardButtons( QMessageBox::Ok );
      msgbox.exec( );
      return;
    }

    const QFileInfo currentFile{ _lastOpenedNetworkFileName };
    const QString jsonGroupsFile = jsonObj.value( "filename" ).toString( );
    if ( !jsonGroupsFile.isEmpty( ) &&
         jsonGroupsFile.compare( currentFile.fileName( ) ,
                                 Qt::CaseInsensitive ) != 0 )
    {
      const auto message = tr(
        "This groups definitions are from file '%1'. Current file"
        " is '%2'. Do you want to continue?" ).arg( jsonGroupsFile ).arg(
        currentFile.fileName( ));

      QMessageBox msgbox{ this };
      msgbox.setWindowTitle( title );
      msgbox.setIcon( QMessageBox::Icon::Question );
      msgbox.setText( message );
      msgbox.setWindowIcon( QIcon( ":/visimpl.png" ));
      msgbox.setStandardButtons( QMessageBox::Cancel | QMessageBox::Ok );
      msgbox.setDefaultButton( QMessageBox::Ok );

      if ( QMessageBox::Ok != msgbox.exec( ))
        return;
    }

    QApplication::setOverrideCursor( Qt::WaitCursor );

    _openGLWidget->makeCurrent( );

    clearGroups( );
    const auto jsonGroups = jsonObj.value( "groups" ).toArray( );

    std::vector< VisualGroup* > groupsList;
    const auto createGroup = [ this ]( const QJsonValue& v )
    {
      const auto o = v.toObject( );

      const auto name = o.value( "name" ).toString( );
      const auto gidsStrings = o.value( "gids" ).toString( ).split( "," );

      GIDUSet gids;
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

      auto group = _domainManager->createGroup(
        gids ,
        _openGLWidget->getGidPositions( ) ,
        name.toStdString( ));
      auto idx = _domainManager->getGroupAmount( ) - 1;
      addGroupControls( group , idx , gids.size( ));
      const auto active = o.value( "active" ).toBool( true );
      auto checkbox = std::get< gr_checkbox >( _groupsVisButtons.at( idx ));
      checkbox->setChecked( active );

      visimpl::Selection selection;
      selection.gids = gids;
      selection.name = name.toStdString( );

      _stackViz->addSelection( selection );

      group->active( active );

      const auto functionPairs = o.value( "function" ).toString( ).split( ";" );
      TTransferFunction function;
      auto addFunctionPair = [ &function ]( const QString& s )
      {
        const auto parts = s.split( "," );
        Q_ASSERT( parts.size( ) == 2 );
        const auto value = parts.first( ).toFloat( );
        const auto color = QColor( parts.last( ));
        function.emplace_back( value , color );
      };
      std::for_each( functionPairs.cbegin( ) , functionPairs.cend( ) ,
                     addFunctionPair );

      const auto sizePairs = o.value( "sizes" ).toString( ).split( ";" );
      TSizeFunction sizes;
      auto addSizes = [ &sizes ]( const QString& s )
      {
        const auto parts = s.split( "," );
        Q_ASSERT( parts.size( ) == 2 );
        const auto a = parts.first( ).toFloat( );
        const auto b = parts.last( ).toFloat( );
        sizes.emplace_back( a , b );
      };
      std::for_each( sizePairs.cbegin( ) , sizePairs.cend( ) , addSizes );

      updateGroup( name.toStdString( ) , function , sizes );
      auto container = std::get< gr_container >( _groupsVisButtons.at( idx ));
      auto tfw = qobject_cast< TransferFunctionWidget* >(
        container->layout( )->itemAt( 0 )->widget( ));
      if ( tfw )
      {
        tfw->setColorPoints( function , true );
        tfw->setSizeFunction( sizes );
        tfw->colorChanged( );
        tfw->sizeChanged( );
      }
    };
    std::for_each( jsonGroups.constBegin( ) , jsonGroups.constEnd( ) ,
                   createGroup );

    _groupLayout->update( );
    checkGroupsVisibility( );

    bool notEmpty = _domainManager->getGroupAmount( ) > 0;
    _buttonClearGroups->setEnabled( notEmpty );
    _buttonSaveGroups->setEnabled( notEmpty );

    _openGLWidget->update( );

    QApplication::restoreOverrideCursor( );
  }

  void MainWindow::saveGroups( )
  {
    const auto& groups = _domainManager->getGroups( );
    if ( groups.empty( )) return;

    const auto dateTime = QDateTime::currentDateTime( );
    QFileInfo lastFile{ _lastOpenedNetworkFileName };
    QString filename = lastFile.dir( ).absoluteFilePath(
      lastFile.baseName( ) + "_groups_" +
      dateTime.toString( "yyyy-MM-dd-hh-mm" ) + ".json" );
    filename = QFileDialog::getSaveFileName( this , tr( "Save Groups" ) ,
                                             filename ,
                                             tr( "Json files (*.json)" ) ,
                                             nullptr ,
                                             QFileDialog::DontUseNativeDialog );

    if ( filename.isEmpty( )) return;

    QFile wFile{ filename };
    if ( !wFile.open(
      QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ))
    {
      const auto message = tr( "Unable to open file %1 for writing." ).arg(
        filename );

      QMessageBox msgbox{ this };
      msgbox.setWindowTitle( tr( "Save Groups" ));
      msgbox.setIcon( QMessageBox::Icon::Critical );
      msgbox.setText( message );
      msgbox.setWindowIcon( QIcon( ":/visimpl.png" ));
      msgbox.setDefaultButton( QMessageBox::Ok );
      msgbox.exec( );
      return;
    }

    QApplication::setOverrideCursor( Qt::WaitCursor );

    QJsonObject obj;
    obj.insert( "filename" ,
                QFileInfo{ _lastOpenedNetworkFileName }.fileName( ));
    obj.insert( "date" , dateTime.toString( ));

    QJsonArray groupsObjs;

    auto insertGroup = [ &groupsObjs ]( std::shared_ptr< VisualGroup > g )
    {
      QJsonObject groupObj;
      groupObj.insert( "name" , QString::fromStdString( g->name( )));
      groupObj.insert( "active" , g->active( ));

      QStringList tfList;
      const auto tf = g->colorMapping( );
      auto addColors = [ &tfList ]( const TTFColor& c )
      {
        tfList << QString( "%1,%2" ).arg( c.first ).arg(
          c.second.name( QColor::HexArgb ));
      };
      std::for_each( tf.cbegin( ) , tf.cend( ) , addColors );
      groupObj.insert( "function" , tfList.join( ";" ));

      QStringList sizesList;
      const auto sizes = g->sizeFunction( );
      auto addSizes = [ &sizesList ]( const TSize& s )
      {
        sizesList << QString( "%1,%2" ).arg( s.first ).arg( s.second );
      };
      std::for_each( sizes.cbegin( ) , sizes.cend( ) , addSizes );
      groupObj.insert( "sizes" , sizesList.join( ";" ));

      const auto& gids = g->getGids( );
      std::vector< unsigned int > gidsVec;
      std::for_each( gids.cbegin( ) , gids.cend( ) ,
                     [ &gidsVec ]( unsigned int v )
                     { gidsVec.push_back( v ); } );
      std::sort( gidsVec.begin( ) , gidsVec.end( ));
      QStringList gidsStrings;
      std::pair< unsigned int , unsigned int > range = std::make_pair(
        std::numeric_limits< unsigned int >::max( ) - 1 ,
        std::numeric_limits< unsigned int >::max( ) - 1 );
      auto enterNumber = [ &range , &gidsStrings ]( )
      {
        if ( range.first == range.second )
          gidsStrings << QString::number( range.first );
        else
          gidsStrings
            << QString( "%1:%2" ).arg( range.first ).arg( range.second );
      };

      for ( auto i = gidsVec.begin( ); i != gidsVec.end( ); ++i )
      {
        auto num = *i;
        if ( num != range.second + 1 )
        {
          if ( range.first != std::numeric_limits< unsigned int >::max( ) - 1 )
          {
            enterNumber( );
          }
          range.first = num;
        }

        range.second = num;
      }
      enterNumber( );

      groupObj.insert( "gids" , gidsStrings.join( "," ));

      groupsObjs << groupObj;
    };
    for ( const auto& item: groups )
      insertGroup( item.second );

    obj.insert( "groups" , groupsObjs );

    QJsonDocument doc{ obj };
    wFile.write( doc.toJson( ));

    QApplication::restoreOverrideCursor( );

    if ( wFile.error( ) != QFile::NoError )
    {
      const auto message = tr( "Error saving file %1." ).arg( filename );

      QMessageBox msgbox{ this };
      msgbox.setWindowTitle( tr( "Save Groups" ));
      msgbox.setIcon( QMessageBox::Icon::Critical );
      msgbox.setText( message );
      msgbox.setDetailedText( wFile.errorString( ));
      msgbox.setWindowIcon( QIcon( ":/visimpl.png" ));
      msgbox.setDefaultButton( QMessageBox::Ok );
      msgbox.exec( );
    }

    wFile.flush( );
    wFile.close( );
  }

  void MainWindow::onGroupDeleteClicked( )
  {
    auto button = qobject_cast< QPushButton* >( sender( ));
    if ( button )
    {
      const auto groupName = button->property( GROUP_NAME_ ).toString( );
      removeVisualGroup( groupName );
    }
  }

  void MainWindow::removeVisualGroup( const QString& name )
  {
    auto findGroup = [ name ]( tGroupRow& r )
    {
      QWidget* container = std::get< gr_container >( r );
      auto groupName = container->property( GROUP_NAME_ ).toString( );
      return groupName.compare( name , Qt::CaseInsensitive ) == 0;
    };
    auto it = std::find_if( _groupsVisButtons.begin( ) ,
                            _groupsVisButtons.end( ) , findGroup );
    if ( it == _groupsVisButtons.end( )) return;

    auto container = std::get< gr_container >( *it );
    _groupLayout->removeWidget( container );
    delete container;
    const auto idx = std::distance( _groupsVisButtons.begin( ) , it );

    _domainManager->removeGroup( name.toStdString( ));

    _stackViz->removeSubset( idx );

    _groupsVisButtons.erase( _groupsVisButtons.begin( ) + idx );

    const bool enabled = !_groupsVisButtons.empty( );
    _buttonClearGroups->setEnabled( enabled );
    _buttonSaveGroups->setEnabled( enabled );
  }

  void MainWindow::changeStackVizToolbarStatus( bool status )
  {
    _ui->actionStackVizAutoNamingSelections->setEnabled( status );
    _ui->actionStackVizFillPlots->setEnabled( status );
    _ui->actionStackVizFocusOnPlayhead->setEnabled( status );
    _ui->actionStackVizFollowPlayHead->setEnabled( status );
    _ui->actionStackVizShowDataManager->setEnabled( status );
    _ui->actionStackVizShowPanels->setEnabled( status );
  }

  void MainWindow::finishRecording( )
  {
    auto recorderAction = _ui->menuTools->actions( ).first( );
    recorderAction->setEnabled( true );
    recorderAction->setChecked( false );
  }

  void MainWindow::closeEvent( QCloseEvent* e )
  {
    if ( _recorder )
    {
      QMessageBox msgBox( this );
      msgBox.setWindowTitle( tr( "Exit SimPart" ));
      msgBox.setWindowIcon( QIcon( ":/visimpl.png" ));
      msgBox.setText(
        tr( "A recording is being made. Do you really want to exit SimPart?" ));
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

  void MainWindow::loadCameraPositions( )
  {
    const QString title = "Load camera positions";

    auto actions = _ui->actionCamera_Positions->menu( )->actions( );
    const auto numActions = actions.size( );
    if ( numActions > 0 )
    {
      const auto warnText = tr( "Loading new camera positions will remove"
                                " %1 existing position%2. Are you sure?" ).arg(
        numActions ).arg( numActions > 1 ? "s" : "" );
      if ( QMessageBox::Ok != QMessageBox::warning( this , title , warnText ,
                                                    QMessageBox::Cancel |
                                                    QMessageBox::Ok ))
        return;
    }

    const QString nameFilter = "Camera positions (*.json)";
    QDir directory;

    if ( _lastOpenedNetworkFileName.isEmpty( ))
      directory = QDir::home( );
    else
      directory = QFileInfo( _lastOpenedNetworkFileName ).dir( );

    QFileDialog fDialog( this );
    fDialog.setWindowIcon( QIcon( ":/visimpl.png" ));
    fDialog.setWindowTitle( title );
    fDialog.setAcceptMode( QFileDialog::AcceptMode::AcceptOpen );
    fDialog.setDefaultSuffix( "json" );
    fDialog.setDirectory( directory );
    fDialog.setOption( QFileDialog::Option::DontUseNativeDialog , true );
    fDialog.setFileMode( QFileDialog::FileMode::ExistingFile );
    fDialog.setNameFilters( QStringList{ nameFilter } );
    fDialog.setNameFilter( nameFilter );

    if ( fDialog.exec( ) != QFileDialog::Accepted )
      return;

    if ( fDialog.selectedFiles( ).empty( )) return;

    auto file = fDialog.selectedFiles( ).first( );

    QFile posFile{ file };
    if ( !posFile.open( QIODevice::ReadOnly | QIODevice::Text ))
    {
      const QString errorText = tr( "Unable to open: %1" ).arg( file );
      QMessageBox::critical( this , title , errorText );
      return;
    }

    const auto contents = posFile.readAll( );
    QJsonParseError parserError;

    const auto jsonDoc = QJsonDocument::fromJson( contents , &parserError );
    if ( jsonDoc.isNull( ) || !jsonDoc.isObject( ))
    {
      const auto message = tr(
        "Couldn't read the contents of %1 or parsing error." ).arg( file );

      QMessageBox msgbox{ this };
      msgbox.setWindowTitle( title );
      msgbox.setIcon( QMessageBox::Icon::Critical );
      msgbox.setText( message );
      msgbox.setWindowIcon( QIcon( ":/visimpl.png" ));
      msgbox.setStandardButtons( QMessageBox::Ok );
      msgbox.setDetailedText( parserError.errorString( ));
      msgbox.exec( );
      return;
    }

    const auto jsonObj = jsonDoc.object( );
    if ( jsonObj.isEmpty( ))
    {
      const auto message = tr( "Error parsing the contents of %1." ).arg(
        file );

      QMessageBox msgbox{ this };
      msgbox.setWindowTitle( title );
      msgbox.setIcon( QMessageBox::Icon::Critical );
      msgbox.setText( message );
      msgbox.setWindowIcon( QIcon( ":/visimpl.png" ));
      msgbox.setStandardButtons( QMessageBox::Ok );
      msgbox.exec( );
      return;
    }

    const QFileInfo currentFile{ _lastOpenedNetworkFileName };
    const QString jsonPositionsFile = jsonObj.value( "filename" ).toString( );
    if ( !jsonPositionsFile.isEmpty( ) &&
         jsonPositionsFile.compare( currentFile.fileName( ) ,
                                    Qt::CaseInsensitive ) != 0 )
    {
      const auto message = tr( "This positions are from file '%1'. Current file"
                               " is '%2'. Do you want to continue?" ).arg(
        jsonPositionsFile ).arg( currentFile.fileName( ));

      QMessageBox msgbox{ this };
      msgbox.setWindowTitle( title );
      msgbox.setIcon( QMessageBox::Icon::Question );
      msgbox.setText( message );
      msgbox.setWindowIcon( QIcon( ":/visimpl.png" ));
      msgbox.setStandardButtons( QMessageBox::Cancel | QMessageBox::Ok );
      msgbox.setDefaultButton( QMessageBox::Ok );

      if ( QMessageBox::Ok != msgbox.exec( ))
        return;
    }

    // Clear existing actions before entering new ones.
    for ( auto action: actions )
    {
      _ui->actionCamera_Positions->menu( )->removeAction( action );
      delete action;
    }

    const auto jsonPositions = jsonObj.value( "positions" ).toArray( );

    auto createPosition = [ this ]( const QJsonValue& v )
    {
      const auto o = v.toObject( );

      const auto name = o.value( "name" ).toString( );
      const auto position = o.value( "position" ).toString( );
      const auto radius = o.value( "radius" ).toString( );
      const auto rotation = o.value( "rotation" ).toString( );

      auto action = new QAction( name );
      action->setProperty( POSITION_KEY_ ,
                           position + ";" + radius + ";" + rotation );

      connect( action , SIGNAL( triggered( bool )) , this ,
               SLOT( applyCameraPosition( )));

      _ui->actionCamera_Positions->menu( )->addAction( action );
    };
    std::for_each( jsonPositions.constBegin( ) , jsonPositions.constEnd( ) ,
                   createPosition );

    const bool positionsExist = !_ui->actionCamera_Positions->menu( )->actions( ).isEmpty( );
    _ui->actionSave_camera_positions->setEnabled( positionsExist );
    _ui->actionRemove_camera_position->setEnabled( positionsExist );
    _ui->actionCamera_Positions->setEnabled( positionsExist );
  }

  void MainWindow::saveCameraPositions( )
  {
    const QString nameFilter = "Camera positions (*.json)";
    QDir directory;
    QString filename;

    if ( _lastOpenedNetworkFileName.isEmpty( ))
    {
      directory = QDir::home( );
      filename = "positions.json";
    }
    else
    {
      QFileInfo fi( _lastOpenedNetworkFileName );
      directory = fi.dir( );
      filename = QString( "%1_positions.json" ).arg( fi.baseName( ));
    }

    QFileDialog fDialog( this );
    fDialog.setWindowIcon( QIcon( ":/visimpl.png" ));
    fDialog.setWindowTitle( "Save camera positions" );
    fDialog.setAcceptMode( QFileDialog::AcceptMode::AcceptSave );
    fDialog.setDefaultSuffix( "json" );
    fDialog.selectFile( filename );
    fDialog.setDirectory( directory );
    fDialog.setOption( QFileDialog::Option::DontUseNativeDialog , true );
    fDialog.setOption( QFileDialog::Option::DontConfirmOverwrite , false );
    fDialog.setFileMode( QFileDialog::FileMode::AnyFile );
    fDialog.setNameFilters( QStringList{ nameFilter } );
    fDialog.setNameFilter( nameFilter );

    if ( fDialog.exec( ) != QFileDialog::Accepted )
      return;

    if ( fDialog.selectedFiles( ).empty( )) return;

    filename = fDialog.selectedFiles( ).first( );

    QFile wFile{ filename };
    if ( !wFile.open(
      QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ))
    {
      const auto message = tr( "Unable to open file %1 for writing." ).arg(
        filename );

      QMessageBox msgbox{ this };
      msgbox.setWindowTitle( tr( "Save camera positions" ));
      msgbox.setIcon( QMessageBox::Icon::Critical );
      msgbox.setText( message );
      msgbox.setWindowIcon( QIcon( ":/visimpl.png" ));
      msgbox.setDefaultButton( QMessageBox::Ok );
      msgbox.exec( );
      return;
    }

    QApplication::setOverrideCursor( Qt::WaitCursor );

    const auto actions = _ui->actionCamera_Positions->menu( )->actions( );

    QJsonArray positionsObjs;

    auto insertPosition = [ &positionsObjs ]( const QAction* a )
    {
      if ( !a ) return;
      const auto posData = a->property( POSITION_KEY_ ).toString( );
      const auto parts = posData.split( ";" );
      Q_ASSERT( parts.size( ) == 3 );
      const auto position = parts.first( );
      const auto radius = parts.at( 1 );
      const auto rotation = parts.last( );

      QJsonObject positionObj;
      positionObj.insert( "name" , a->text( ));
      positionObj.insert( "position" , position );
      positionObj.insert( "radius" , radius );
      positionObj.insert( "rotation" , rotation );

      positionsObjs << positionObj;
    };
    std::for_each( actions.cbegin( ) , actions.cend( ) , insertPosition );

    QJsonObject obj;
    obj.insert( "filename" ,
                QFileInfo{ _lastOpenedNetworkFileName }.fileName( ));
    obj.insert( "positions" , positionsObjs );

    QJsonDocument doc{ obj };
    wFile.write( doc.toJson( ));

    QApplication::restoreOverrideCursor( );

    if ( wFile.error( ) != QFile::NoError )
    {
      const auto message = tr( "Error saving file %1." ).arg( filename );

      QMessageBox msgbox{ this };
      msgbox.setWindowTitle( tr( "Save camera positions" ));
      msgbox.setIcon( QMessageBox::Icon::Critical );
      msgbox.setText( message );
      msgbox.setDetailedText( wFile.errorString( ));
      msgbox.setWindowIcon( QIcon( ":/visimpl.png" ));
      msgbox.setDefaultButton( QMessageBox::Ok );
      msgbox.exec( );
    }

    wFile.flush( );
    wFile.close( );
  }

  void MainWindow::addCameraPosition( )
  {
    QStringList items;

    auto actions = _ui->actionCamera_Positions->menu( )->actions( );
    auto insertItemName = [ &items ]( const QAction* a )
    { items << a->text( ); };
    std::for_each( actions.cbegin( ) , actions.cend( ) , insertItemName );

    const QString title = tr( "Add camera position" );

    bool ok = false;
    QString name;
    while ( !ok || name.isEmpty( ))
    {
      name = QInputDialog::getText( this , title , tr( "Position name:" ) ,
                                    QLineEdit::Normal , tr( "New position" ) ,
                                    &ok );

      if ( !ok ) return; // user cancelled

      if ( ok && !name.isEmpty( ))
      {
        QString tempName( name );
        int collision = 0;
        while ( items.contains( tempName , Qt::CaseInsensitive ))
        {
          ++collision;
          tempName = tr( "%1 (%2)" ).arg( name ).arg( collision );
        }

        name = tempName;
      }
    }

    auto action = new QAction( name );

    const auto position = _openGLWidget->cameraPosition( );
    action->setProperty( POSITION_KEY_ , position.toString( ));

    connect( action , SIGNAL( triggered( bool )) , this ,
             SLOT( applyCameraPosition( )));
    _ui->actionCamera_Positions->menu( )->addAction( action );
    _ui->actionCamera_Positions->setEnabled( true );
    _ui->actionSave_camera_positions->setEnabled( true );
    _ui->actionRemove_camera_position->setEnabled( true );
  }

  void MainWindow::removeCameraPosition( )
  {
    bool ok = false;
    QStringList items;

    auto actions = _ui->actionCamera_Positions->menu( )->actions( );
    auto insertItemName = [ &items ]( const QAction* a )
    { items << a->text( ); };
    std::for_each( actions.cbegin( ) , actions.cend( ) , insertItemName );

    auto item = QInputDialog::getItem( this , tr( "Remove camera position" ) ,
                                       tr( "Position name:" ) , items , 0 ,
                                       false , &ok );
    if ( ok && !item.isEmpty( ))
    {
      auto actionOfName = [ &item ]( const QAction* a )
      { return a->text( ) == item; };
      const auto it = std::find_if( actions.cbegin( ) , actions.cend( ) ,
                                    actionOfName );
      auto distance = std::distance( actions.cbegin( ) , it );
      auto action = actions.at( distance );
      _ui->actionCamera_Positions->menu( )->removeAction( action );
      delete action;

      const auto enabled = actions.size( ) > 1;
      _ui->actionRemove_camera_position->setEnabled( enabled );
      _ui->actionSave_camera_positions->setEnabled( enabled );
      _ui->actionCamera_Positions->setEnabled( enabled );
    }
  }

  void MainWindow::applyCameraPosition( )
  {
    auto action = qobject_cast< QAction* >( sender( ));
    if ( action )
    {
      auto positionString = action->property( POSITION_KEY_ ).toString( );
      CameraPosition position( positionString );
      _openGLWidget->setCameraPosition( position );
    }
  }

  void MainWindow::openRESTThroughDialog( )
  {
#ifdef SIMIL_WITH_REST_API

    if ( _alreadyConnected && _openGLWidget->player( ) != nullptr )
    {
      auto player = _openGLWidget->player( );
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
          : player->getNetwork( );

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
    const auto title = tr( "Connect REST API" );
    const auto message = tr( "REST data loading is unsupported." );
    QMessageBox::critical( this , title , message , QMessageBox::Ok );
#endif
  }

  void MainWindow::configureREST( )
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
    const auto title = tr( "Configure REST API" );
    const auto message = tr( "REST data loading is unsupported." );
    QMessageBox::critical( this , title , message , QMessageBox::Ok );
#endif
  }

#ifdef SIMIL_WITH_REST_API

  void MainWindow::loadRESTData( const simil::LoaderRestData::Configuration& o )
  {
    closeLoadingDialog( );

    QApplication::setOverrideCursor( Qt::WaitCursor );

    _objectInspectorGB->setCheckUpdates( false );

    m_loaderDialog = new LoadingDialog( this );
    m_loaderDialog->show( );

    QApplication::processEvents( );

    m_loader = std::make_shared< LoaderThread >( );
    m_loader->setRESTConfiguration( o );

    connect( m_loader.get( ) , SIGNAL( finished( )) ,
             this , SLOT( onDataLoaded( )));
    connect( m_loader.get( ) , SIGNAL( progress( int )) ,
             m_loaderDialog , SLOT( setProgress( int )));
    connect( m_loader.get( ) , SIGNAL( network( unsigned int )) ,
             m_loaderDialog , SLOT( setNetwork( unsigned int )));
    connect( m_loader.get( ) , SIGNAL( spikes( unsigned int )) ,
             m_loaderDialog , SLOT( setSpikesValue( unsigned int )));

    m_loader->start( );
  }

#endif

  void MainWindow::changePlanesColor( const QColor& color_ )
  {
    const auto currentColor = QColor(
      _frameClippingColor->property( PLANES_COLOR_KEY_ ).toString( ));
    if ( currentColor == color_ ) return;

    _frameClippingColor->setStyleSheet( "background-color: " + color_.name( ));
    _frameClippingColor->setProperty( PLANES_COLOR_KEY_ , color_.name( ));
  }

  void MainWindow::sendZeroEQPlaybackOperation( const unsigned int op )
  {
#ifdef SIMIL_USE_ZEROEQ
    try
    {
      auto eventMgr = _openGLWidget->player( )->zeqEvents( );
      if ( eventMgr )
      {
        eventMgr->sendPlaybackOp(
          static_cast<zeroeq::gmrv::PlaybackOperation>(op));
      }
    }
    catch ( const std::exception& e )
    {
      std::cerr << "Exception when sending play operation. " << e.what( )
                << __FILE__ << ":" << __LINE__ << std::endl;
    }
    catch ( ... )
    {
      std::cerr << "Unknown exception when sending play operation. " << __FILE__
                << ":" << __LINE__ << std::endl;
    }
#else
    __attribute__((unused)) const auto unused = op; // c++17 [[maybe_unused]]
#endif
  }

} // namespace visimpl
