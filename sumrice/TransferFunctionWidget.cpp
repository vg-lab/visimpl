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

// Sumrice
#include "TransferFunctionWidget.h"
#include "EditorTF/ColorPoints.h"
#include "EditorTF/Gradient.h"

// C++
#include <iostream>
#include <limits>
#include <fstream>

// Qt
#include <QPushButton>
#include <QMouseEvent>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>

TransferFunctionWidget::TransferFunctionWidget( QWidget* parent_ ) :
QWidget( parent_ )
{
  InitDialog( );

  _result = new Gradient( );
  _result->setDirection( Gradient::HORIZONTAL );

  _result->setGradientStops( _nTGradientFrame->getGradientStops( ));
  _tResult = _gradientFrame->getGradientStops( );

  setSizeFunction( _sizeFunction );

  QGridLayout* mainLayout = new QGridLayout( );
  mainLayout->addWidget( _result, 0, 0, 1, 5 );

  // Size labels
  mainLayout->addWidget( _minValueLabel, 1, 1, 1, 1 );
  mainLayout->addWidget( _maxValueLabel, 1, 3, 1, 1 );

  this->setLayout( mainLayout );
}

void TransferFunctionWidget::addPreset(const Preset &p)
{
  _presets.push_back(p);
  _presetsComboBox->addItem(p.name());
  _presetsComboBox->setCurrentIndex(_presets.size()-1);
}

void TransferFunctionWidget::InitDialog( void )
{
  _dialog = new QWidget( );
  _dialog->setWindowModality( Qt::NonModal );
  _dialog->setWindowTitle(tr("Color Transfer Functions"));
  _dialog->setMinimumSize( 800, 600 );

  _presetsComboBox = new QComboBox( _dialog );
  {
    QGradientStops _stops;

    _stops.clear( );
    _stops << qMakePair( 0.0,  QColor::fromRgbF( 0.0, 1.0, 0.0, 0.2 ))
           << qMakePair( 0.35, QColor::fromRgbF( 1.0, 0.0, 0.0, 0.5  ))
           << qMakePair( 0.7,  QColor::fromRgbF( 1.0, 1.0, 0.0, 0.5  ))
           << qMakePair( 1.0,  QColor::fromRgbF( 0.0, 0.0, 1.0, 0.5  ));
    _presets.push_back( Preset( "Default [ multi-hue]", _stops ));

    _stops.clear( );
    _stops << qMakePair( 0.0, QColor( 255, 0, 0, 127 ))
           << qMakePair( 1.0, QColor( 0, 0, 255, 127 ));
    _presets.push_back( Preset( "Red-blue [ multi-hue]", _stops ));

    _stops.clear( );
    _stops << qMakePair( 0.0, QColor( 255, 0, 0, 127 ))
           << qMakePair( 1.0, QColor( 0, 255, 0, 127 ));
    _presets.push_back( Preset( "Red-green [ multi-hue]", _stops ));

    _stops.clear( );
    _stops << qMakePair( 0.0, QColor( 255, 0, 0, 100 ))
           << qMakePair( 0.1, QColor( 255, 0, 0, 200 ))
           << qMakePair( 1.0, QColor( 0, 0, 0, 20 ));
    _presets.push_back( Preset( "Red [ mono-hue]", _stops ));

    _stops.clear( );
    _stops << qMakePair( 0.0, QColor( 0, 255, 0, 100 ))
           << qMakePair( 1.0, QColor( 0, 0, 0, 0 ));
    _presets.push_back( Preset( "Green [ mono-hue]", _stops ));

    _stops.clear( );
    _stops << qMakePair( 0.0, QColor( 0, 0, 255, 100 ))
           << qMakePair( 1.0, QColor( 0, 0, 0, 0 ));
    _presets.push_back( Preset( "Blue [ mono-hue]", _stops ));

    _stops.clear( );
    _stops << qMakePair( 0.0, QColor::fromHsv( 60, 255, 255, 100 ))
           << qMakePair( 1.0, QColor::fromHsv( 60, 128, 128,  0 ))
           << qMakePair( 1.0, QColor::fromHsv( 60,   0,   0,  0 ));
    _presets.push_back( Preset( "Yellow [ mono-hue]", _stops ));

    for ( const auto& preset : _presets )
      _presetsComboBox->addItem( preset.name( ));

    connect( _presetsComboBox, SIGNAL( currentIndexChanged( int )),
             this, SLOT( presetSelected( int )));
  }

  _saveButton = new QPushButton( "Save" );
  _discardButton = new QPushButton( "Discard" );
  _previewButton = new QPushButton( "Preview" );

  _gradientFrame = new Gradient( );
  _nTGradientFrame = new Gradient( );

  redGradientFrame = new Gradient( );
  greenGradientFrame = new Gradient( );
  blueGradientFrame = new Gradient( );
  alphaGradientFrame = new Gradient( );

  _maxSizeBox = new QDoubleSpinBox( );
  _maxSizeBox->setMinimum( 1.0 );
  _maxSizeBox->setMaximum( 300.0 );
  _minSizeBox = new QDoubleSpinBox( );
  _maxSizeBox->setMinimum( 1.0 );
  _maxSizeBox->setMaximum( 300.0 );

  connect(_minSizeBox, SIGNAL(valueChanged(double)), this, SLOT(onSizeValueChanged(double)));
  connect(_maxSizeBox, SIGNAL(valueChanged(double)), this, SLOT(onSizeValueChanged(double)));

  _minValueLabel = new QLabel( );
  _maxValueLabel = new QLabel( );

  _sizeFrame = new Gradient( );

  unsigned int row = 0;
  unsigned int totalColumns = 6;
  auto dialogLayout = new QGridLayout( );

  // Presets
  dialogLayout->addWidget( new QLabel( "Presets:" ), row, 1, 1, 1 );
  dialogLayout->addWidget( _presetsComboBox, row++, 2, 1, 3 );

  dialogLayout->addWidget( new QLabel( "Red" ), row, 0, 1, 1 );
  dialogLayout->addWidget( redGradientFrame, row++, 1, 1, totalColumns );
  dialogLayout->addWidget( new QLabel( "Green" ), row, 0, 1, 1 );
  dialogLayout->addWidget( greenGradientFrame, row++, 1, 1, totalColumns );
  dialogLayout->addWidget( new QLabel( "Blue" ), row, 0, 1, 1 );
  dialogLayout->addWidget( blueGradientFrame, row++, 1, 1, totalColumns );
  dialogLayout->addWidget( new QLabel( "Alpha" ), row, 0, 1, 1 );
  dialogLayout->addWidget( alphaGradientFrame, row++, 1, 1, totalColumns );

  dialogLayout->addWidget( new QLabel( "Result (pure)" ), row, 0, 1, 1 );
  dialogLayout->addWidget( _nTGradientFrame, row++, 1, 1, totalColumns );
  dialogLayout->addWidget( new QLabel( "Result (alpha)" ), row, 0, 1, 1 );
  dialogLayout->addWidget( _gradientFrame, row++, 1, 1, totalColumns );

  dialogLayout->addWidget( new QLabel( "Size" ), row, 0, 1, 1 );
  dialogLayout->addWidget( _sizeFrame, row++, 1, 1, totalColumns);
  row++;
  unsigned int sizeLabels = totalColumns / 3;
  dialogLayout->addWidget( new QLabel( "Min size:"), row, sizeLabels - 1, 1, 1);
  dialogLayout->addWidget( _minSizeBox, row, sizeLabels , 1, 1 );
  sizeLabels *= 2;
  dialogLayout->addWidget( new QLabel( "Max size:"), row, sizeLabels - 1, 1, 1);
  dialogLayout->addWidget( _maxSizeBox, row, sizeLabels, 1, 1 );

  row++;
  dialogLayout->addWidget( _discardButton, row, 1, 1, 1 );
  dialogLayout->addWidget( _previewButton, row, 3, 1, 1 );
  dialogLayout->addWidget( _saveButton, row, 5, 1, 1 );

  _dialog->setLayout( dialogLayout );

  /* Setting gradient frames */
  QGradientStops stops;
  stops << qMakePair(0.0, QColor(0, 0, 0, 0))
        << qMakePair(1.0, QColor(255, 255, 255, 255));
  _gradientFrame->setDirection(Gradient::HORIZONTAL);
  _gradientFrame->setGradientStops(stops);

  QGradientStops ntStops;
  stops << qMakePair(0.0, QColor(0, 0, 0, 255))
        << qMakePair(1.0, QColor(255, 255, 255, 255));
  _nTGradientFrame->setDirection(Gradient::HORIZONTAL);
  _nTGradientFrame->setGradientStops(stops);

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

  QGradientStops sizeStops;
  sizeStops << qMakePair( 0.0, QColor( 127, 127, 127, 127 ))
            << qMakePair( 1.0, QColor( 127, 127, 127, 127 ));
  _sizeFrame->setDirection( Gradient::HORIZONTAL );
  _sizeFrame->setGradientStops( sizeStops );

  QPolygonF sPoints;
  sPoints << QPointF(0, 0) << QPointF(1, 1);

  _sizePoints = new ColorPoints( _sizeFrame );
  _sizePoints->setPoints( sPoints );

  /* Connecting slots */
  connect( _saveButton, SIGNAL( clicked( void )),
           this, SLOT( acceptClicked( void )));
  connect( _discardButton, SIGNAL( clicked( void )),
           this, SLOT( cancelClicked( void )));

  connect( _previewButton, SIGNAL( clicked( void )),
           this, SLOT( previewClicked( void) ));

  const QList<ColorPoints *> colorPoints = { _redPoints, _greenPoints, _bluePoints, _alphaPoints };
  for(auto &cp1: colorPoints)
  {
    for(auto &cp2: colorPoints)
    {
      if(cp1 == cp2) continue;
      connect(cp1, SIGNAL(pointInserted(int, float)), cp2, SLOT(insertPoint(int, float)));
      connect(cp1, SIGNAL(pointRemoved(int)), cp2, SLOT(removePoint(int)));
      connect(cp1, SIGNAL(pointAbscissaChanged(int, float)), cp2, SLOT(movePointAbscissa(int, float)));
    }

    connect(cp1, SIGNAL(pointsChanged(const QPolygonF &)), this, SLOT(colorPointsChanged(const QPolygonF &)));
  }
}

visimpl::TTransferFunction TransferFunctionWidget::getColors(bool includeAlpha) const
{
  visimpl::TTransferFunction result;

  const QGradientStops &stops = includeAlpha ? _tResult : _result->getGradientStops( );

  auto insertStop = [&result](const QGradientStop &stop)
  {
    result.push_back( std::make_pair( float( stop.first ), stop.second ));
  };
  std::for_each(stops.cbegin(), stops.cend(), insertStop);

  return result;
}

visimpl::TTransferFunction TransferFunctionWidget::getPreviewColors() const
{
  visimpl::TTransferFunction result;

  auto stops = _gradientFrame->getGradientStops( );

  auto insertStop = [&result](const QGradientStop &stop)
  {
    result.push_back( std::make_pair( float( stop.first ), stop.second ));
  };
  std::for_each(stops.cbegin(), stops.cend(), insertStop);

  return result;
}

void TransferFunctionWidget::colorPointsChanged( const QPolygonF &points )
{
  auto pointSet = qobject_cast<ColorPoints *>(sender());
  if(!pointSet) return;

  const std::vector<ColorPoints *> colorPoints = {_redPoints, _greenPoints, _bluePoints, _alphaPoints};
  auto it = std::find(colorPoints.cbegin(), colorPoints.cend(), pointSet);
  if(it == colorPoints.cend()) return;

  const int indexToUpdate = std::distance(colorPoints.cbegin(), it);

  auto stops = _gradientFrame->getGradientStops();
  stops.resize(points.size());

  auto ntStops = stops;
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

  _gradientFrame->setGradientStops(stops);
  _nTGradientFrame->setGradientStops( ntStops );
  _sizeFrame->setGradientStops( ntStops );
}

void TransferFunctionWidget::setColorPoints( const visimpl::TTransferFunction& colors,
                                             bool updateResult )
{
  QPolygonF redPoints;
  QPolygonF greenPoints;
  QPolygonF bluePoints;
  QPolygonF alphaPoints;

  auto insertColor = [&](const visimpl::TTFColor &c)
  {
    redPoints.append( QPointF( c.first, c.second.redF( )));
    greenPoints.append( QPointF( c.first, c.second.greenF( )));
    bluePoints.append( QPointF( c.first, c.second.blueF( )));
    alphaPoints.append( QPointF( c.first, c.second.alphaF( )));
  };
  std::for_each(colors.cbegin(), colors.cend(), insertColor);

  _redPoints->setPoints( redPoints, true );
  _greenPoints->setPoints( greenPoints, true );
  _bluePoints->setPoints( bluePoints, true );
  _alphaPoints->setPoints( alphaPoints, true );

  if( updateResult )
  {
    _result->setGradientStops( _nTGradientFrame->getGradientStops( ));
    _tResult = _gradientFrame->getGradientStops( );
    _sizeFrame->setGradientStops( _nTGradientFrame->getGradientStops( ));
  }
}

visimpl::TSizeFunction TransferFunctionWidget::getSizeFunction( void ) const
{
  return _sizeFunction;
}

visimpl::TSizeFunction TransferFunctionWidget::getSizePreview( void ) const
{
  return pointsToSizeFunc( _sizePoints->points( ),
                           _minSizeBox->value( ),
                           _maxSizeBox->value( ));
}

visimpl::TSizeFunction TransferFunctionWidget::pointsToSizeFunc( const QPolygonF& points ) const
{
  return pointsToSizeFunc( points, _minSize, _maxSize );
}

visimpl::TSizeFunction TransferFunctionWidget::pointsToSizeFunc( const QPolygonF &points,
                                                    float minSize,
                                                    float maxSize ) const
{
  visimpl::TSizeFunction result;

  auto insertPoint = [&result, minSize, maxSize](const QPointF &p)
  {
    const float time = p.x( );
    const float value = p.y( ) * ( maxSize - minSize) + minSize;
    result.push_back( std::make_pair( time, value ));
  };
  std::for_each(points.cbegin(), points.cend(), insertPoint);

  return result;
}

void TransferFunctionWidget::setSizeFunction( const visimpl::TSizeFunction& sizeFunc )
{
  _minSize = std::numeric_limits< float >::max( );
  _maxSize = std::numeric_limits< float >::min( );

  auto updateSizeLimits = [this](const visimpl::TSize &value)
  {
    _minSize = std::min(_minSize, value.second);
    _maxSize = std::max(_maxSize, value.second);
  };
  std::for_each(sizeFunc.cbegin(), sizeFunc.cend(), updateSizeLimits);

  const float invTotal = 1.0f / ( _maxSize - _minSize ) ;

  QPolygonF result;
  auto insertPoint = [&result, invTotal, this](const visimpl::TSize &value)
  {
    result.append( QPointF( value.first, ( value.second - _minSize ) * invTotal ));
  };
  std::for_each(sizeFunc.cbegin(), sizeFunc.cend(), insertPoint);

  if(sizeFunc.empty())
  {
    _minValueLabel->setText( QString("Min size: Unknown") );
    _maxValueLabel->setText( QString("Max size: Unknown") );

    // Set a default valid value.
    _minSize = 10.0;
    _maxSize = 20.0;
    result.clear();
    result.append(QPointF{0,1});
    result.append(QPointF{1,0});
    _sizePoints->setPoints(result, true );
    _sizeFunction = pointsToSizeFunc(result, 10, 20);
  }
  else
  {
    _minValueLabel->setText( QString("Min size: ") +
                             QString::number( static_cast<double>( _minSize ) ));

    _maxValueLabel->setText( QString("Max size: ") +
                             QString::number( static_cast<double>( _maxSize ) ));

    _sizePoints->setPoints( result, true );
    _sizeFunction = sizeFunc;
  }

  _minSizeBox->setValue( _minSize );
  _maxSizeBox->setValue( _maxSize );

  _result->plot( result );
}

void TransferFunctionWidget::gradientClicked( void )
{
  if( !_dialog )
    return;

  if( !_dialog->isVisible( ))
  {
    setColorPoints( getColors( ), false);
    setSizeFunction( getSizeFunction( ));

    _dialog->show( );
  }
  else
  {
    _dialog->activateWindow( );
    _dialog->setFocus( );
  }
}

void TransferFunctionWidget::acceptClicked( void )
{
  _dialog->close( );

  _result->setGradientStops( _nTGradientFrame->getGradientStops( ));
  _tResult = _gradientFrame->getGradientStops( );
  _sizeFrame->setGradientStops( _nTGradientFrame->getGradientStops( ));

  _maxSize = static_cast<float>( _maxSizeBox->value( ));
  _minSize = static_cast<float>( _minSizeBox->value( ));

  _minValueLabel->setText( QString("Min size: ") +
                          QString::number( static_cast<double>( _minSize ) ));

  _maxValueLabel->setText( QString("Max size: ") +
                          QString::number( static_cast<double>( _maxSize ) ));

  _sizeFunction = pointsToSizeFunc( _sizePoints->points( ));

  _result->plot( _sizePoints->points( ));

  emit colorChanged( );
  emit sizeChanged( );
}

void TransferFunctionWidget::cancelClicked( void )
{
  _dialog->close( );

  if( previewed )
  {
    emit colorChanged( );
    emit sizeChanged( );
  }
}

void TransferFunctionWidget::previewClicked( void )
{
  previewed = true;
  emit previewColor( );
  emit sizePreview( );
}

void TransferFunctionWidget::mousePressEvent(QMouseEvent * event_ )
{
  if( event_->button( ) == Qt::LeftButton )
  {
    emit gradientClicked( );
  }
}

void TransferFunctionWidget::presetSelected( int presetIdx )
{
  const auto& stops = _presets[ presetIdx ].stops( );

  QPolygonF redPoints;
  QPolygonF greenPoints;
  QPolygonF bluePoints;
  QPolygonF alphaPoints;

  auto insertStop = [&](const QGradientStop &stop)
  {
    redPoints.append( QPointF( stop.first, stop.second.redF( )));
    greenPoints.append( QPointF( stop.first, stop.second.greenF( )));
    bluePoints.append( QPointF( stop.first, stop.second.blueF( )));
    alphaPoints.append( QPointF( stop.first, stop.second.alphaF( )));
  };
  std::for_each(stops.cbegin(), stops.cend(), insertStop);

  _redPoints->setPoints( redPoints, true );
  _greenPoints->setPoints( greenPoints, true );
  _bluePoints->setPoints( bluePoints, true );
  _alphaPoints->setPoints( alphaPoints, true );

  _result->setGradientStops( _nTGradientFrame->getGradientStops( ));
  _tResult = _gradientFrame->getGradientStops( );
  _sizeFrame->setGradientStops( _nTGradientFrame->getGradientStops( ));
}

void TransferFunctionWidget::onSizeValueChanged(double value)
{
  auto updateBoxValue = [](QDoubleSpinBox* w, double val)
  {
    w->blockSignals(true);
    w->setValue(val);
    w->blockSignals(false);
  };

  auto box = qobject_cast<QDoubleSpinBox *>(sender());
  if(box)
  {
    if(box == _maxSizeBox)
    {
      updateBoxValue(box, std::max(value, _minSizeBox->value()));
    }
    else
    {
      updateBoxValue(box, std::min(value, _maxSizeBox->value()));
    }
  }
}
