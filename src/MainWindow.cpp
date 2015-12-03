#include "ui_mainwindow.h"
#include "MainWindow.h"
#include <QDebug>
#include <QFileDialog>
#include <QInputDialog>

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

#ifdef NSOL_USE_BBPSDK
  _ui->actionOpenBlueConfig->setEnabled( true );
#else
  _ui->actionOpenBlueConfig->setEnabled( false );
#endif

#ifdef NSOL_USE_QT5CORE
  _ui->actionOpenXMLScene->setEnabled( true );
#else
  _ui->actionOpenXMLScene->setEnabled( false );
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
                                 const std::string& targetLabel,
                                 const std::string& reportLabel)
{
  _openGLWidget->loadData( fileName,
                           OpenGLWidget::TDataFileType::tBlueConfig,
                           targetLabel, reportLabel );

}

void MainWindow::openBlueConfigThroughDialog( void )
{
#ifdef NSOL_USE_BBPSDK

  QString path = QFileDialog::getOpenFileName(
    this, tr( "Open BlueConfig" ), _lastOpenedFileName,
    tr( "BlueConfig ( BlueConfig CircuitConfig);; All files (*)" ),
    nullptr, QFileDialog::DontUseNativeDialog );

  if (path != QString( "" ))
  {
    bool ok1, ok2;
    QString text = QInputDialog::getText(
      this, tr( "Please select target" ),
      tr( "Cell target:" ), QLineEdit::Normal,
      "Layer1", &ok1 );

    QString text2 = QInputDialog::getText(
          this, tr( "Please select report" ),
          tr( "Cell report:" ), QLineEdit::Normal,
          "voltage", &ok2 );

    if ( ok1 && ok2 && !text.isEmpty( ) && !text2.isEmpty( ))
    {
      std::string targetLabel = text.toStdString( );
      std::string reportLabel = text.toStdString( );
      _lastOpenedFileName = QFileInfo(path).path( );
      std::string fileName = path.toStdString( );
      openBlueConfig( fileName, targetLabel, reportLabel );
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
