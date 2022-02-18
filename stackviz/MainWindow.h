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

#include <QMainWindow>
#include <QDockWidget>
#include <QPushButton>
#include <QSlider>
#include <QTimer>
#include <QLabel>
#include <QGridLayout>
#include <QScrollArea>
#include <QCheckBox>

#include <sumrice/sumrice.h>
#include <simil/simil.h>

#ifdef VISIMPL_USE_ZEROEQ
#include <zeroeq/zeroeq.h>
#include <thread>
#ifdef VISIMPL_USE_LEXIS
#include <lexis/lexis.h>
#endif
#ifdef VISIMPL_USE_GMRVLEX
#include <gmrvlex/gmrvlex.h>
#endif

#endif

#include "ui_stackviz.h"
#include "DisplayManagerWidget.h"

namespace Ui
{
  class MainWindow;
}

namespace stackviz
{
  class MainWindow
  : public QMainWindow
  {
    Q_OBJECT

  public:
    explicit MainWindow( QWidget* parent = nullptr );
    ~MainWindow( void );

    void init( const std::string& zeqUri = std::string() );
    void showStatusBarMessage ( const QString& message );

    void loadData(const simil::TDataType type,
                  const std::string &arg1,
                  const std::string &arg2,
                  const simil::TSimulationType simType = simil::TSimulationType::TSimSpikes,
                  const std::string &subsetEventFile = "");

    void openSubsetEventFile( const std::string& fileName,
                              bool append = false );

  public slots:

    void openBlueConfigThroughDialog( void );
    void openCSVFilesThroughDialog( void );
    void openH5FilesThroughDialog( void );
    void openSubsetEventsFileThroughDialog( void );

    void aboutDialog( void );
    void togglePlaybackDock( void );
    void showDisplayManagerWidget( void );

    void PlayPause( bool notify = true );
    void Play( bool notify = true );
    void Pause( bool notify = true );
    void Stop( bool notify = true );
    void Repeat( bool notify = true);
    void PlayAtPosition( bool notify = true );
    void PlayAtPosition( int, bool notify = true );
    void PlayAtPercentage( float, bool notify = true );
    void PlayAtTime( float, bool notify = true);
    void Restart( bool notify = true );
    void GoToEnd( bool notify = true );

    void UpdateSimulationSlider( float percentage );

    void addCorrelation( const std::string& subset );

    void calculateCorrelations( void );

  protected slots:

  #ifdef VISIMPL_USE_GMRVLEX

    void HistogramClicked( visimpl::HistogramWidget* );
    void ApplyPlaybackOperation( unsigned int playbackOp );
    void _zeqEventRepeat( bool repeat );

  #endif

    void playAtButtonClicked( void );

    void loadComplete( void );

    void onLoadFinished();

    void onDataUpdated();

  protected:
    void configurePlayer( void );

    void initSummaryWidget( void );
    void initPlaybackDock( void );

  #ifdef VISIMPL_USE_ZEROEQ

    void _onSelectionEvent( lexis::data::ConstSelectedIDsPtr selected );

  #endif

    Ui::MainWindow* _ui;

    QString _lastOpenedFileNamePath;
    QString _lastOpenedSubsetsFileName;

    simil::TSimulationType _simulationType;

    visimpl::Summary* _summary;

    simil::SimulationPlayer* _player;
    simil::SubsetEventManager* _subsetEventManager;

    bool _autoCalculateCorrelations;

    // Playback Control
    QDockWidget* _dockSimulation;
    QPushButton* _playButton;
    CustomSlider* _simSlider;
    QPushButton* _repeatButton;
    QPushButton* _goToButton;
    bool _playing;

    QIcon _playIcon;
    QIcon _pauseIcon;

    QLabel* _startTimeLabel;
    QLabel* _endTimeLabel;

    visimpl::DisplayManagerWidget* _displayManager;

  private:
    /** \brief Helper method to update the UI after a dataset has been loaded.
     * \param[in] eventsFile Subset events filename.
     *
     */
    void updateUIonOpen(const std::string &eventsFile);

    /** \brief Wrapper aroung player operations to catch exceptions.
     * \param[in] op ZeroEQ Operation identifier.
     *
     */
    void sendZeroEQPlaybackOperation(const unsigned int op);

    /* \brief Helper method to close the loading dialog.
     *
     */
    void closeLoadingDialog();

    std::vector< std::string > _correlations;

    std::string m_subsetEventFile;
    std::shared_ptr<LoaderThread> m_loader;
    LoadingDialog *m_loaderDialog;
    DataInspector * m_dataInspector;
  };


} // namespace stackviz
