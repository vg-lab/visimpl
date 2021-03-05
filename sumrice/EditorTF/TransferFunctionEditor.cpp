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

// Sumrice
#include "TransferFunctionEditor.h"
#include "ColorPoints.h"

// C++
#include <iostream>
#include <limits>
#include <fstream>

// Qt
#include <QVBoxLayout>
#include <QHBoxLayout>

TransferFunctionEditor::TransferFunctionEditor( QWidget* parent_ )
: _parent( parent_ )
{
  gradientFrame = new Gradient( );
  noTransparencyGradientFrame = new Gradient( );

  redGradientFrame = new Gradient( );
  greenGradientFrame = new Gradient( );
  blueGradientFrame = new Gradient( );
  alphaGradientFrame = new Gradient( );

  QVBoxLayout* verticalLayout = new QVBoxLayout( );
  QHBoxLayout* horizontalLayout = new QHBoxLayout( );

  horizontalLayout->addWidget( redGradientFrame );
  horizontalLayout->addWidget( greenGradientFrame );
  horizontalLayout->addWidget( blueGradientFrame );
  horizontalLayout->addWidget( alphaGradientFrame );

  QWidget* vw = new QWidget( );

  verticalLayout->addWidget( vw );
  vw->setLayout( horizontalLayout );
  verticalLayout->addWidget( noTransparencyGradientFrame );
  verticalLayout->addWidget( gradientFrame );

  this->setLayout( verticalLayout );

  QGradientStops stops;
  stops << qMakePair(0.0, QColor(0, 0, 0, 0))
        << qMakePair(1.0, QColor(255, 255, 255, 255));
  gradientFrame->setDirection(Gradient::HORIZONTAL);
  gradientFrame->setGradientStops(stops);

  QGradientStops ntStops;
  stops << qMakePair(0.0, QColor(0, 0, 0, 255))
        << qMakePair(1.0, QColor(255, 255, 255, 255));
  noTransparencyGradientFrame->setDirection(Gradient::HORIZONTAL);
  noTransparencyGradientFrame->setGradientStops(stops);

  QPolygonF points;
  points << QPointF(0, 0) << QPointF(1, 1);

  _redPoints = new ColorPoints(redGradientFrame);
  _redPoints->setPoints(points);
  redGradientFrame->redGradient();
    
  _greenPoints = new ColorPoints(greenGradientFrame);
  _greenPoints->setPoints(points);
  greenGradientFrame->greenGradient();

  _bluePoints = new ColorPoints(blueGradientFrame);
  _bluePoints->setPoints(points);
  blueGradientFrame->blueGradient();

  _alphaPoints = new ColorPoints(alphaGradientFrame);
  _alphaPoints->setPoints(points);
  alphaGradientFrame->alphaGradient();

  connect(_redPoints, SIGNAL(pointInserted(int, float)), _greenPoints, SLOT(insertPoint(int, float)));
  connect(_redPoints, SIGNAL(pointInserted(int, float)), _bluePoints, SLOT(insertPoint(int, float)));
  connect(_redPoints, SIGNAL(pointInserted(int, float)), _alphaPoints, SLOT(insertPoint(int, float)));

  connect(_greenPoints, SIGNAL(pointInserted(int, float)), _redPoints, SLOT(insertPoint(int, float)));
  connect(_greenPoints, SIGNAL(pointInserted(int, float)), _bluePoints, SLOT(insertPoint(int, float)));
  connect(_greenPoints, SIGNAL(pointInserted(int, float)), _alphaPoints, SLOT(insertPoint(int, float)));

  connect(_bluePoints, SIGNAL(pointInserted(int, float)), _redPoints, SLOT(insertPoint(int, float)));
  connect(_bluePoints, SIGNAL(pointInserted(int, float)), _greenPoints, SLOT(insertPoint(int, float)));
  connect(_bluePoints, SIGNAL(pointInserted(int, float)), _alphaPoints, SLOT(insertPoint(int, float)));

  connect(_alphaPoints, SIGNAL(pointInserted(int, float)), _redPoints, SLOT(insertPoint(int, float)));
  connect(_alphaPoints, SIGNAL(pointInserted(int, float)), _greenPoints, SLOT(insertPoint(int, float)));
  connect(_alphaPoints, SIGNAL(pointInserted(int, float)), _bluePoints, SLOT(insertPoint(int, float)));

  connect(_redPoints, SIGNAL(pointRemoved(int)), _greenPoints, SLOT(removePoint(int)));
  connect(_redPoints, SIGNAL(pointRemoved(int)), _bluePoints, SLOT(removePoint(int)));
  connect(_redPoints, SIGNAL(pointRemoved(int)), _alphaPoints, SLOT(removePoint(int)));

  connect(_greenPoints, SIGNAL(pointRemoved(int)), _redPoints, SLOT(removePoint(int)));
  connect(_greenPoints, SIGNAL(pointRemoved(int)), _bluePoints, SLOT(removePoint(int)));
  connect(_greenPoints, SIGNAL(pointRemoved(int)), _alphaPoints, SLOT(removePoint(int)));

  connect(_bluePoints, SIGNAL(pointRemoved(int)), _redPoints, SLOT(removePoint(int)));
  connect(_bluePoints, SIGNAL(pointRemoved(int)), _greenPoints, SLOT(removePoint(int)));
  connect(_bluePoints, SIGNAL(pointRemoved(int)), _alphaPoints, SLOT(removePoint(int)));

  connect(_alphaPoints, SIGNAL(pointRemoved(int)), _redPoints, SLOT(removePoint(int)));
  connect(_alphaPoints, SIGNAL(pointRemoved(int)), _greenPoints, SLOT(removePoint(int)));
  connect(_alphaPoints, SIGNAL(pointRemoved(int)), _bluePoints, SLOT(removePoint(int)));

  connect(_redPoints, SIGNAL(pointAbscissaChanged(int, float)), _greenPoints, SLOT(movePointAbscissa(int, float)));
  connect(_redPoints, SIGNAL(pointAbscissaChanged(int, float)), _bluePoints, SLOT(movePointAbscissa(int, float)));
  connect(_redPoints, SIGNAL(pointAbscissaChanged(int, float)), _alphaPoints, SLOT(movePointAbscissa(int, float)));

  connect(_greenPoints, SIGNAL(pointAbscissaChanged(int, float)), _redPoints, SLOT(movePointAbscissa(int, float)));
  connect(_greenPoints, SIGNAL(pointAbscissaChanged(int, float)), _bluePoints, SLOT(movePointAbscissa(int, float)));
  connect(_greenPoints, SIGNAL(pointAbscissaChanged(int, float)), _alphaPoints, SLOT(movePointAbscissa(int, float)));

  connect(_bluePoints, SIGNAL(pointAbscissaChanged(int, float)), _redPoints, SLOT(movePointAbscissa(int, float)));
  connect(_bluePoints, SIGNAL(pointAbscissaChanged(int, float)), _greenPoints, SLOT(movePointAbscissa(int, float)));
  connect(_bluePoints, SIGNAL(pointAbscissaChanged(int, float)), _alphaPoints, SLOT(movePointAbscissa(int, float)));

  connect(_alphaPoints, SIGNAL(pointAbscissaChanged(int, float)), _redPoints, SLOT(movePointAbscissa(int, float)));
  connect(_alphaPoints, SIGNAL(pointAbscissaChanged(int, float)), _greenPoints, SLOT(movePointAbscissa(int, float)));
  connect(_alphaPoints, SIGNAL(pointAbscissaChanged(int, float)), _bluePoints, SLOT(movePointAbscissa(int, float)));

  connect(_redPoints, SIGNAL(pointsChanged(const QPolygonF &)), this, SLOT(colorPointsChanged(const QPolygonF &)));
  connect(_greenPoints, SIGNAL(pointsChanged(const QPolygonF &)), this, SLOT(colorPointsChanged(const QPolygonF &)));
  connect(_bluePoints, SIGNAL(pointsChanged(const QPolygonF &)), this, SLOT(colorPointsChanged(const QPolygonF &)));
  connect(_alphaPoints, SIGNAL(pointsChanged(const QPolygonF &)), this, SLOT(colorPointsChanged(const QPolygonF &)));
}

visimpl::TTransferFunction TransferFunctionEditor::getColorPoints( void )
{
  visimpl::TTransferFunction result;

  QGradientStops stops = gradientFrame->getGradientStops( );

  for( auto stop : stops )
  {
    result.push_back( std::make_pair( float(stop.first), stop.second ));
  }

  return result;
}

void TransferFunctionEditor::colorPointsChanged
(const QPolygonF &points)
{
    QObject *pointSet = sender();
    int indexToUpdate = 0;
    if (pointSet == _redPoints)
        indexToUpdate = 0;
    else if (pointSet == _greenPoints)
        indexToUpdate = 1;
    else if (pointSet == _bluePoints)
        indexToUpdate = 2;
    else if (pointSet == _alphaPoints)
        indexToUpdate = 3;

    QGradientStops stops = gradientFrame->getGradientStops();

    stops.resize(points.size());

    QGradientStops ntStops = stops;
    for (int i = 0; i < points.size(); i++)
    {
        stops[i].first = points[i].x();
        switch (indexToUpdate) {
        case 0: stops[i].second.setRedF(points[i].y()); break;
        case 1: stops[i].second.setGreenF(points[i].y()); break;
        case 2: stops[i].second.setBlueF(points[i].y()); break;
        case 3: stops[i].second.setAlphaF(points[i].y());break;
        }
        ntStops[i].second.setAlphaF(1.0);
    }
    gradientFrame->setGradientStops(stops);
    noTransparencyGradientFrame->setGradientStops( ntStops );
}

void TransferFunctionEditor::setColorPoints( const visimpl::TTransferFunction& colors )
{
  QPolygonF redPoints;
  QPolygonF greenPoints;
  QPolygonF bluePoints;
  QPolygonF alphaPoints;

  for( auto c : colors )
  {
    redPoints.append( QPointF( c.first, c.second.redF( )));
    greenPoints.append( QPointF( c.first, c.second.greenF( )));
    bluePoints.append( QPointF( c.first, c.second.blueF( )));
    alphaPoints.append( QPointF( c.first, c.second.alphaF( )));
  }

  _redPoints->setPoints( redPoints, true );
  _greenPoints->setPoints( greenPoints, true );
  _bluePoints->setPoints( bluePoints, true );
  _alphaPoints->setPoints( alphaPoints, true );
}

