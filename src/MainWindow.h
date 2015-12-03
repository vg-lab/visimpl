#include <QMainWindow>
#include "OpenGLWidget.h"

namespace Ui
{
class MainWindow;
}

class MainWindow
  : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow( QWidget* parent = 0,
                       bool updateOnIdle = false );
  ~MainWindow( void );

  void init( const std::string& zeqUri = "" );
  void showStatusBarMessage ( const QString& message );

  void openBlueConfig( const std::string& fileName,
                       const std::string& target,
                       const std::string& report);

  void openXMLScene( const std::string& fileName );
  void openSWCFile( const std::string& fileName );


public slots:

  void openBlueConfigThroughDialog( void );
  void openXMLSceneThroughDialog( void );
  void openSWCFileThroughDialog( void );

protected:

  QString _lastOpenedFileName;


private:

  Ui::MainWindow* _ui;
  OpenGLWidget* _openGLWidget;


};
