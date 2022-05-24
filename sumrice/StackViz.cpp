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

#include "StackViz.h"
#include <QDebug>
#include <QInputDialog>
#include <QGridLayout>
#include <QShortcut>
#include <QDateTime>
#include <QtGlobal>
#include <QApplication>

#include <thread>

#include <sumrice/sumrice.h>

template<class T> void ignore( const T& ) { }

using namespace visimpl;

StackViz::StackViz( QWidget *parent_ )
  : QWidget( parent_ )
  , _simulationType( simil::TSimNetwork )
  , _summary( nullptr )
  , _player( nullptr )
  , _subsetEventManager( nullptr )
  , _autoCalculateCorrelations( false )
  , _followPlayhead( false )
  , _displayManager( nullptr )
{
  setLayout( new QGridLayout( ));
}

void StackViz::init( simil::SimulationPlayer* player, simil::SubsetEventManager *manager )
{
  if ( !player ) return;

  QApplication::setOverrideCursor( Qt::WaitCursor );

  _player = player;
  _simulationType = _player->data( )->simulationType( );
  _subsetEventManager = manager;

  // TODO: events file.
  initSummaryWidget( );

  QApplication::restoreOverrideCursor( );
}

void StackViz::openSubsetEventsFile( bool fromH5 )
{
  if(_summary)
  {
    _summary->clearEvents();
    _summary->generateEventsRep();
    _autoCalculateCorrelations = fromH5;
  }

  if (_displayManager) _displayManager->refresh();
}

void StackViz::showDisplayManagerWidget( )
{
  if(!_summary) return;

  if(!_displayManager)
  {
    _displayManager = new DisplayManagerWidget( );
    _displayManager->init( _summary->eventWidgets(),
                           _summary->histogramWidgets( ));

    connect( _displayManager, SIGNAL( eventVisibilityChanged( unsigned int, bool )),
             _summary, SLOT( eventVisibility( unsigned int, bool )));

    connect( _displayManager, SIGNAL( subsetVisibilityChanged( unsigned int, bool )),
             _summary, SLOT( subsetVisibility( unsigned int, bool )));

    connect( _displayManager, SIGNAL( removeEvent( unsigned int )),
             _summary, SLOT( removeEvent( unsigned int )));

    connect( _displayManager, SIGNAL( removeHistogram( unsigned int )),
             _summary, SLOT( removeSubset( unsigned int )));

  }

  _displayManager->refresh( );

  _displayManager->show( );
}

void StackViz::initSummaryWidget( )
{
  if(!_player) return;

  const bool hasSummary = _summary != nullptr;

  if(!hasSummary)
  {
    _summary = new visimpl::Summary( this , visimpl::T_STACK_EXPANDABLE );
  }

  if ( _simulationType == simil::TSimSpikes )
  {
    auto spikesPlayer = dynamic_cast< simil::SpikesPlayer * >( _player );

    _summary->Init( spikesPlayer->data( ));
    _summary->simulationPlayer( _player );
  }

  if(!hasSummary)
  {
    layout( )->addWidget( _summary );

    connect( _summary , SIGNAL( histogramClicked( visimpl::HistogramWidget * )) ,
             this , SLOT( HistogramClicked( visimpl::HistogramWidget * )) );

    connect( _summary, SIGNAL(changedBins(const unsigned int)),
             this, SIGNAL(changedBins(const unsigned int)));
  }

  if ( _autoCalculateCorrelations )
  {
    calculateCorrelations( );
  }

  QTimer::singleShot( 0 , _summary , SLOT( adjustSplittersSize( )) );
}

void StackViz::HistogramClicked(visimpl::HistogramWidget *histogram)
{
#ifdef VISIMPL_USE_ZEROEQ
  const visimpl::GIDUSet *selection;

  if (histogram->filteredGIDs().size() == 0)
    selection = &_summary->gids();
  else
    selection = &histogram->filteredGIDs();

  std::vector<uint32_t> selected(selection->begin(), selection->end());

  try
  {
    auto &zInstance = visimpl::ZeroEQConfig::instance();
    if(zInstance.isConnected())
    {
      zInstance.publisher()->publish(lexis::data::SelectedIDs(selected));
    }
  }
  catch(std::exception &e)
  {
    std::cerr << "Exception sending histogram id event. " << e.what() << ". "
              << __FILE__ << ":" << __LINE__ << std::endl;
  }
  catch(...)
  {
    std::cerr << "Unknown exception sending histogram id event."
              << __FILE__ << ":" << __LINE__ << std::endl;
  }
#else
  ignore(histogram);
#endif
}

void StackViz::addSelection(const visimpl::Selection &selection)
{
  if (_summary) _summary->AddNewHistogram(selection);
}

void StackViz::removeSubset(const unsigned int i)
{
  // Histogram 0 is always gonna be present. Selections are 1-N.
  const auto idx = i+1;
  if(_summary && idx < _summary->histogramsNumber())
  {
    _summary->removeSubset(idx);
  }
}

void StackViz::calculateCorrelations(void)
{
  if(!_player || !_subsetEventManager) return;

  visimpl::CorrelationComputer cc(dynamic_cast<simil::SpikeData*>(_player->data()));

  const auto eventNames = _subsetEventManager->eventNames();

  constexpr double deltaTime = 0.125;

  cc.configureEvents(eventNames, deltaTime);

  auto correlateSubsets = [&eventNames, &cc](const std::string &event)
  {
    cc.correlateSubset( event, eventNames, deltaTime, 2600, 2900 );
  };
  std::for_each(_correlations.cbegin(), _correlations.cend(), correlateSubsets);

  const auto names = cc.correlationNames();

  auto addHistogram = [this, &cc](const std::string &name)
  {
    auto correlation = cc.correlation( name );

    if( !correlation ) return;

    visimpl::Selection selection;
    selection.name = correlation->fullName;
    selection.gids = cc.getCorrelatedNeurons( correlation->fullName );

    _summary->AddNewHistogram( selection );
  };
  std::for_each(names.cbegin(), names.cend(), addHistogram);
}

void visimpl::StackViz::changeHistogramName(const unsigned idx, const QString &name)
{
  if(_summary) _summary->changeHistogramName(idx + 1, name);
}

void visimpl::StackViz::setHistogramVisible(const unsigned idx, const bool state)
{
  if(_summary) _summary->changeHistogramVisibility(idx + 1, state);
}

void visimpl::StackViz::updateHistograms( )
{
  if ( _summary ) _summary->UpdateHistograms( );

  if ( _followPlayhead ) _summary->focusPlayback( );
}

void visimpl::StackViz::toggleAutoNameSelections( )
{
  if ( _summary ) _summary->toggleAutoNameSelections( );
}

void visimpl::StackViz::fillPlots( bool fill )
{
  if ( _summary ) _summary->fillPlots( fill );
}

void visimpl::StackViz::focusPlayback( )
{
  if ( _summary ) _summary->focusPlayback( );
}

void StackViz::followPlayhead( bool follow )
{
  _followPlayhead = follow;
}

void visimpl::StackViz::showStackVizPanels(bool value)
{
  if(_summary) _summary->showConfigPanels(value);
}

void visimpl::StackViz::repaintHistograms()
{
  if(_summary)
  {
    if(_followPlayhead) _summary->focusPlayback();

    _summary->repaintHistograms();
  }
}

void visimpl::StackViz::closeData()
{
  _player = nullptr;
  _subsetEventManager = nullptr;

  // remove histograms
  if(_summary)
  {
    while(_summary->histogramsNumber() > 1)
      _summary->removeSubset(1);

    layout()->removeWidget(_summary);
    _summary->deleteLater();
    _summary = nullptr;
  }

  if(_displayManager)
  {
    if(_displayManager->isVisible()) _displayManager->hide();
    _displayManager->deleteLater();
    _displayManager = nullptr;
  }

  _correlations.clear();
}
