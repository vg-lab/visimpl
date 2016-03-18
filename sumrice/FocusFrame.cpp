/*
 * @file	ZoomFrame.cpp
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es> 
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *					Do not distribute without further notice.
 */

#include "FocusFrame.h"

#include <QPainter>

FocusFrame::FocusFrame( QWidget* parent_ )
: QFrame( parent_ )
, _visibleStart( 0.0f )
, _width( 0.0f )
, _firstPointLocal( 0 )
, _lastPointLocal( 0 )
, _firstPointGlobal( 0 )
, _lastPointGlobal( 0 )
{

}


void FocusFrame::viewRegion( const visimpl::Histogram& histogram,
                             float marker,
                             float regionWidth )
{
  unsigned int maxSize = histogram.histogram( ).size( ) - 1;
  
  unsigned int position = maxSize * marker;
  unsigned int regionW =  maxSize * regionWidth;

  int start = position - regionW;
  unsigned int end = position + regionW;
  if( start < 0 )
  {
    start = 0;
    end = regionW;
  }
  else if( end > maxSize )
  {
    end = maxSize;
    start = end - ( 2 * regionW );
  }


  start = std::max( 0, start );


  // Get the previous one
  if( position > 0)
    position--;

  if( end > maxSize )
    end = maxSize;

  _curveLocal = histogram.localFunction( );
  _curveGlobal = histogram.globalFunction( );

  _firstPointLocal = start;
  _firstPointGlobal = start;

  _lastPointLocal = end;
  _lastPointGlobal = end;

}


void FocusFrame::paintEvent( QPaintEvent* /*event_*/ )
{

  if( _lastPointGlobal == _firstPointGlobal )
    return;

  QPainter painter( this );

  painter.setRenderHint( QPainter::Antialiasing );

  painter.fillRect( rect( ), QBrush( QColor( 255, 255, 255, 255 ),
                                     Qt::SolidPattern ));
  QPainterPath pathLocal;
  QPainterPath pathGlobal;

  QPolygonF::iterator current;

  unsigned int delta = width( ) / ( _lastPointLocal - _firstPointLocal );

  pathGlobal.moveTo( 0, height( ));
  for( current  = _curveGlobal.begin( ) + _firstPointGlobal;
       current != _curveGlobal.begin( ) + _lastPointGlobal;
       current++ )
  {
    int coordX = ( current - ( _curveGlobal.begin( ) + _firstPointGlobal) )
        * delta; //(*current).x( ) * width( );
    int coordY = (*current).y( ) * height( );

    pathGlobal.lineTo( coordX, coordY );
  }
  pathGlobal.lineTo( width( ), height( ));

  pathLocal.moveTo( 0, height( ));
  for( current  = _curveLocal.begin( ) + _firstPointLocal;
       current != _curveLocal.begin( ) + _lastPointLocal;
       current++ )
  {
    int coordX = ( current - ( _curveLocal.begin( ) + _firstPointLocal) )
            * delta; //(*current).x( ) * width( );
    int coordY = (*current).y( ) * height( );
    pathLocal.lineTo( coordX, coordY );
  }
  pathLocal.lineTo( width( ), height( ));

  painter.setBrush( QBrush( QColor( 255, 0, 0, 100 ), Qt::SolidPattern));
  painter.drawPath( pathGlobal );

  painter.setBrush( QBrush( QColor( 0, 0, 128, 50 ), Qt::SolidPattern));
  painter.drawPath( pathLocal );

}
