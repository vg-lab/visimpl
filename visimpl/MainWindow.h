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


  public slots:

    void openBlueConfigThroughDialog( void );
    void openHDF5ThroughDialog( void );

    void openSubsetEventFile( const std::string& fileName,
                              bool append = false );

    void aboutDialog( void );

    void togglePlaybackDock( void );
    void toggleSimConfigDock( void );

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

    void showInactive( bool show );

    void changeEditorDecayValue( void );
    void updateSimulationDecayValue( void );

    void changeEditorStepByStepDuration( void );
    void updateSimStepByStepDuration( void );

    void AlphaBlendingToggled( void );

    void updateAttributeStats( void );

    void updateSelectedStatsPickingSingle( unsigned int selected );

    void PlayPause( bool notify = true );
    void Play( bool notify = true );
    void Pause( bool notify = true );
    void Stop( bool notify = true );
    void Repeat( bool notify = true );
    void PlayAt( bool notify = true );
    void PlayAt( float, bool notify = true );
    void PlayAt( int, bool notify = true );
    void requestPlayAt( float );
    void PreviousStep( bool notify = true );
    void NextStep( bool notify = true );

  protected slots:

    void addGroupFromSelection( void );
    void checkGroupsVisibility( void );
    void checkAttributeGroupsVisibility( void );

    void completedStep( void );
    void playAtButtonClicked( void );

  protected:

    void _initSimControlDock( void );
    void _initPlaybackDock( void );
    void _initSummaryWidget( void );

    void _configurePlayer( void );


  #ifdef VISIMPL_USE_ZEROEQ
  #ifdef VISIMPL_USE_GMRVLEX

  protected slots:

    void ApplyPlaybackOperation( unsigned int playbackOp );
    void _zeqEventRepeat( bool repeat );

  #endif

    void ClearSelection( void );

  protected:

    void _onSelectionEvent( lexis::data::ConstSelectedIDsPtr );
    void _setZeqUri( const std::string& );
    bool _zeqConnection;

    std::string _uri;
    zeroeq::Subscriber* _subscriber;

    std::thread* _thread;

  #endif // VISIMPL_USE_ZEROEQ

    Ui::MainWindow* _ui;

    QString _lastOpenedFileName;
    QIcon _playIcon;
    QIcon _pauseIcon;

    OpenGLWidget* _openGLWidget;
    DomainManager* _domainManager;
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

    QGroupBox* _groupBoxTransferFunction;
    TransferFunctionEditor* _tfEditor;
    TransferFunctionWidget* _tfWidget;

    bool _autoNameGroups;
    QGroupBox* _groupBoxGroups;
    QGridLayout* _groupLayout;
    std::vector< QCheckBox* > _groupsVisButtons;

    QDoubleSpinBox* _decayBox;
    QDoubleSpinBox* _deltaTimeBox;
    QDoubleSpinBox* _timeStepsPSBox;
    QDoubleSpinBox* _stepByStepDurationBox;

    QPushButton* _addGroupButton;
    QPushButton* _clearSelectionButton;
    QLabel* _selectionSizeLabel;

    QRadioButton* _alphaNormalButton;
    QRadioButton* _alphaAccumulativeButton;

    QGroupBox* _groupBoxAttrib;
    QComboBox* _comboAttribSelection;
    QVBoxLayout* _layoutAttribStats;
    QGridLayout* _layoutAttribGroups;
    std::vector< QCheckBox* > _attribGroupsVisButtons;

  };

} // namespace visimpl
