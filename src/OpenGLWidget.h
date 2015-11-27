#ifndef __QNEUROLOTS__OPENGLWIDGET__
#define __QNEUROLOTS__OPENGLWIDGET__

#include <QOpenGLFunctions>
#include <QOpenGLWidget>
#include <QLabel>
#include <chrono>

#define NEUROLOTS_SKIP_GLEW_INCLUDE 1

#define PREFR_SKIP_GLEW_INCLUDE 1
#include <prefr/prefr.h>

#include "CShader.h"

#include <neurolots/nlrender/nlrender.h>

class OpenGLWidget
  : public QOpenGLWidget
  , public QOpenGLFunctions
{

  Q_OBJECT;

public:

  typedef enum
  {
    BlueConfig,
    SWC,
    NsolScene
  } TDataFileType;

  OpenGLWidget( QWidget* parent = 0,
                Qt::WindowFlags windowFlags = 0,
                bool paintNeurons = true,
                const std::string& zeqUri = "" );
  ~OpenGLWidget( void );

  void createNeuronsCollection( void );
  void createParticleSystem( void );
  void loadData( const std::string& fileName,
                 const TDataFileType fileType = TDataFileType::BlueConfig,
                 const std::string& target = std::string( "" ));

  void idleUpdate( bool idleUpdate_ = true )
  {
    _idleUpdate = idleUpdate_;
  }

public slots:

  void togglePaintNeurons( void );
  void changeClearColor( void );
  void toggleUpdateOnIdle( void );
  void toggleShowFPS( void );
  void toggleWireframe( void );


protected:

  virtual void initializeGL( void );
  virtual void paintGL( void );
  virtual void resizeGL( int w, int h );

  virtual void mousePressEvent( QMouseEvent* event );
  virtual void mouseReleaseEvent( QMouseEvent* event );
  virtual void wheelEvent( QWheelEvent* event );
  virtual void mouseMoveEvent( QMouseEvent* event );
  virtual void keyPressEvent( QKeyEvent* event );

  void paintParticles( void );

  QLabel _fpsLabel;
  bool _showFps;

  bool _wireframe;

  neurolots::Camera* _camera;
  neurolots::NeuronsCollection* _neuronsCollection;
  bool _paintNeurons;

  unsigned int _frameCount;

  int _mouseX, _mouseY;
  bool _rotation, _translation;

  bool _idleUpdate;
  bool _paint;

  QColor _currentClearColor;

  std::chrono::time_point< std::chrono::system_clock > _then;

  CShader* _particlesShader;
  prefr::ParticleSystem* _ps;

};

#endif // __QNEUROLOTS__OPENGLWIDGET__
