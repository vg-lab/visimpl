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

#include "EventWidget.h"

#include <QPainter>
#include <QBrush>

namespace visimpl
{

  EventWidget::EventWidget( void )
  : QFrame( nullptr )
  , _events( nullptr )
  , _index( 0 )
  , _heightPerRow( 50 )
  , _columns( 20 )
  , _centralColumns( _columns - 2)
  , _margin( 20 )
  {

  }

  void EventWidget::resizeEvent( QResizeEvent* /*event*/ )
  {
    unsigned int w = width( );
    unsigned int h = height( );

    unsigned int up = _margin;
    unsigned int down = h - _margin;

    unsigned int counter = 0;
    for( auto& e : *_events )
    {
      if( counter != _index )
      {
        counter++;
        continue;
      }
      unsigned int left;
      unsigned int right;

      e._cachedCustomSolidRep.clear( );
      e._cachedCustomTransRep.clear( );
      e._cachedCommonRep.clear( );

      e._cachedCustomSolidRep.reserve( e.percentages.size( ));
      e._cachedCustomTransRep.reserve( e.percentages.size( ));
      e._cachedCommonRep.reserve( e.percentages.size( ));

      for( const auto& timeFrameChunk : e.percentages )
      {
        left = timeFrameChunk.first * w;
        right = timeFrameChunk.second * w;

        // Create cached custom representation for events.
        QPainterPath customSolid;
        customSolid.moveTo( left, up );
        customSolid.lineTo( left, down );
        customSolid.lineTo( right, down );
        customSolid.lineTo( right, up );
        customSolid.closeSubpath( );

        e._cachedCustomSolidRep.push_back( customSolid );

        QPainterPath customTransparent;
        customTransparent.moveTo( left, down );
        customTransparent.lineTo( left, h );
        customTransparent.lineTo( right, h );
        customTransparent.lineTo( right, down );
        customTransparent.closeSubpath( );

        e._cachedCustomTransRep.push_back( customTransparent );

        // Create cached common representation for histograms.
        QPainterPath common;
        common.moveTo( left, 0 );
        common.lineTo( left, _heightPerRow);
        common.lineTo( right, _heightPerRow );
        common.lineTo( right, 0 );
        common.closeSubpath( );

        e._cachedCommonRep.push_back( common );
      }

      break;
    }
  }

  void EventWidget::updateCommonRepSizeVert( unsigned int newHeight )
  {
    _heightPerRow = newHeight;

    unsigned int w = width( );

    unsigned int left;
    unsigned int right;
    for( auto& e : *_events )
    {
      e._cachedCommonRep.clear( );
      for( const auto& timeFrameChunk : e.percentages )
      {
        left = timeFrameChunk.first * w;
        right = timeFrameChunk.second * w;

        // Create cached common representation for histograms.
        QPainterPath common;
        common.moveTo( left, 0 );
        common.lineTo( left, _heightPerRow);
        common.lineTo( right, _heightPerRow );
        common.lineTo( right, 0 );
        common.closeSubpath( );

        e._cachedCommonRep.push_back( common );
      }
    }
  }

  void EventWidget::paintEvent( QPaintEvent* /*event_*/ )
  {
    QPainter painter( this );

    painter.setPen( Qt::NoPen );
    painter.fillRect( rect( ), QBrush( QColor( 255, 255, 255, 255 ),
                                       Qt::SolidPattern ));

    unsigned int counter = 0;
    for( auto& e : *_events )
    {
      if( counter > _index )
        break;

      if( !e.visible )
        continue;

      QColor color = e.color;

      if( counter == _index )
      {
        color.setAlpha( 255 );
        painter.setBrush( QBrush( color, Qt::SolidPattern ));

        for( const auto& p : e._cachedCustomSolidRep )
          painter.drawPath( p );

        color.setAlpha( 50 );
        painter.setBrush( QBrush( color, Qt::SolidPattern ));

        for( const auto& p : e._cachedCustomTransRep )
          painter.drawPath( p );
      }
      else
      {
        color.setAlpha( 50 );
        painter.setBrush( QBrush( color, Qt::SolidPattern ));

        for( const auto& p : e._cachedCommonRep )
          painter.drawPath( p );

      }

      ++counter;
    }
  }

  void EventWidget::timeFrames( std::vector< TEvent >* timeFrameVector )
  {
    _events = timeFrameVector;
  }

  std::vector< TEvent >* EventWidget::timeFrames( void )
  {
    return _events;
  }

  void EventWidget::index( unsigned int index_ )
  {
    _index = index_;
  }

  void EventWidget::name( const std::string& name_ )
  {
    _name = name_;
  }

  const std::string& EventWidget::name( void ) const
  {
    return _name;
  }

  
  
}

