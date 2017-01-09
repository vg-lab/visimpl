/*
 * @file	SubsetEventWidget.cpp
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es> 
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *					Do not distribute without further notice.
 */

#include "SubsetEventWidget.h"

#include <QPainter>
#include <QBrush>

namespace visimpl
{

  SubsetEventWidget::SubsetEventWidget( void )
  : QFrame( nullptr )
//  , _subsetEventMngr( nullptr )
  , _timeFrames( nullptr )
  , _heightPerRow( 50 )
  , _columns( 20 )
  , _centralColumns( _columns - 2)
  , _margin( 20 )
  {

  }



  void SubsetEventWidget::paintEvent( QPaintEvent* /*event_*/ )
  {
    unsigned int totalHeight = height( );
    unsigned int totalWidth = width( );

    QPainter painter( this );

    painter.setPen( Qt::NoPen );
    painter.fillRect( rect( ), QBrush( QColor( 255, 255, 255, 255 ),
                                       Qt::SolidPattern ));

    unsigned int counter = 0;
    for( auto timeFrame : *_timeFrames )
    {
      if( counter > _index )
        break;

      QColor color = timeFrame.color;

      unsigned int up = /*( _heightPerRow * counter ) + */ _margin;
      unsigned int down = /* ( _heightPerRow * ( counter + 1 )) - */ totalHeight - _margin;

      unsigned int left;
      unsigned int right;

      if( counter == _index )
      {
//        unsigned int chunk = 0;
        for( auto timeFrameChunk : timeFrame.percentages )
        {
          QPainterPath path;
//          std::cout << "Painting " << counter
//                    << " chunk " << chunk
//                    << " from " << timeFrameChunk.first
//                    << " to " << timeFrameChunk.second
//                    << std::endl;

          left = timeFrameChunk.first * totalWidth;
          right = timeFrameChunk.second * totalWidth;

          path.moveTo( left, up );
          path.lineTo( left, down );
          path.lineTo( right, down );
          path.lineTo( right, up );
          path.closeSubpath( );

          color.setAlpha( 255 );
          painter.setBrush( QBrush( color, Qt::SolidPattern ));

          painter.drawPath( path );

          path.moveTo( left, down );
          path.lineTo( left, totalHeight );
          path.lineTo( right, totalHeight );
          path.lineTo( right, down );
          path.closeSubpath( );

          color.setAlpha( 50 );
          painter.setBrush( QBrush( color, Qt::SolidPattern ));

          painter.drawPath( path );

//          chunk++;
        }
      }
      else
      {
        color.setAlpha( 50 );
        painter.setBrush( QBrush( color, Qt::SolidPattern ));

        for( auto timeFrameChunk : timeFrame.percentages )
        {
            QPainterPath path;

          left = timeFrameChunk.first * totalWidth;
          right = timeFrameChunk.second * totalWidth;

          path.moveTo( left, 0 );
          path.lineTo( left, totalHeight );
          path.lineTo( right, totalHeight );
          path.lineTo( right, 0 );
          path.closeSubpath( );


          painter.drawPath( path );
        }
      }

      ++counter;
    }
  }

  void SubsetEventWidget::timeFrames( std::vector< TimeFrame >* timeFrameVector )
  {
    _timeFrames = timeFrameVector;
  }

  std::vector< TimeFrame >* SubsetEventWidget::timeFrames( void )
  {
    return _timeFrames;
  }

  void SubsetEventWidget::index( unsigned int index_ )
  {
    _index = index_;
  }

}

