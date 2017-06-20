/*
 * @file	SubsetEventWidget.cpp
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es> 
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *					Do not distribute without further notice.
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

