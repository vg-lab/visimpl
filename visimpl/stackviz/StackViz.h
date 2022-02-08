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

#include "ui_StackVizGui.h"

#include "DisplayManagerWidget.h"

namespace Ui
{
  class StackVizGui;
}

namespace visimpl
{
  class StackViz
  : public QMainWindow
  {
    Q_OBJECT

  public:
    explicit StackViz( QWidget* parent = nullptr );
    virtual ~StackViz( void );

    void init( simil::SimulationPlayer* p );
    void showStatusBarMessage ( const QString& message );

    void openSubsetEventFile( const std::string& fileName,
                              bool append = false );

    /** \brief Adds an histogram for the given ids.
     * \param[in] selection Selected indexes.
     */
    void addSelection(const visimpl::Selection &selection);

    /** \brief Removes the histogram with the given index (removing a selection):
     *  \param[in] idx Histogram index.
     *
     */
    void removeSubset(const unsigned int idx);

    /** \brief Changes the name of the histogram.
     * \param[in] idx Histogram index.
     * \param[in] name New name.
     *
     */
    void changeHistogramName(const unsigned idx, const QString &name);

  public slots:

    void openSubsetEventsFileThroughDialog( void );

    void showDisplayManagerWidget( void );

    void addCorrelation( const std::string& subset );

    void calculateCorrelations( void );

    void updateHistograms();

  protected slots:

    void HistogramClicked( visimpl::HistogramWidget* );

    void loadComplete( void );

  protected:

    virtual void closeEvent(QCloseEvent *e);

    void initSummaryWidget( void );

    Ui::StackVizGui* _ui;

    QString _lastOpenedFileNamePath;
    QString _lastOpenedSubsetsFileName;

    simil::TSimulationType _simulationType;

    visimpl::Summary* _summary;

    simil::SimulationPlayer* _player;
    simil::SubsetEventManager* _subsetEventManager;

    bool _autoCalculateCorrelations;

    DisplayManagerWidget* _displayManager;

  private:
    /** \brief Helper method to update the UI after a dataset has been loaded.
     * \param[in] eventsFile Subset events filename.
     *
     */
    void updateUIonOpen(const std::string &eventsFile);

    std::vector< std::string > _correlations;
  };


} // namespace stackviz
