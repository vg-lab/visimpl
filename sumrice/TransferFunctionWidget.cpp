/*
 * @file  TransferFunctionWidget.cpp
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */

#include "TransferFunctionWidget.h"

#include <iostream>
#include <limits>
#include <fstream>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

//typedef std::map<float, osg::Vec4> Colors;

/*
  TransferFunctionWidget::Impl
*/

TransferFunctionWidget::TransferFunctionWidget( QWidget* parent_ ) :
QWidget( parent_ )
{

  InitDialog( );

  _result = new Gradient( );
  _result->setDirection( Gradient::HORIZONTAL );

  _result->setGradientStops( _nTGradientFrame->getGradientStops( ));
  _tResult = _gradientFrame->getGradientStops( );

  setSizeFunction( _sizeFunction );

//  connect( this, SIGNAL( clicked( void )),
//           this, SLOT( gradientClicked( void )));

  QGridLayout* mainLayout = new QGridLayout( );
  mainLayout->addWidget( _result, 0, 0, 1, 5 );

  // Size labels
  mainLayout->addWidget( _minValueLabel, 1, 1, 1, 1 );
  mainLayout->addWidget( _maxValueLabel, 1, 3, 1, 1 );

  this->setLayout( mainLayout );

}

TransferFunctionWidget::~TransferFunctionWidget()
{
}

void TransferFunctionWidget::InitDialog( void )
{
  _dialog = new QWidget( );
  _dialog->setWindowModality( Qt::NonModal );
  _dialog->setMinimumSize( 800, 600 );

  _presetsComboBox = new QComboBox( _dialog );
  {
    QGradientStops stops;

    stops.clear( );
    stops << qMakePair( 0.0,  QColor::fromRgbF( 0.0, 1.0, 0.0, 0.05 ))
          << qMakePair( 0.35, QColor::fromRgbF( 1.0, 0.0, 0.0, 0.2  ))
          << qMakePair( 0.7,  QColor::fromRgbF( 1.0, 1.0, 0.0, 0.2  ))
          << qMakePair( 1.0,  QColor::fromRgbF( 0.0, 0.0, 1.0, 0.2  ));
    _presets.push_back( Preset( "Default [ multi-hue]", stops ));

     stops.clear( );
    stops << qMakePair( 0.0, QColor( 255, 0, 0, 127 ))
          << qMakePair( 1.0, QColor( 0, 0, 255, 127 ));
    _presets.push_back( Preset( "Red-blue [ multi-hue]", stops ));

    stops.clear( );
    stops << qMakePair( 0.0, QColor( 255, 0, 0, 127 ))
          << qMakePair( 1.0, QColor( 0, 255, 0, 127 ));
    _presets.push_back( Preset( "Red-green [ multi-hue]", stops ));

    stops.clear( );
    stops << qMakePair( 0.0, QColor( 255, 0, 0, 50 ))
          << qMakePair( 0.1, QColor( 255, 0, 0, 200 ))
          << qMakePair( 1.0, QColor( 0, 0, 0, 20 ));
    _presets.push_back( Preset( "Red [ mono-hue]", stops ));

    stops.clear( );
    stops << qMakePair( 0.0, QColor( 0, 255, 0, 50 ))
          << qMakePair( 1.0, QColor( 0, 0, 0, 0 ));
    _presets.push_back( Preset( "Green [ mono-hue]", stops ));

    stops.clear( );
    stops << qMakePair( 0.0, QColor( 0, 0, 255, 50 ))
          << qMakePair( 1.0, QColor( 0, 0, 0, 0 ));
    _presets.push_back( Preset( "Blue [ mono-hue]", stops ));

    stops.clear( );
    stops << qMakePair( 0.0, QColor::fromHsv( 60, 255, 255, 50 ))
          << qMakePair( 1.0, QColor::fromHsv( 60, 128, 128,  0 ))
          << qMakePair( 1.0, QColor::fromHsv( 60,   0,   0,  0 ));
    _presets.push_back( Preset( "Yellow [ mono-hue]", stops ));

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

  _minValueLabel = new QLabel( );
  _maxValueLabel = new QLabel( );

  _sizeFrame = new Gradient( );

  unsigned int row = 0;
  unsigned int totalColumns = 6;
  QGridLayout* dialogLayout = new QGridLayout( );

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

  connect(_redPoints, SIGNAL(pointInserted(int, float)),
          _greenPoints, SLOT(insertPoint(int, float)));
  connect(_redPoints, SIGNAL(pointInserted(int, float)),
          _bluePoints, SLOT(insertPoint(int, float)));
  connect(_redPoints, SIGNAL(pointInserted(int, float)),
          _alphaPoints, SLOT(insertPoint(int, float)));

  connect(_greenPoints, SIGNAL(pointInserted(int, float)),
          _redPoints, SLOT(insertPoint(int, float)));
  connect(_greenPoints, SIGNAL(pointInserted(int, float)),
          _bluePoints, SLOT(insertPoint(int, float)));
  connect(_greenPoints, SIGNAL(pointInserted(int, float)),
          _alphaPoints, SLOT(insertPoint(int, float)));

  connect(_bluePoints, SIGNAL(pointInserted(int, float)),
          _redPoints, SLOT(insertPoint(int, float)));
  connect(_bluePoints, SIGNAL(pointInserted(int, float)),
          _greenPoints, SLOT(insertPoint(int, float)));
  connect(_bluePoints, SIGNAL(pointInserted(int, float)),
          _alphaPoints, SLOT(insertPoint(int, float)));

  connect(_alphaPoints, SIGNAL(pointInserted(int, float)),
          _redPoints, SLOT(insertPoint(int, float)));
  connect(_alphaPoints, SIGNAL(pointInserted(int, float)),
          _greenPoints, SLOT(insertPoint(int, float)));
  connect(_alphaPoints, SIGNAL(pointInserted(int, float)),
          _bluePoints, SLOT(insertPoint(int, float)));

  connect(_redPoints, SIGNAL(pointRemoved(int)),
          _greenPoints, SLOT(removePoint(int)));
  connect(_redPoints, SIGNAL(pointRemoved(int)),
          _bluePoints, SLOT(removePoint(int)));
  connect(_redPoints, SIGNAL(pointRemoved(int)),
          _alphaPoints, SLOT(removePoint(int)));

  connect(_greenPoints, SIGNAL(pointRemoved(int)),
          _redPoints, SLOT(removePoint(int)));
  connect(_greenPoints, SIGNAL(pointRemoved(int)),
          _bluePoints, SLOT(removePoint(int)));
  connect(_greenPoints, SIGNAL(pointRemoved(int)),
          _alphaPoints, SLOT(removePoint(int)));

  connect(_bluePoints, SIGNAL(pointRemoved(int)),
          _redPoints, SLOT(removePoint(int)));
  connect(_bluePoints, SIGNAL(pointRemoved(int)),
          _greenPoints, SLOT(removePoint(int)));
  connect(_bluePoints, SIGNAL(pointRemoved(int)),
          _alphaPoints, SLOT(removePoint(int)));

  connect(_alphaPoints, SIGNAL(pointRemoved(int)),
          _redPoints, SLOT(removePoint(int)));
  connect(_alphaPoints, SIGNAL(pointRemoved(int)),
          _greenPoints, SLOT(removePoint(int)));
  connect(_alphaPoints, SIGNAL(pointRemoved(int)),
          _bluePoints, SLOT(removePoint(int)));

  connect(_redPoints, SIGNAL(pointAbscissaChanged(int, float)),
          _greenPoints, SLOT(movePointAbscissa(int, float)));
  connect(_redPoints, SIGNAL(pointAbscissaChanged(int, float)),
          _bluePoints, SLOT(movePointAbscissa(int, float)));
  connect(_redPoints, SIGNAL(pointAbscissaChanged(int, float)),
          _alphaPoints, SLOT(movePointAbscissa(int, float)));

  connect(_greenPoints, SIGNAL(pointAbscissaChanged(int, float)),
          _redPoints, SLOT(movePointAbscissa(int, float)));
  connect(_greenPoints, SIGNAL(pointAbscissaChanged(int, float)),
          _bluePoints, SLOT(movePointAbscissa(int, float)));
  connect(_greenPoints, SIGNAL(pointAbscissaChanged(int, float)),
          _alphaPoints, SLOT(movePointAbscissa(int, float)));

  connect(_bluePoints, SIGNAL(pointAbscissaChanged(int, float)),
          _redPoints, SLOT(movePointAbscissa(int, float)));
  connect(_bluePoints, SIGNAL(pointAbscissaChanged(int, float)),
          _greenPoints, SLOT(movePointAbscissa(int, float)));
  connect(_bluePoints, SIGNAL(pointAbscissaChanged(int, float)),
          _alphaPoints, SLOT(movePointAbscissa(int, float)));

  connect(_alphaPoints, SIGNAL(pointAbscissaChanged(int, float)),
          _redPoints, SLOT(movePointAbscissa(int, float)));
  connect(_alphaPoints, SIGNAL(pointAbscissaChanged(int, float)),
          _greenPoints, SLOT(movePointAbscissa(int, float)));
  connect(_alphaPoints, SIGNAL(pointAbscissaChanged(int, float)),
          _bluePoints, SLOT(movePointAbscissa(int, float)));

  connect(_redPoints, SIGNAL(pointsChanged(const QPolygonF &)),
          this, SLOT(colorPointsChanged(const QPolygonF &)));
  connect(_greenPoints, SIGNAL(pointsChanged(const QPolygonF &)),
          this, SLOT(colorPointsChanged(const QPolygonF &)));
  connect(_bluePoints, SIGNAL(pointsChanged(const QPolygonF &)),
          this, SLOT(colorPointsChanged(const QPolygonF &)));
  connect(_alphaPoints, SIGNAL(pointsChanged(const QPolygonF &)),
          this, SLOT(colorPointsChanged(const QPolygonF &)));
}

TTransferFunction TransferFunctionWidget::getColors( bool includeAlpha )
{
  TTransferFunction result;

  for( auto stop : ( includeAlpha ? _tResult : _result->getGradientStops( )) )
  {
    result.push_back( std::make_pair( float( stop.first ), stop.second ));
  }

  return result;
}

TTransferFunction TransferFunctionWidget::getPreviewColors( void )
{
  TTransferFunction result;

  QGradientStops stops = _gradientFrame->getGradientStops( );

  for( auto stop : stops )
  {
    result.push_back( std::make_pair( float( stop.first ), stop.second ));
  }

  return result;
}


void TransferFunctionWidget::colorPointsChanged( const QPolygonF &points )
{
    QObject *pointSet = sender();
    int indexToUpdate = 0;
    if (pointSet == _redPoints)
        indexToUpdate = 0;
    else if (pointSet == _greenPoints)
        indexToUpdate = 1;
    else if (pointSet == _bluePoints)
        indexToUpdate = 2;
    else if (pointSet == _alphaPoints)
        indexToUpdate = 3;

    QGradientStops stops = _gradientFrame->getGradientStops();

    stops.resize(points.size());

    QGradientStops ntStops = stops;
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

void TransferFunctionWidget::setColorPoints( const TTransferFunction& colors,
                                             bool updateResult )
{
//  QGradientStops gradientColors;
//  QGradientStops ntGradientColors;
  QPolygonF redPoints;
  QPolygonF greenPoints;
  QPolygonF bluePoints;
  QPolygonF alphaPoints;

  for( auto c : colors )
  {

    redPoints.append( QPointF( c.first, c.second.redF( )));
    greenPoints.append( QPointF( c.first, c.second.greenF( )));
    bluePoints.append( QPointF( c.first, c.second.blueF( )));
    alphaPoints.append( QPointF( c.first, c.second.alphaF( )));

  }

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


TSizeFunction TransferFunctionWidget::getSizeFunction( void )
{
  return _sizeFunction;
}

TSizeFunction TransferFunctionWidget::getSizePreview( void )
{
  return pointsToSizeFunc( _sizePoints->points( ),
                           _minSizeBox->value( ),
                           _maxSizeBox->value( ));
}

TSizeFunction TransferFunctionWidget::pointsToSizeFunc( const QPolygonF& points )
{
  return pointsToSizeFunc( points, _minSize, _maxSize );
}

TSizeFunction TransferFunctionWidget::pointsToSizeFunc( const QPolygonF &points,
                                                    float minSize,
                                                    float maxSize )
{

  TSizeFunction result;
  float time;
  float value;
//  float invHeight = 1.0f / float( height( ));
//  float invWidth = 1.0f / float( width( ));

  for( auto point : points)
  {
    time = point.x( );
    value = point.y( ) * ( maxSize - minSize) + minSize;
    result.push_back( std::make_pair( time, value ));
  }

  return result;
}

void TransferFunctionWidget::setSizeFunction( const TSizeFunction& sizeFunc )
{

  _sizeFunction = sizeFunc;
  QPolygonF result;

  _minSize = std::numeric_limits< float >::max( );
  _maxSize = std::numeric_limits< float >::min( );
  for( auto value : sizeFunc )
  {
    if( value.second < _minSize )
      _minSize = value.second;

    if( value.second > _maxSize )
      _maxSize = value.second;
  }

  float invTotal = 1.0f / ( _maxSize - _minSize ) ;

//  auto valueIt = sizeFunc.values.begin( );
  for( auto time : sizeFunc )
  {
    result.append( QPointF( time.first, ( time.second - _minSize ) * invTotal ));

//    valueIt++;
  }

  _sizePoints->setPoints( result, true );
  _minValueLabel->setText( QString("Min size: ") +
                           QString::number( double( _minSize ) ));

  _maxValueLabel->setText( QString("Max size: ") +
                           QString::number( double( _maxSize ) ));

  _minSizeBox->setValue( _minSize );
  _maxSizeBox->setValue( _maxSize );

  //TODO draw plot on gradient
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

//void TransferFunctionWidget::buttonClicked( QAbstractButton* button )
//{
//  _dialog->close( );
//
//  if( button == _saveButton )
//  {
//    _result->setGradientStops( _nTGradientFrame->getGradientStops( ));
//    _tResult = _gradientFrame->getGradientStops( );
//    emit colorChanged( );
//  }
//  else if( button == _discardButton )
//  {
//    if( previewed )
//      emit colorChanged( );
//  }
//}

void TransferFunctionWidget::acceptClicked( void )
{
  _dialog->close( );

  _result->setGradientStops( _nTGradientFrame->getGradientStops( ));
  _tResult = _gradientFrame->getGradientStops( );
  _sizeFrame->setGradientStops( _nTGradientFrame->getGradientStops( ));

  _maxSize = float( _maxSizeBox->value( ));
  _minSize = float( _minSizeBox->value( ));

  _minValueLabel->setText( QString("Min size: ") +
                          QString::number( double( _minSize ) ));

  _maxValueLabel->setText( QString("Max size: ") +
                          QString::number( double( _maxSize ) ));

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
  std::cout << "Selected "
            << _presets[ presetIdx ].name( ).toUtf8( ).constData( )
            << std::endl;
  const auto& stops = _presets[ presetIdx ].stops( );
//  _gradientFrame->setGradientStops( stops );
  QPolygonF redPoints;
  QPolygonF greenPoints;
  QPolygonF bluePoints;
  QPolygonF alphaPoints;
  for( const auto stop : stops )
  {
    redPoints.append( QPointF( stop.first, stop.second.redF( )));
    greenPoints.append( QPointF( stop.first, stop.second.greenF( )));
    bluePoints.append( QPointF( stop.first, stop.second.blueF( )));
    alphaPoints.append( QPointF( stop.first, stop.second.alphaF( )));

  }
  _redPoints->setPoints( redPoints, true );
  _greenPoints->setPoints( greenPoints, true );
  _bluePoints->setPoints( bluePoints, true );
  _alphaPoints->setPoints( alphaPoints, true );

  _result->setGradientStops( _nTGradientFrame->getGradientStops( ));
  _tResult = _gradientFrame->getGradientStops( );
  _sizeFrame->setGradientStops( _nTGradientFrame->getGradientStops( ));
}
