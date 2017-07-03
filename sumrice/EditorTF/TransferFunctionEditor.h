/*
 * @file  TransferFunctionEditor.h
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
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
   TransferFunctionEditor( QWidget *parent_ = 0 );

   ~TransferFunctionEditor( void );

   visimpl::TTransferFunction getColorPoints( );
   void setColorPoints( const visimpl::TTransferFunction& colors );

protected:
    void applyColorMap();

/* Member slots */
public slots:
    void colorPointsChanged(const QPolygonF &points);

protected slots:
    void buttonClicked(QAbstractButton *button);

/* Member attributes */
private:
    QWidget *_parent;

    ColorPoints *_redPoints;
    ColorPoints *_bluePoints;
    ColorPoints *_greenPoints;
    ColorPoints *_alphaPoints;

//    osg::ref_ptr<osg::Texture1D> _texture;

    Gradient* gradientFrame;
    Gradient* noTransparencyGradientFrame;
    Gradient* redGradientFrame;
    Gradient* greenGradientFrame;
    Gradient* blueGradientFrame;
    Gradient* alphaGradientFrame;
};

#endif
