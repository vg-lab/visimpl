/*
 * @file  FocusFrame.cpp
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es> 
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */

#include "FocusFrame.h"

#include <QPainter>

FocusFrame::FocusFrame( QWidget* parent_ )
: QFrame( parent_ )
, _visibleStart( 0.0f )
, _width( 0.0f )
, _offset( 0.0f )
, _currentPosition( 0 )
, _firstPointLocal( 0 )
, _lastPointLocal( 0 )
, _firstPointGlobal( 0 )
, _lastPointGlobal( 0 )
, _colorLocal( "#e31a1c" )
, _colorGlobal( "#1f78b4" )
, _fillPlots( true )
{ }


void FocusFrame::viewRegion( const visimpl::MultiLevelHistogram& histogram,
                             float marker,// float offset,
                             float regionWidth )
{
  unsigned int maxSize = histogram.focusHistogramSize( );
  
  unsigned int position = maxSize * marker;
  unsigned int regionW =  maxSize * regionWidth;

  int start = position - regionW - 1;
  unsigned int end = position + regionW;
  if( start < 0 )
  {
    start = 0;
    end = 2 * regionW;
  }
  else if( end > maxSize )
  {
    end = maxSize;
    start = end - ( 2 * regionW );
  }

  _currentPosition = position;

  _curveLocal = histogram.focusLocalFunction( );
  _curveGlobal = histogram.focusGlobalFunction( );

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

//  int offset = _offset * width( );
  float delta = width( ) / float( _lastPointLocal - _firstPointLocal - 1);

  unsigned int counter = 0;
  pathGlobal.moveTo( 0, height( ));
  for( current  = _curveGlobal.begin( ) + _firstPointGlobal;
       current != _curveGlobal.begin( ) + _lastPointGlobal;
       current++ )
  {
    int coordX = counter * delta;
//        ( current - ( _curveGlobal.begin( ) + _firstPointGlobal) )
    //* delta; //(*current).x( ) * width( );
    int coordY = (*current).y( ) * height( );

    pathGlobal.lineTo( coordX, coordY );
    counter++;
  }
  pathGlobal.lineTo( width( ), height( ));

  counter = 0;
  pathLocal.moveTo( 0, height( ));
  for( current  = _curveLocal.begin( ) + _firstPointLocal;
       current != _curveLocal.begin( ) + _lastPointLocal;
       current++ )
  {
    int coordX = counter * delta;
//        ( current - ( _curveLocal.begin( ) + _firstPointLocal) )
           // * delta; //(*current).x( ) * width( );
    int coordY = (*current).y( ) * height( );
    pathLocal.lineTo( coordX, coordY );
    counter++;
  }
  pathLocal.lineTo( width( ), height( ));

  QColor globalColor( _colorGlobal );
  QColor localColor( _colorLocal );

  if( _fillPlots )
  {
    globalColor.setAlpha( 50 );
    localColor.setAlpha( 50 );

    painter.setPen( Qt::NoPen );
    painter.setBrush( QBrush( globalColor, Qt::SolidPattern ));
    painter.drawPath( pathGlobal );

    painter.setBrush( QBrush( _colorLocal, Qt::SolidPattern ));
    painter.drawPath( pathLocal );

  }
  else
  {
    globalColor.setAlpha( 100 );
    localColor.setAlpha( 100 );

    painter.setBrush( Qt::NoBrush );
    painter.setPen( QPen( globalColor, Qt::SolidPattern ));
    painter.drawPath( pathGlobal );

    painter.setPen( QPen( localColor, Qt::SolidPattern ));
    painter.drawPath( pathLocal );

  }

  float markerPos = ( _currentPosition - _firstPointLocal ) /
                  float( _lastPointLocal - _firstPointLocal - 1 );

  int positionX = markerPos * width( );

  QLine marker( QPoint( positionX, 0 ), QPoint( positionX, height( )));
  painter.setPen( QColor( 177, 50, 50 ));
  painter.drawLine( marker );

}

QColor FocusFrame::colorLocal( void )
{
  return _colorLocal;
}

void FocusFrame::colorLocal( const QColor& color )
{
  _colorLocal = color;
}

QColor FocusFrame::colorGlobal( void )
{
  return _colorGlobal;
}

void FocusFrame::colorGlobal( const QColor& color )
{
  _colorGlobal = color;
}

void FocusFrame::fillPlots( bool fillPlots_ )
{
  _fillPlots = fillPlots_;
}
