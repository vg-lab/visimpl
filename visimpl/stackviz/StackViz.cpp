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
#include <QFileDialog>
#include <QInputDialog>
#include <QGridLayout>
#include <QInputDialog>
#include <QMessageBox>
#include <QShortcut>
#include <QDateTime>
#include <QtGlobal>
#include <QApplication>

#include <boost/bind.hpp>

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

void StackViz::init( simil::SimulationPlayer* p )
{
  // StackViz already loaded.
  if ( p == nullptr || _player != nullptr ) return;

  QApplication::setOverrideCursor( Qt::WaitCursor );

  _player = p;
  _simulationType = _player->data( )->simulationType( );
  _subsetEventManager = _player->data( )->subsetsEvents( );

  // TODO: events file.
  updateUIonOpen( std::string( ));

  QApplication::restoreOverrideCursor( );
}

StackViz::~StackViz( void )
{
}

void StackViz::openSubsetEventFile(const std::string &filePath, bool append)
{
  if (filePath.empty() || !_subsetEventManager) return;

  QApplication::setOverrideCursor(Qt::WaitCursor);

  if (!append) _subsetEventManager->clear();

  _summary->clearEvents();

  QString errorText;
  try
  {
    if (filePath.find("json") != std::string::npos)
    {
      _subsetEventManager->loadJSON(filePath);
    }
    else
      if (filePath.find("h5") != std::string::npos)
      {
        _subsetEventManager->loadH5(filePath);
        _autoCalculateCorrelations = true;
      }
      else
      {
        errorText = tr("Subset Events file not found: %1").arg(QString::fromStdString(filePath));
      }
  }
  catch(const std::exception &e)
  {
    if(_subsetEventManager) _subsetEventManager->clear();

    errorText = QString::fromLocal8Bit(e.what());
  }

  QApplication::restoreOverrideCursor();

  if(!errorText.isEmpty())
  {
    QMessageBox::critical(this, tr("Error loading Events file"), errorText, QMessageBox::Ok);
    return;
  }
}

void StackViz::openSubsetEventsFileThroughDialog( void )
{
    const QString eventsFilename = QFileDialog::getOpenFileName(this,
              tr( "Open file containing subsets/events data" ),
              _lastOpenedSubsetsFileName,
              tr( "JSON (*.json);; hdf5 (*.h5);; All files (*)" ),
              nullptr, QFileDialog::DontUseNativeDialog );

  if( !eventsFilename.isEmpty( ))
  {
    _lastOpenedSubsetsFileName = QFileInfo( eventsFilename ).path( );

    openSubsetEventFile( eventsFilename.toStdString( ), false );

    _summary->generateEventsRep( );
    _summary->importSubsetsFromSubsetMngr( );

    if( _displayManager )
      _displayManager->refresh( );
  }
}

void StackViz::showDisplayManagerWidget( void )
{
  if(!_summary) return;

  if( !_displayManager)
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
  _summary = new visimpl::Summary( this , visimpl::T_STACK_EXPANDABLE );

  if ( _simulationType == simil::TSimSpikes )
  {
    auto spikesPlayer = dynamic_cast< simil::SpikesPlayer * >( _player );

    _summary->Init( spikesPlayer->data( ));
    _summary->simulationPlayer( _player );
  }


  layout( )->addWidget( _summary );

  connect( _summary , SIGNAL( histogramClicked( visimpl::HistogramWidget * )) ,
           this , SLOT( HistogramClicked( visimpl::HistogramWidget * )) );

  //_ui->actionFocusOnPlayhead->setVisible( true );

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
  if (_summary)
  {
     _summary->AddNewHistogram(selection);
  }
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

void StackViz::loadComplete(void)
{
  _summary->showMarker(false);
}

void StackViz::addCorrelation(const std::string &subset)
{
  _correlations.push_back(subset);
}

void StackViz::calculateCorrelations(void)
{
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
  if(_summary)
  {
    _summary->changeHistogramName(idx + 1, name);

  }
}

void visimpl::StackViz::setHistogramVisible(const unsigned idx, const bool state)
{
  if(_summary)
  {
    _summary->changeHistogramVisibility(idx + 1, state);

  }
}

void StackViz::updateUIonOpen(const std::string &eventsFile)
{
  initSummaryWidget( );

  openSubsetEventFile( eventsFile, true );

  _summary->generateEventsRep( );
  _summary->importSubsetsFromSubsetMngr( );

  if( _displayManager )
    _displayManager->refresh( );

  //_ui->actionShowDataManager->setEnabled(true);
}

void visimpl::StackViz::updateHistograms( )
{
  if ( _summary )
    _summary->repaintHistograms( );

  if ( _followPlayhead )
    _summary->focusPlayback( );
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
