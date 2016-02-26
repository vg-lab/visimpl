#include "ui_stackviz.h"
#include "MainWindow.h"
#include <QDebug>
#include <QFileDialog>
#include <QInputDialog>
#include <QGridLayout>

MainWindow::MainWindow( QWidget* parent_ )
  : QMainWindow( parent_ )
  // , _lastOpenedFileName( "" )
  , _ui( new Ui::MainWindow )
  // , _openGLWidget( nullptr )
{
  _ui->setupUi( this );

#ifdef VISIMPL_USE_BBPSDK
  _ui->actionOpenBlueConfig->setEnabled( true );
#else
  _ui->actionOpenBlueConfig->setEnabled( false );
#endif

  connect( _ui->actionQuit, SIGNAL( triggered( )),
           QApplication::instance(), SLOT( quit( )));

}

void MainWindow::init( const std::string& /*zeqUri*/ )
{

  connect( _ui->actionOpenBlueConfig, SIGNAL( triggered( )),
           this, SLOT( openBlueConfigThroughDialog( )));

  // initSimulationDock( );

  // #ifdef VISIMPL_USE_ZEQ
  // if( !zeqUri.empty( ))
  // {
  //     _setZeqUri( zeqUri );
  // }
  // #endif
}

MainWindow::~MainWindow( void )
{
    delete _ui;
}


void MainWindow::showStatusBarMessage ( const QString& message )
{
  _ui->statusbar->showMessage( message );
}

void MainWindow::openBlueConfig( const std::string& /*fileName*/,
                                 // visimpl::TSimulationType simulationType,
                                 const std::string& /*reportLabel*/)
{
  // _openGLWidget->loadData( fileName,
  //                          OpenGLWidget::TDataFileType::tBlueConfig,
  //                          simulationType, reportLabel );


  // connect( _openGLWidget, SIGNAL( updateSlider( float )),
  //          this, SLOT( UpdateSimulationSlider( float )));


  // _startTimeLabel->setText(
  //     QString::number( (double)_openGLWidget->player( )->startTime( )));

  // _endTimeLabel->setText(
  //       QString::number( (double)_openGLWidget->player( )->endTime( )));


  // changeEditorColorMapping( );
  // initSummaryWidget( );
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