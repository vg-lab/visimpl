/*
 * @file  MainWindow.h
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */

#include <QMainWindow>
#include <QDockWidget>
#include <QPushButton>
#include <QSlider>
#include <QTimer>
#include <QRadioButton>
#include <QGroupBox>
#include <QPushButton>

#include "OpenGLWidget.h"

#include <sumrice/sumrice.h>

#include <ui_visimpl.h>

namespace Ui
{
class MainWindow;
}

namespace visimpl
{

  class MainWindow
    : public QMainWindow
  {
    Q_OBJECT

  public:
    explicit MainWindow( QWidget* parent = 0,
                         bool updateOnIdle = true );
    ~MainWindow( void );

    void init( const std::string& zeqUri = "" );
    void showStatusBarMessage ( const QString& message );

    void openBlueConfig( const std::string& fileName,
                         simil::TSimulationType simulationType,
                         const std::string& report,
                         const std::string& subsetEventFile = "");

    void openHDF5File( const std::string& networkFile,
                       simil::TSimulationType simulationType,
                       const std::string& activityFile = "",
                       const std::string& subsetEventFile = "" );

    void openCSVFile( const std::string& networkFile,
                      simil::TSimulationType simulationType,
                      const std::string& activityFile = "",
                      const std::string& subsetEventFile = "" );

  public slots:

    void openBlueConfigThroughDialog( void );
    void openHDF5ThroughDialog( void );

    void openSubsetEventFile( const std::string& fileName,
                              bool append = false );

    void aboutDialog( void );
    void togglePlaybackDock( void );
    void toggleSimConfigDock( void );

    void PlayPause( bool notify = true );
    void Play( bool notify = true );
    void Pause( bool notify = true );
    void Stop( bool notify = true );
    void Repeat( bool notify = true );
    void PlayAt( bool notify = true );
    void PlayAt( float, bool notify = true );
    void PlayAt( int, bool notify = true );
    void requestPlayAt( float );
    void Restart( bool notify = true );
    void GoToEnd( bool notify = true );

    void UpdateSimulationSlider( float percentage );

    void UpdateSimulationColorMapping( void );
    void PreviewSimulationColorMapping( void );
    void changeEditorColorMapping( void );
    void changeEditorSizeFunction( void );
    void UpdateSimulationSizeFunction( void );
    void PreviewSimulationSizeFunction( void );

    void changeEditorSimDeltaTime( void );
    void updateSimDeltaTime( void );

    void changeEditorSimTimestepsPS( void );
    void updateSimTimestepsPS( void );

    void showSelection( bool show );

    void changeEditorDecayValue( void );
    void updateSimulationDecayValue( void );

    void AlphaBlendingToggled( void );

  protected slots:

    void addGroupFromSelection( void );
    void checkGroupsVisibility( void );

  #ifdef VISIMPL_USE_ZEROEQ
  #ifdef VISIMPL_USE_GMRVLEX

    void ApplyPlaybackOperation( unsigned int playbackOp );
    void _zeqEventRepeat( bool repeat );

  #endif

    void ClearSelection( void );
    void playAtButtonClicked( void );

  protected:

    void _onSelectionEvent( lexis::data::ConstSelectedIDsPtr );
    void _setZeqUri( const std::string& );
    bool _zeqConnection;

    std::string _uri;
    zeroeq::Subscriber* _subscriber;

    std::thread* _thread;

  #endif // VISIMPL_USE_ZEROEQ

  protected:
    void configurePlayer( void );

    Ui::MainWindow* _ui;

    QString _lastOpenedFileName;
    QIcon _playIcon;
    QIcon _pauseIcon;

  private:

    void initPlaybackDock( void );
    void initSimControlDock( void );
    void initSummaryWidget( void );

    OpenGLWidget* _openGLWidget;
    visimpl::Summary* _summary;

    scoop::ColorPalette _colorPalette;

    QDockWidget* _simulationDock;
    QSlider* _simSlider;
    QPushButton* _playButton;
    QLabel* _startTimeLabel;
    QLabel* _endTimeLabel;
    QPushButton* _repeatButton;
    QPushButton* _goToButton;

    QDockWidget* _simConfigurationDock;

    QGroupBox* _tfGroupBox;
    TransferFunctionEditor* _tfEditor;
    TransferFunctionWidget* _tfWidget;

    bool _autoNameGroups;
    QGroupBox* _groupsGroupBox;
    QGridLayout* _groupLayout;
    std::vector< QCheckBox* > _groupsVisButtons;

    QDoubleSpinBox* _decayBox;
    QDoubleSpinBox* _deltaTimeBox;
    QDoubleSpinBox* _timeStepsPSBox;

    QPushButton* _addGroupButton;
    QPushButton* _clearSelectionButton;
    QLabel* _selectionSizeLabel;

    QRadioButton* _alphaNormalButton;
    QRadioButton* _alphaAccumulativeButton;
  };

} // namespace visimpl
