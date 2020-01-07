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
#include <QToolBox>

#include "OpenGLWidget.h"
#include "SelectionManagerWidget.h"
#include "SubsetImporter.h"
#include "ui/DataInspector.h"

#include <sumrice/sumrice.h>

#include <ui_visimpl.h>

namespace Ui
{
class MainWindow;
}

namespace visimpl
{

  enum TSelectionSource
  {
    SRC_EXTERNAL = 0,
    SRC_PLANES,
    SRC_WIDGET,
    SRC_UNDEFINED
  };

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

    void openRestListener( const std::string& url,
                      simil::TSimulationType simulationType,
                      const std::string& port = "",
                      const std::string& subsetEventFile = "" );

  public slots:

    void openBlueConfigThroughDialog( void );
    void openCSVFilesThroughDialog( void );
    void openHDF5ThroughDialog( void );
    void openSubsetEventsFileThroughDialog( void );

    void openSubsetEventFile( const std::string& fileName,
                              bool append = false );

    void closeData( void );

    void dialogAbout( void );
    void dialogSelectionManagement( void );
    void dialogSubsetImporter( void );

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

    void setCircuitSizeScaleFactor( vec3 );
    vec3 getCircuitSizeScaleFactor( void ) const;

    void showInactive( bool show );

    void changeCircuitScaleValue( void );
    void updateCircuitScaleValue( void );

    void changeEditorDecayValue( void );
    void updateSimulationDecayValue( void );

    void changeEditorStepByStepDuration( void );
    void updateSimStepByStepDuration( void );

    void AlphaBlendingToggled( void );

    void updateAttributeStats( void );

    void updateSelectedStatsPickingSingle( unsigned int selected );

    void clippingPlanesActive ( int state );

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

    void configureComponents( void );
    void importVisualGroups( void );

    void addGroupControls( const std::string& name, unsigned int index,
                           unsigned int size );
    void removeGroupControls( unsigned int index );
    void clearGroups( void );

    void addGroupFromSelection( void );
    void checkGroupsVisibility( void );
    void checkAttributeGroupsVisibility( void );

    void spinBoxValueChanged( void );

    void completedStep( void );
    void playAtButtonClicked( void );

    void clippingPlanesReset( void );

    void colorSelectionClicked( void );

    void selectionManagerChanged( void );
    void setSelection( const GIDUSet& selection_, TSelectionSource source_ = SRC_UNDEFINED );
    void clearSelection( void );
    void selectionFromPlanes( void );

  protected:

    void _initSimControlDock( void );
    void _initPlaybackDock( void );
    void _initSummaryWidget( void );

    void _configurePlayer( void );
    void _resetClippingParams( void );

    void _updateSelectionGUI( void );

    bool _showDialog( QColor& current, const QString& message = "" );


  #ifdef VISIMPL_USE_ZEROEQ
  #ifdef VISIMPL_USE_GMRVLEX

  protected slots:

    void ApplyPlaybackOperation( unsigned int playbackOp );
    void _zeqEventRepeat( bool repeat );

  #endif


  protected:

    void _onSelectionEvent( lexis::data::ConstSelectedIDsPtr );
    void _setZeqUri( const std::string& );
    bool _zeqConnection;

    std::string _uri;
    zeroeq::Subscriber* _subscriber;

    std::thread* _thread;

  #endif // VISIMPL_USE_ZEROEQ

    Ui::MainWindow* _ui;

    QString _lastOpenedNetworkFileName;
    QString _lastOpenedActivityFileName;
    QString _lastOpenedSubsetsFileName;
    QIcon _playIcon;
    QIcon _pauseIcon;

    OpenGLWidget* _openGLWidget;
    DomainManager* _domainManager;
    simil::SubsetEventManager* _subsetEvents;
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

    QTabWidget* _modeSelectionWidget;
    QToolBox* _toolBoxOptions;

    DataInspector * _objectInspectoGB;

    QGroupBox* _groupBoxTransferFunction;
    TransferFunctionWidget* _tfWidget;
    SelectionManagerWidget* _selectionManager;
    SubsetImporter* _subsetImporter;

    enum TGroupRow { gr_container = 0, gr_checkbox };
    typedef std::tuple< QWidget*, QCheckBox* > tGroupRow;

    bool _autoNameGroups;
    QVBoxLayout* _groupLayout;
    std::vector< tGroupRow > _groupsVisButtons;

    QDoubleSpinBox* _decayBox;
    QDoubleSpinBox* _deltaTimeBox;
    QDoubleSpinBox* _timeStepsPSBox;
    QDoubleSpinBox* _stepByStepDurationBox;
    QDoubleSpinBox* _circuitScaleX;
    QDoubleSpinBox* _circuitScaleY;
    QDoubleSpinBox* _circuitScaleZ;

    QPushButton* _buttonImportGroups;
    QPushButton* _buttonClearGroups;
    QPushButton* _buttonAddGroup;
    QPushButton* _buttonClearSelection;
    QLabel* _selectionSizeLabel;

    QRadioButton* _alphaNormalButton;
    QRadioButton* _alphaAccumulativeButton;

    QLabel* _labelGID;
    QLabel* _labelPosition;

    QGroupBox* _groupBoxAttrib;
    QComboBox* _comboAttribSelection;
    QVBoxLayout* _layoutAttribStats;
    QGridLayout* _layoutAttribGroups;
    std::vector< QCheckBox* > _attribGroupsVisButtons;

    // Clipping planes
    QCheckBox* _checkClipping;
    QCheckBox* _checkShowPlanes;
    QPushButton* _buttonResetPlanes;
    QDoubleSpinBox* _spinBoxClippingHeight;
    QDoubleSpinBox* _spinBoxClippingWidth;
    QDoubleSpinBox* _spinBoxClippingDist;
    QPushButton* _frameClippingColor;
    QPushButton* _buttonSelectionFromClippingPlanes;

  };

} // namespace visimpl
