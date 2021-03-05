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

#ifndef GRADIENT_H
#define GRADIENT_H

#include <QFrame>

#include "../types.h"

class Gradient : public QFrame
{

public:
    enum Direction {HORIZONTAL, VERTICAL};

public:
    Gradient(QWidget *parent = 0);
    virtual ~Gradient( ) {}

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

protected:
    Direction _direction;
    QGradientStops _stops;

    QPolygonF _plot;

private:
    float xPos( float x_ );
    float yPos( float y_ );
};

#endif
