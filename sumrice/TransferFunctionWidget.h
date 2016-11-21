/*
 * @file  TransferFunctionWidget.h
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */

#ifndef __TRANSFERFUNCTIONWIDGET_H__
#define __TRANSFERFUNCTIONWIDGET_H__

#include <prefr/prefr.h>

#include <QWidget>
#include <QPushButton>
#include <QMouseEvent>
#include <QDoubleSpinBox>
#include <QLabel>

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

  TSizeFunction getSizeFunction( void );
  void setSizeFunction( const TSizeFunction& sizeFunc );


  TTransferFunction getPreviewColors( void );
  TSizeFunction getSizePreview( void );




protected slots:

  void gradientClicked( void );

//  void buttonClicked( QAbstractButton* button );
  void acceptClicked( void );
  void cancelClicked( void );
  void previewClicked( void );

  void colorPointsChanged( const QPolygonF &points );

signals:

  void colorChanged( void );
  void previewColor( void );

  void sizeChanged( void );
  void sizePreview( void );

protected:

  TSizeFunction pointsToSizeFunc( const QPolygonF& points );
  TSizeFunction pointsToSizeFunc( const QPolygonF& points,
                                  float minValue,
                                  float maxValue );
  QPolygonF sizeFuncToPoints( const TSizeFunction& sizeFunc );

  virtual void mousePressEvent( QMouseEvent * event );

  void InitDialog( void );

  // Color transfer function points
  ColorPoints *_redPoints;
  ColorPoints *_bluePoints;
  ColorPoints *_greenPoints;
  ColorPoints *_alphaPoints;

  // Size transfer function points
  ColorPoints* _sizePoints;
  Gradient* _sizeFrame;

  QDoubleSpinBox* _minSizeBox;
  QDoubleSpinBox* _maxSizeBox;

  TSizeFunction _sizeFunction;
  float _minSize;
  float _maxSize;
  QLabel* _minValueLabel;
  QLabel* _maxValueLabel;

//    osg::ref_ptr<osg::Texture1D> _texture;

  Gradient* _gradientFrame;
  Gradient* _nTGradientFrame;
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
