#include <QMainWindow>
#include <QDockWidget>
#include <QPushButton>
#include <QSlider>
#include <QTimer>

// #include "SimulationPlayer.h"

#include <sumrice/sumrice.h>
// #include "SimulationSummaryWidget.h"

// #include "EditorTF/TransferFunctionEditor.h"


namespace Ui
{
class MainWindow;
}

class MainWindow
  : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow( QWidget* parent = 0 );
  ~MainWindow( void );

  void init( const std::string& zeqUri = "" );
  void showStatusBarMessage ( const QString& message );

  void openBlueConfig( const std::string& fileName,
                       // visimpl::TSimulationType simulationType,
                       const std::string& report);

public slots:

  void openBlueConfigThroughDialog( void );

private:

  Ui::MainWindow* _ui;

};
