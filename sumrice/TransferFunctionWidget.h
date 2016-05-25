/*
 * ColorMapperWidget.h
 *
 *  Created on: 2 de mar. de 2016
 *      Author: sgalindo
 */
#ifndef __TRANSFERFUNCTIONWIDGET_H__
#define __TRANSFERFUNCTIONWIDGET_H__

#include <prefr/prefr.h>

#include <QWidget>
#include <QPushButton>
#include <QMouseEvent>

#include "EditorTF/ColorPoints.h"
#include "EditorTF/Gradient.h"

class TransferFunctionWidget : public QWidget
{
  Q_OBJECT;

public:
  TransferFunctionWidget( QWidget* parent = 0 );
  virtual ~TransferFunctionWidget( );

  TTransferFunction getColors( bool includeAlpha = true );
  void setColorPoints( const TTransferFunction& colors,
                       bool updateResult = true );

  TTransferFunction getPreviewColors( void );

protected slots:

  void gradientClicked( void );

  void buttonClicked( QAbstractButton* button );
  void acceptClicked( void );
  void cancelClicked( void );
  void previewClicked( void );

  void colorPointsChanged( const QPolygonF &points );

signals:

  void colorChanged( void );

  void previewColor( void );

protected:

  virtual void mousePressEvent( QMouseEvent * event );

  void InitDialog( void );

  ColorPoints *_redPoints;
  ColorPoints *_bluePoints;
  ColorPoints *_greenPoints;
  ColorPoints *_alphaPoints;

//    osg::ref_ptr<osg::Texture1D> _texture;

  Gradient* gradientFrame;
  Gradient* nTGradientFrame;
  Gradient* redGradientFrame;
  Gradient* greenGradientFrame;
  Gradient* blueGradientFrame;
  Gradient* alphaGradientFrame;

  QWidget* _dialog;
  QPushButton* _saveButton;
  QPushButton* _discardButton;
  QPushButton* _previewButton;
  bool previewed;

  Gradient* _result;
  QGradientStops _tResult;
};



#endif /* __TRANSFERFUNCTIONWIDGET_H__ */
