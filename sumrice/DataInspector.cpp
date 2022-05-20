/*
 * Copyright (c) 2015-2020 VG-Lab/URJC.
 *
 * Authors: Aaron Sujar <aaron.sujar@urjc.es>
 *
 * This file is part of SimIL <https://github.com/gmrvvis/SimIL>
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

// ViSimpl
#include "DataInspector.h"

// Simil
#include <simil/SpikesPlayer.h>

// Qt
#include <QGridLayout>
#include <QLabel>

constexpr int DEFAULT_CKECK_INTERVAL = 5000;

DataInspector::DataInspector( const QString& title, QWidget* parent )
  : QGroupBox( title, parent )
  , _gidsize( 0 )
  , _spikesize( 0 )
  , _labelGIDs( nullptr )
  , _labelSpikes( nullptr )
  , _labelStartTime( nullptr )
  , _labelEndTime( nullptr )
  , _simPlayer( nullptr )
  , m_check{false}
{
  _labelGIDs = new QLabel( QString::number( _gidsize ) );
  _labelSpikes = new QLabel( QString::number( _spikesize ) );
  _labelStartTime = new QLabel( "0" );
  _labelEndTime = new QLabel( "0" );
  QGridLayout* gLayout = new QGridLayout( );
  gLayout->setAlignment( Qt::AlignTop );
  gLayout->addWidget( new QLabel( "Network Information:" ), 0, 0, 1, 1 );
  gLayout->addWidget( _labelGIDs, 0, 1, 1, 3 );
  gLayout->addWidget( new QLabel( "Simulation Spikes: " ), 1, 0, 1, 1 );
  gLayout->addWidget( _labelSpikes, 1, 1, 1, 3 );
  gLayout->addWidget( new QLabel( "Start Time: " ), 4, 0, 1, 1 );
  gLayout->addWidget( _labelStartTime, 4, 1, 1, 3 );
  gLayout->addWidget( new QLabel( "End Time: " ), 5, 0, 1, 1 );
  gLayout->addWidget( _labelEndTime, 5, 1, 1, 3 );
  setLayout( gLayout );

  m_timer.setInterval(DEFAULT_CKECK_INTERVAL);
  connect(&m_timer, SIGNAL(timeout()), this, SLOT(updateInfo()));
}

void DataInspector::addWidget( QWidget* widget, int row, int column,
                               int rowSpan, int columnSpan,
                               Qt::Alignment alignment )
{
  static_cast< QGridLayout* >( layout( ) )
    ->addWidget( widget, row, column, rowSpan, columnSpan, alignment );
}

void DataInspector::setSimPlayer( simil::SimulationPlayer* simPlayer_ )
{
  const auto timerActive = m_timer.isActive();
  if(timerActive) m_timer.stop();

  _simPlayer = simPlayer_;
  _gidsize = 0;
  _spikesize = 0;

  if(timerActive) m_timer.start();
}

void DataInspector::setCheckUpdates(const bool value)
{
  if(value != m_check)
  {
    m_check = value;

    if(m_check)
    {
      m_timer.start();
    }
    else
    {
      m_timer.stop();
    }
  }
}

void DataInspector::paintEvent( QPaintEvent* event )
{
  updateInfo( );
  QGroupBox::paintEvent( event );
}

void DataInspector::updateInfo( )
{
  if ( _simPlayer )
  {
    bool updated = false;
    if ( _simPlayer->gidsSize( ) != _gidsize )
    {
      updated = true;
      _gidsize = _simPlayer->gidsSize( );
      _labelGIDs->setText( QString::number( _gidsize ) );
    }

    auto spkPlay = dynamic_cast< simil::SpikesPlayer* >( _simPlayer );
    if ( spkPlay )
    {
      if ( spkPlay->spikesSize( ) != _spikesize )
      {
        updated = true;
        _spikesize = spkPlay->spikesSize( );
        _labelSpikes->setText( QString::number( _spikesize ) );
        _labelStartTime->setText( QString::number( spkPlay->startTime( ) ) );
        _labelEndTime->setText( QString::number( spkPlay->endTime( ) ) );
      }
    }

    if ( updated ) emit simDataChanged( );
  }
  else
  {
    _labelGIDs = new QLabel( "0" );
    _labelSpikes = new QLabel( "0" );
    _labelStartTime = new QLabel( "0" );
    _labelEndTime = new QLabel( "0" );
  }
}

void DataInspector::setCheckTimer(const int ms)
{
  if(m_timer.interval() == ms) return;

  if(m_timer.isActive())
  {
    m_timer.stop();
    m_timer.setInterval(ms);
    m_timer.start();
  }
  else
  {
    m_timer.setInterval(ms);
  }
}
