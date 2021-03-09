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

#ifndef TRANSFERFUNCTIONEDITOR_H
#define TRANSFERFUNCTIONEDITOR_H

#include <memory>

#include <QWidget>
#include <QAbstractButton>

#include "../types.h"

#include "ColorPoints.h"
#include "Gradient.h"

class TransferFunctionEditor : public QWidget
{
Q_OBJECT

public:
   TransferFunctionEditor( QWidget *parent_ = nullptr );

   ~TransferFunctionEditor( ) {};

   visimpl::TTransferFunction getColorPoints( );
   void setColorPoints( const visimpl::TTransferFunction& colors );

public slots:
    void colorPointsChanged(const QPolygonF &points);

private:
    QWidget *_parent;

    ColorPoints *_redPoints;
    ColorPoints *_bluePoints;
    ColorPoints *_greenPoints;
    ColorPoints *_alphaPoints;

    Gradient* gradientFrame;
    Gradient* noTransparencyGradientFrame;
    Gradient* redGradientFrame;
    Gradient* greenGradientFrame;
    Gradient* blueGradientFrame;
    Gradient* alphaGradientFrame;
};

#endif
