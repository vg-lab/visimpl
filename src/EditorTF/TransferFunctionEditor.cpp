/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2006. All rights reserved.

        Authors: Juan Bautista Hernando Vieites

*/

#include "TransferFunctionEditor.h"
#include "ColorPoints.h"

#include <iostream>
#include <limits>
#include <fstream>

#include <QtGui>
#include <QVBoxLayout>
#include <QHBoxLayout>

//typedef std::map<float, osg::Vec4> Colors;

/*
  TransferFunctionEditor::Impl
*/

TransferFunctionEditor::TransferFunctionEditor( QWidget* parent_ ) :
    _parent( parent_ )
{
//    setupUi(parent);

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

    /* Setting gradient frames */
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

    /* Connecting slots */
//    connect(buttonBox, SIGNAL(clicked(QAbstractButton*)),
//            this, SLOT(buttonClicked(QAbstractButton*)));

    connect(_redPoints, SIGNAL(pointInserted(int, float)), 
            _greenPoints, SLOT(insertPoint(int, float)));
    connect(_redPoints, SIGNAL(pointInserted(int, float)), 
            _bluePoints, SLOT(insertPoint(int, float)));
    connect(_redPoints, SIGNAL(pointInserted(int, float)), 
            _alphaPoints, SLOT(insertPoint(int, float)));

    connect(_greenPoints, SIGNAL(pointInserted(int, float)), 
            _redPoints, SLOT(insertPoint(int, float)));
    connect(_greenPoints, SIGNAL(pointInserted(int, float)), 
            _bluePoints, SLOT(insertPoint(int, float)));
    connect(_greenPoints, SIGNAL(pointInserted(int, float)), 
            _alphaPoints, SLOT(insertPoint(int, float)));

    connect(_bluePoints, SIGNAL(pointInserted(int, float)), 
            _redPoints, SLOT(insertPoint(int, float)));
    connect(_bluePoints, SIGNAL(pointInserted(int, float)), 
            _greenPoints, SLOT(insertPoint(int, float)));
    connect(_bluePoints, SIGNAL(pointInserted(int, float)), 
            _alphaPoints, SLOT(insertPoint(int, float)));

    connect(_alphaPoints, SIGNAL(pointInserted(int, float)), 
            _redPoints, SLOT(insertPoint(int, float)));
    connect(_alphaPoints, SIGNAL(pointInserted(int, float)), 
            _greenPoints, SLOT(insertPoint(int, float)));
    connect(_alphaPoints, SIGNAL(pointInserted(int, float)), 
            _bluePoints, SLOT(insertPoint(int, float)));

    connect(_redPoints, SIGNAL(pointRemoved(int)), 
            _greenPoints, SLOT(removePoint(int)));
    connect(_redPoints, SIGNAL(pointRemoved(int)), 
            _bluePoints, SLOT(removePoint(int)));
    connect(_redPoints, SIGNAL(pointRemoved(int)), 
            _alphaPoints, SLOT(removePoint(int)));

    connect(_greenPoints, SIGNAL(pointRemoved(int)), 
            _redPoints, SLOT(removePoint(int)));
    connect(_greenPoints, SIGNAL(pointRemoved(int)), 
            _bluePoints, SLOT(removePoint(int)));
    connect(_greenPoints, SIGNAL(pointRemoved(int)), 
            _alphaPoints, SLOT(removePoint(int)));

    connect(_bluePoints, SIGNAL(pointRemoved(int)), 
            _redPoints, SLOT(removePoint(int)));
    connect(_bluePoints, SIGNAL(pointRemoved(int)), 
            _greenPoints, SLOT(removePoint(int)));
    connect(_bluePoints, SIGNAL(pointRemoved(int)), 
            _alphaPoints, SLOT(removePoint(int)));

    connect(_alphaPoints, SIGNAL(pointRemoved(int)), 
            _redPoints, SLOT(removePoint(int)));
    connect(_alphaPoints, SIGNAL(pointRemoved(int)), 
            _greenPoints, SLOT(removePoint(int)));
    connect(_alphaPoints, SIGNAL(pointRemoved(int)), 
            _bluePoints, SLOT(removePoint(int)));

    connect(_redPoints, SIGNAL(pointAbscissaChanged(int, float)), 
            _greenPoints, SLOT(movePointAbscissa(int, float)));
    connect(_redPoints, SIGNAL(pointAbscissaChanged(int, float)), 
            _bluePoints, SLOT(movePointAbscissa(int, float)));
    connect(_redPoints, SIGNAL(pointAbscissaChanged(int, float)), 
            _alphaPoints, SLOT(movePointAbscissa(int, float)));

    connect(_greenPoints, SIGNAL(pointAbscissaChanged(int, float)), 
            _redPoints, SLOT(movePointAbscissa(int, float)));
    connect(_greenPoints, SIGNAL(pointAbscissaChanged(int, float)), 
            _bluePoints, SLOT(movePointAbscissa(int, float)));
    connect(_greenPoints, SIGNAL(pointAbscissaChanged(int, float)), 
            _alphaPoints, SLOT(movePointAbscissa(int, float)));

    connect(_bluePoints, SIGNAL(pointAbscissaChanged(int, float)), 
            _redPoints, SLOT(movePointAbscissa(int, float)));
    connect(_bluePoints, SIGNAL(pointAbscissaChanged(int, float)), 
            _greenPoints, SLOT(movePointAbscissa(int, float)));
    connect(_bluePoints, SIGNAL(pointAbscissaChanged(int, float)), 
            _alphaPoints, SLOT(movePointAbscissa(int, float)));

    connect(_alphaPoints, SIGNAL(pointAbscissaChanged(int, float)), 
            _redPoints, SLOT(movePointAbscissa(int, float)));
    connect(_alphaPoints, SIGNAL(pointAbscissaChanged(int, float)), 
            _greenPoints, SLOT(movePointAbscissa(int, float)));
    connect(_alphaPoints, SIGNAL(pointAbscissaChanged(int, float)), 
            _bluePoints, SLOT(movePointAbscissa(int, float)));

    connect(_redPoints, SIGNAL(pointsChanged(const QPolygonF &)),
            this, SLOT(colorPointsChanged(const QPolygonF &)));
    connect(_greenPoints, SIGNAL(pointsChanged(const QPolygonF &)),
            this, SLOT(colorPointsChanged(const QPolygonF &)));
    connect(_bluePoints, SIGNAL(pointsChanged(const QPolygonF &)),
            this, SLOT(colorPointsChanged(const QPolygonF &)));
    connect(_alphaPoints, SIGNAL(pointsChanged(const QPolygonF &)),
            this, SLOT(colorPointsChanged(const QPolygonF &)));

}

TransferFunctionEditor::~TransferFunctionEditor()
{
}

TTransferFunction TransferFunctionEditor::getColorPoints( void )
{
  TTransferFunction result;

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

void TransferFunctionEditor::setColorPoints( const TTransferFunction& colors )
{
//  QGradientStops gradientColors;
//  QGradientStops ntGradientColors;
  QPolygonF redPoints;
  QPolygonF greenPoints;
  QPolygonF bluePoints;
  QPolygonF alphaPoints;

  for( auto c : colors )
  {
//    QGradientStop stop ( qreal( c.first ), c.second );
//    gradientColors.push_back( stop );
//
//    QGradientStop aux( stop );
//    aux.second.setAlphaF( 1.0 );
//    ntGradientColors.push_back( aux );

    redPoints.append( QPointF( c.first, c.second.redF( )));
    greenPoints.append( QPointF( c.first, c.second.greenF( )));
    bluePoints.append( QPointF( c.first, c.second.blueF( )));
    alphaPoints.append( QPointF( c.first, c.second.alphaF( )));

  }

//  gradientFrame->setGradientStops( gradientColors );
//
//  noTransparencyGradientFrame->setGradientStops( ntGradientColors );

  _redPoints->setPoints( redPoints, true );
  _greenPoints->setPoints( greenPoints, true );
  _bluePoints->setPoints( bluePoints, true );
  _alphaPoints->setPoints( alphaPoints, true );
}

void TransferFunctionEditor::buttonClicked(QAbstractButton* /*button*/)
{
//    switch (buttonBox->standardButton(button))
//    {
//    case QDialogButtonBox::Apply: applyColorMap(); break;
//    default:;
//    }
}

void TransferFunctionEditor::applyColorMap()
{
//    if (_texture.get() == 0)
//        return;
//
//    const QGradientStops &stops = gradientFrame->getGradientStops();
//
//    size_t max = _texture->getImage()->s();
//    QGradientStops::const_iterator next = stops.begin();
//    QGradientStops::const_iterator previous = next++;
//    for (; next != stops.end(); previous = next++)
//    {
//        unsigned int start = (unsigned int)(max * previous->first);
//        unsigned int end = (unsigned int)(max * next->first);
//        for (size_t i = start; i < end; ++i)
//        {
//            float a = (i - start) / double(end - start);
//            unsigned char *color = _texture->getImage()->data(i);
//            color[0] = round(previous->second.red() * (1 - a) +
//                             next->second.red() * a);
//            color[1] = round(previous->second.green() * (1 - a) +
//                             next->second.green() * a);
//            color[2] = round(previous->second.blue() * (1 - a) +
//                             next->second.blue() * a);
//            color[3] = round(previous->second.alpha() * (1 - a) +
//                             next->second.alpha() * a);
//        }
//    }
//    _texture->getImage()->dirty();
}


/*
  Constructor 
*/


/* 
  Destructor
*/


