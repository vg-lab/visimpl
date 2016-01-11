#ifndef TRANSFERFUNCTIONEDITOR_H
#define TRANSFERFUNCTIONEDITOR_H

#include <memory>

#include <QWidget>
#include <QAbstractButton>

#include "ColorPoints.h"
#include "Gradient.h"

typedef std::pair< float, QColor > TTFColor;
typedef std::vector< TTFColor > TTransferFunction;

class TransferFunctionEditor : public QWidget
{
Q_OBJECT

public:
   TransferFunctionEditor( QWidget *parent_ = 0 );

   ~TransferFunctionEditor( void );

   TTransferFunction getColorPoints( );

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

    QString _lastFilePath;

//    osg::ref_ptr<osg::Texture1D> _texture;

    Gradient* gradientFrame;
    Gradient* noTransparencyGradientFrame;
    Gradient* redGradientFrame;
    Gradient* greenGradientFrame;
    Gradient* blueGradientFrame;
    Gradient* alphaGradientFrame;
};

#endif
