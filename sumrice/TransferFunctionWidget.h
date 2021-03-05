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

#ifndef __TRANSFERFUNCTIONWIDGET_H__
#define __TRANSFERFUNCTIONWIDGET_H__

// Sumrice
#include <sumrice/api.h>
#include <sumrice/types.h>

// Preft
#include <prefr/prefr.h>

// Qt
#include <QWidget>

class ColorPoints;
class Gradient;

class QPushButton;
class QMouseEvent;
class QDoubleSpinBox;
class QLabel;
class QComboBox;

class SUMRICE_API TransferFunctionWidget : public QWidget
{
  Q_OBJECT;

public:

    class Preset
    {
      public:
        Preset(const QString &name_, const QGradientStops &stops_)
        : _name(name_)
        , _stops(stops_)
        {}

        const QString& name() const
        {
          return _name;
        }

        const QGradientStops& stops() const
        {
          return _stops;
        }

      protected:
        QString _name;
        QGradientStops _stops;
    };

  TransferFunctionWidget(QWidget* parent = nullptr);
  virtual ~TransferFunctionWidget( )
  {};

  visimpl::TTransferFunction getColors( bool includeAlpha = true ) const;

  void setColorPoints( const visimpl::TTransferFunction& colors,
                       bool updateResult = true );

  visimpl::TSizeFunction getSizeFunction( void ) const;
  void setSizeFunction( const visimpl::TSizeFunction& sizeFunc );


  visimpl::TTransferFunction getPreviewColors( void ) const;
  visimpl::TSizeFunction getSizePreview( void ) const;

protected slots:
  void gradientClicked( void );
  void acceptClicked( void );
  void cancelClicked( void );
  void previewClicked( void );
  void colorPointsChanged( const QPolygonF &points );
  void presetSelected( int presetIdx );

signals:
  void colorChanged( void );
  void previewColor( void );

  void sizeChanged( void );
  void sizePreview( void );

protected:
  visimpl::TSizeFunction pointsToSizeFunc( const QPolygonF& points ) const;
  visimpl::TSizeFunction pointsToSizeFunc( const QPolygonF& points,
                                  float minValue,
                                  float maxValue ) const;
  QPolygonF sizeFuncToPoints( const visimpl::TSizeFunction& sizeFunc );

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

  visimpl::TSizeFunction _sizeFunction;
  float _minSize;
  float _maxSize;
  QLabel* _minValueLabel;
  QLabel* _maxValueLabel;

  Gradient* _gradientFrame;
  Gradient* _nTGradientFrame;
  Gradient* redGradientFrame;
  Gradient* greenGradientFrame;
  Gradient* blueGradientFrame;
  Gradient* alphaGradientFrame;

  QWidget* _dialog;
  QComboBox* _presetsComboBox;
  QPushButton* _saveButton;
  QPushButton* _discardButton;
  QPushButton* _previewButton;
  bool previewed;

  Gradient* _result;
  QGradientStops _tResult;

  std::vector< Preset > _presets;
};

#endif /* __TRANSFERFUNCTIONWIDGET_H__ */
