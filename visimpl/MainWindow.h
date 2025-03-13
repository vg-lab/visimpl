/*
 * Copyright (c) 2015-2022 VG-Lab/URJC.
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

#include "sumrice/ConnectRESTDialog.h"
#ifdef WIN32
#include <winsock2.h>
#endif

// ViSimpl
#include <ui_visimpl.h>
#include "OpenGLWidget.h"
#include "VisualGroup.h"
#include "SelectionManagerWidget.h"
#include "SubsetImporter.h"

// Qt
#include <QMainWindow>

// Sumrice
#include <sumrice/sumrice.h>

// C++
#include <memory>

class Recorder;

class QDockWidget;

class QPushButton;

class QSlider;

class QTimer;

class QRadioButton;

class QGroupBox;

class QPushButton;

class QToolBox;

class QCloseEvent;

class LoadingDialog;

class LoaderThread;

namespace Ui
{
  class MainWindow;
}

namespace visimpl
{
  class StackViz;

  enum TSelectionSource
  {
    SRC_EXTERNAL = 0 ,
    SRC_PLANES ,
    SRC_WIDGET ,
    SRC_UNDEFINED
  };

  class MainWindow
    : public QMainWindow
  {
  Q_OBJECT

  public:
    explicit MainWindow( QWidget* parent = nullptr ,
                         bool updateOnIdle = true );

    virtual ~MainWindow( void );

    void init( const std::string& zeqUri = std::string( ));

    void showStatusBarMessage( const QString& message );

    void loadData( const simil::TDataType type ,
                   const std::string arg_1 ,
                   const std::string arg_2 ,
                   const simil::TSimulationType simType = simil::TSimulationType::TSimSpikes ,
                   const std::string& subsetEventFile = std::string( ));

#ifdef SIMIL_WITH_REST_API

    /** \brief Connects and loads data using the given REST connection.
     * \param[in] config REST connection configuration.
     *
     */
    void loadRESTData( const simil::LoaderRestData::Configuration& config );

#endif

  public slots:

    void openBlueConfigThroughDialog( void );

    void openCSVFilesThroughDialog( void );

    void openHDF5ThroughDialog( void );

    void openRESTThroughDialog( );

    void openSubsetEventsFileThroughDialog( void );

    void openSubsetEventFile( const std::string& fileName ,
                              bool append = false );

    void openRecorder( void );

    bool closeData( void );

    void dialogAbout( void );

    void dialogSelectionManagement( void );

    void dialogSubsetImporter( void );

    void togglePlaybackDock( void );

    void toggleSimConfigDock( void );

    void toggleStackVizDock( void );

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

    void clippingPlanesActive( int state );

    void PlayPause( bool notify = true );

    void Play( bool notify = true );

    void Pause( bool notify = true );

    void Stop( bool notify = true );

    void Repeat( bool notify = true );

    void PlayAtPosition( bool notify = true );

    void PlayAtPosition( int , bool notify = true );

    void PlayAtPercentage( float , bool notify = true );

    void PlayAtTime( float , bool notify = true );

    void requestPlayAt( float );

    void PreviousStep( bool notify = true );

    void NextStep( bool notify = true );

  protected slots:

    void configureComponents( void );

    void importVisualGroups( void );

    void addGroupControls( std::shared_ptr< VisualGroup > group ,
                           unsigned int index ,
                           unsigned int size ,
                           QColor groupColor = QColor{ 0 , 0 , 0 } );

    void clearGroups( void );

    void addGroupFromSelection( void );

    void checkGroupsVisibility( void );

    void spinBoxValueChanged( void );

    void completedStep( void );

    void playAtButtonClicked( void );

    void clippingPlanesReset( void );

    void colorSelectionClicked( void );

    /** \brief Updates group selection color.
     *
     */
    void onGroupColorChanged( );

    /** \brief Updates group selection color.
     *
     */
    void onGroupPreview( );

    /** \brief Shows the dialog to change the group name.
     *
     */
    void onGroupNameClicked( );

    /** \brief Removes the group.
     *
     */
    void onGroupDeleteClicked( );

    /** \brief Removes the visual group with the given index.
     * \param[in] name Name of the group to remove.
     *
     */
    void removeVisualGroup( const QString& name );

    void selectionManagerChanged( void );

    void setSelection( const GIDUSet& selection_ ,
                       TSelectionSource source_ = SRC_UNDEFINED );

    void clearSelection( void );

    void selectionFromPlanes( void );

    /** \brief Executed when OpenGlWidget reports finished loading data.
     *
     */
    void onDataLoaded( );

    /** \brief Loads groups and its properties from a file on disk.
     *
     */
    void loadGroups( );

    /** \brief Saves current groups and its properties to a file on disk.
     *
     */
    void saveGroups( );

    /** \brief Enables/disables the toolbar buttons related to stackviz widget.
     * \param[in] status True to enable and false to disable.
     *
     */
    void changeStackVizToolbarStatus( bool status );

    /** \brief Updates the UI after a recording has finished.
     *
     */
    void finishRecording( );

    /** \brief Loads camera positions from a file.
     *
     */
    void loadCameraPositions( );

    /** \brief Saves camera positions to a file on disk.
     *
     */
    void saveCameraPositions( );

    /** \brief Stores current camera position in the positions list.
     *
     */
    void addCameraPosition( );

    /** \brief Lets the user select a position to remove from the positions list.
     *
     */
    void removeCameraPosition( );

    /** \brief Changes the camera position to the one specified by the user.
     *
     */
    void applyCameraPosition( );

    /** \brief Shows a dialog with REST API options and applies them.
     *
     */
    void configureREST( );

    /** \brief Updates the color of the clipping planes color button.
     *
     */
    void changePlanesColor( const QColor& );

    /** \brief Takes a screenshot of the 3d view and shows a dialog to resize before saving. 
     * 
     */
    void saveScreenshot();

    /** \brief Sets/removes the presentation mode.
     * 
     */
    void presentationMode();

    /** \brief Enables/Disables fullscreen.
     * 
     */
    void toggleFullscreen();

  protected:
    virtual bool eventFilter(QObject *obj, QEvent *event);

    void _initSimControlDock(void);

    void _initPlaybackDock( void );

    void _initStackVizDock( void );

    void _initSummaryWidget( void );

    void _configurePlayer( void );

    void _resetClippingParams( void );

    void _updateSelectionGUI( void );

    bool _showDialog( QColor& current , const QString& message = "" );

    /** \brief Helper to update a group colors and size.
     * \param[in] name the name of the group.
     * \param[in] t Transfer function.
     * \param[in] s Size function.
     *
     */
    void updateGroup( std::string name , const TTransferFunction& t ,
                      const TSizeFunction& s );

    virtual void closeEvent( QCloseEvent* e ) override;

    /** \brief Closes the data loading dialog.
     *
     */
    void closeLoadingDialog( );

#ifdef VISIMPL_USE_ZEROEQ
#ifdef VISIMPL_USE_GMRVLEX

    protected slots:

      void ApplyPlaybackOperation( unsigned int playbackOp );

      void _zeqEventRepeat( bool repeat );

#endif

    protected:
      void _onSelectionEvent( lexis::data::ConstSelectedIDsPtr );

#endif // VISIMPL_USE_ZEROEQ

    /** \brief Wrapper aroung player operations to catch exceptions.
     * \param[in] op ZeroEQ Operation identifier.
     *
     */
    void sendZeroEQPlaybackOperation( const unsigned int op );


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

    QDockWidget* _stackVizDock;
    StackViz* _stackViz;

    QTabWidget* _modeSelectionWidget;
    QToolBox* _toolBoxOptions;

    DataInspector* _objectInspectorGB;

    QGroupBox* _groupBoxTransferFunction;
    TransferFunctionWidget* _tfWidget;
    SelectionManagerWidget* _selectionManager;
    SubsetImporter* _subsetImporter;

    enum TGroupRow
    {
      gr_container = 0 , gr_checkbox
    };
    typedef std::tuple< QWidget* , QCheckBox* > tGroupRow;

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
    QPushButton* _buttonLoadGroups;
    QPushButton* _buttonSaveGroups;
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

    Recorder* _recorder; /** Recorder */

    std::shared_ptr< LoaderThread > m_loader; /** data loader thread. */
    LoadingDialog* m_loaderDialog;          /** data loader dialog. */

#ifdef SIMIL_WITH_REST_API

    simil::LoaderRestData::Configuration _restConnectionInformation;
    bool _alreadyConnected;

#endif

  };
} // namespace visimpl
