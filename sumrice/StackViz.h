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

#ifndef STACKVIZ_H_
#define STACKVIZ_H_

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

namespace visimpl
{

  class DisplayManagerWidget;

  class StackViz
  : public QWidget
  {
    Q_OBJECT

  public:
    explicit StackViz( QWidget* parent = nullptr );
    virtual ~StackViz( void );

    void init( simil::SimulationPlayer* p );

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

    /** \brief Shows/hides the given histogram.
     * \param[in] idx Histogram index.
     * \param[in] state True to make visible and false to hide it.
     *
     */
    void setHistogramVisible(const unsigned idx, const bool state);

  signals:
    void changedBins(const unsigned int);

  public slots:

    void openSubsetEventsFile( bool fromH5 );

    void showDisplayManagerWidget( void );

    void calculateCorrelations( void );

    void updateHistograms( );

    void repaintHistograms();

    void toggleAutoNameSelections( );

    void fillPlots( bool fill );

    void focusPlayback( );

    void followPlayhead ( bool follow );

    /** \brief Shows/Hides the StackViz configuration panels.
     * \param[in] value True to show the panels and false otherwise.
     *
     */
    void showStackVizPanels( bool value);

  protected slots:

    void HistogramClicked( visimpl::HistogramWidget * );

  protected:

    void initSummaryWidget( void );

    simil::TSimulationType _simulationType;

    visimpl::Summary* _summary;

    simil::SimulationPlayer* _player;
    simil::SubsetEventManager* _subsetEventManager;

    bool _autoCalculateCorrelations;
    bool _followPlayhead;

    DisplayManagerWidget* _displayManager;

  private:

    std::vector< std::string > _correlations;
  };


} // namespace stackviz

#endif //STACKVIZ_H_
