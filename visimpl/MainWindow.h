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
 
#ifdef WIN32
#include <winsock2.h>
#endif

// ViSimpl
#include <ui_visimpl.h>
#include "OpenGLWidget.h"
#include "VisualGroup.h"
#include "SelectionManagerWidget.h"
#include "SubsetImporter.h"
#include "ui/DataInspector.h"

// Qt
#include <QMainWindow>

// Sumrice
#include <sumrice/sumrice.h>

class QDockWidget;
class QPushButton;
class QSlider;
class QTimer;
class QRadioButton;
class QGroupBox;
class QPushButton;
class QToolBox;

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
    explicit MainWindow( QWidget* parent = nullptr,
                         bool updateOnIdle = true );
    virtual ~MainWindow( void );

    void init( const std::string& zeqUri = std::string() );
    void showStatusBarMessage ( const QString& message );

    void loadData(const simil::TDataType type,
                  const std::string arg_1,
                  const std::string arg_2,
                  const simil::TSimulationType simType = simil::TSimulationType::TSimSpikes,
                  const std::string &subsetEventFile = std::string());

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

    /** \brief Executed when OpenGlWidget reports finished loading data.
     *
     */
    void onDataLoaded();

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

    std::string _zeqUri;
    zeroeq::Subscriber* _subscriber;

    std::thread* _thread;

  #endif // VISIMPL_USE_ZEROEQ

    /** \brief Wrapper aroung player operations to catch exceptions.
     * \param[in] op ZeroEQ Operation identifier.
     *
     */
    void sendZeroEQPlaybackOperation(const unsigned int op);

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

#ifdef SIMIL_WITH_REST_API
    DataInspector * _objectInspectorGB;
#endif

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

    std::string m_subsetEventFile;
    simil::TDataType m_type;
  };
} // namespace visimpl
