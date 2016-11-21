/*
 * @file  Gradient.h
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */

#ifndef GRADIENT_H
#define GRADIENT_H

#include <QFrame>

typedef std::pair< float, QColor > TTFColor;
typedef std::vector< TTFColor > TTransferFunction;

typedef std::pair< float, float > TSize;
typedef std::vector< TSize > TSizeFunction;

class Gradient : public QFrame
{
/* Declarations */
public:
    enum Direction {HORIZONTAL, VERTICAL};

/* Constructor */
public:
    Gradient(QWidget *parent = 0);
    virtual ~Gradient( ) {}
/* Member functions */
public:
    void setDirection(Direction direction)
    {
        _direction = direction;
        update();
    }

    void setGradientStops(const QGradientStops &stops)
    {
        _stops = stops;
        update();
    }

    const QGradientStops &getGradientStops() const
    {
        return _stops;
    }

    void redGradient();
    void blueGradient();
    void greenGradient();
    void alphaGradient();

    virtual void paintEvent(QPaintEvent *event);

    void plot( const QPolygonF& plot );
    QPolygonF plot( void );
    void clearPlot( void );

/* Member attributes */
protected:
    Direction _direction;
    QGradientStops _stops;

    QPolygonF _plot;

private:

    float xPos( float x_ );
    float yPos( float y_ );
};

#endif
