/*
 * ParticleSizeWidget.h
 *
 *  Created on: 4 de mar. de 2016
 *      Author: sgalindo
 */

#ifndef __PARTICLESIZEWIDGET_H__
#define __PARTICLESIZEWIDGET_H__

#include <QWidget>
#include <QPushButton>
#include <QMouseEvent>
#include <QDoubleSpinBox>
#include <QLabel>

#include "EditorTF/ColorPoints.h"
#include "EditorTF/Gradient.h"

class ParticleSizeWidget : public QWidget
{
  Q_OBJECT;

public:

  ParticleSizeWidget( QWidget* parent = 0 );

  TSizeFunction getSizeFunction( void );

  void setSizeFunction( const TSizeFunction& sizeFunc );

  TSizeFunction getSizePreview( void );

  void SetFrameBackground( const TTransferFunction& colors_ );

signals:

  void sizeChanged( void );
  void sizePreview( void );

protected slots:

  void gradientClicked( void );

  void acceptClicked( void );
  void cancelClicked( void );
  void previewClicked( void );

protected:

  TSizeFunction pointsToSizeFunc( const QPolygonF& points );
  TSizeFunction pointsToSizeFunc( const QPolygonF& points,
                                  float minValue,
                                  float maxValue );
  QPolygonF sizeFuncToPoints( const TSizeFunction& sizeFunc );


  virtual void mousePressEvent( QMouseEvent * event );

  void InitDialog( void );

  ColorPoints* sizePoints;
  Gradient* _sizeFrame;
  QDoubleSpinBox* _minSizeBox;
  QDoubleSpinBox* _maxSizeBox;

  TSizeFunction _sizeFunction;
  float _minSize;
  float _maxSize;

  Gradient* _result;
  QLabel* _minValueLabel;
  QLabel* _maxValueLabel;

  QWidget* _dialog;
  QPushButton* _saveButton;
  QPushButton* _discardButton;
  QPushButton* _previewButton;
  bool previewed;
};



#endif /* __PARTICLESIZEWIDGET_H__ */
