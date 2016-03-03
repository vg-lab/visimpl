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

  _result->setGradientStops( nTGradientFrame->getGradientStops( ));
  _tResult = gradientFrame->getGradientStops( );

  connect( this, SIGNAL( clicked( void )),
           this, SLOT( gradientClicked( void )));

  QGridLayout* mainLayout = new QGridLayout( );
  mainLayout->addWidget( _result );

  this->setLayout( mainLayout );

}

TransferFunctionWidget::~TransferFunctionWidget()
{
}

void TransferFunctionWidget::InitDialog( void )
{
  _dialog = new QWidget( );
  _dialog->setWindowModality( Qt::ApplicationModal );
  _dialog->setMinimumSize( 800, 600 );

  _acceptButton = new QPushButton( "Accept" );
  _cancelButton = new QPushButton( "Cancel" );
  _previewButton = new QPushButton( "Preview" );

  gradientFrame = new Gradient( );
  nTGradientFrame = new Gradient( );

  redGradientFrame = new Gradient( );
  greenGradientFrame = new Gradient( );
  blueGradientFrame = new Gradient( );
  alphaGradientFrame = new Gradient( );

  unsigned int row = 0;
  unsigned int totalColumns = 6;
  QGridLayout* dialogLayout = new QGridLayout( );

  dialogLayout->addWidget( new QLabel( "Red" ), row, 0, 1, 1 );
  dialogLayout->addWidget( redGradientFrame, row++, 1, 1, totalColumns );
  dialogLayout->addWidget( new QLabel( "Green" ), row, 0, 1, 1 );
  dialogLayout->addWidget( greenGradientFrame, row++, 1, 1, totalColumns );
  dialogLayout->addWidget( new QLabel( "Blue" ), row, 0, 1, 1 );
  dialogLayout->addWidget( blueGradientFrame, row++, 1, 1, totalColumns );
  dialogLayout->addWidget( new QLabel( "Alpha" ), row, 0, 1, 1 );
  dialogLayout->addWidget( alphaGradientFrame, row++, 1, 1, totalColumns );

  dialogLayout->addWidget( new QLabel( "Result (pure)" ), row, 0, 1, 1 );
  dialogLayout->addWidget( nTGradientFrame, row++, 1, 1, totalColumns );
  dialogLayout->addWidget( new QLabel( "Result (alpha)" ), row, 0, 1, 1 );
  dialogLayout->addWidget( gradientFrame, row++, 1, 1, totalColumns );

  dialogLayout->addWidget( _cancelButton, row, 1, 1, 1 );
  dialogLayout->addWidget( _previewButton, row, 3, 1, 1 );
  dialogLayout->addWidget( _acceptButton, row, 5, 1, 1 );

  _dialog->setLayout( dialogLayout );

  /* Setting gradient frames */
  QGradientStops stops;
  stops << qMakePair(0.0, QColor(0, 0, 0, 0))
        << qMakePair(1.0, QColor(255, 255, 255, 255));
  gradientFrame->setDirection(Gradient::HORIZONTAL);
  gradientFrame->setGradientStops(stops);

  QGradientStops ntStops;
  stops << qMakePair(0.0, QColor(0, 0, 0, 255))
        << qMakePair(1.0, QColor(255, 255, 255, 255));
  nTGradientFrame->setDirection(Gradient::HORIZONTAL);
  nTGradientFrame->setGradientStops(stops);

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

  /* Connecting slots */
  connect( _acceptButton, SIGNAL( clicked( void )),
           this, SLOT( acceptClicked( void )));
  connect( _cancelButton, SIGNAL( clicked( void )),
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

TTransferFunction TransferFunctionWidget::getColors( void )
{
  TTransferFunction result;

  for( auto stop : _tResult )
  {
    result.push_back( std::make_pair( float( stop.first ), stop.second ));
  }

  return result;
}

TTransferFunction TransferFunctionWidget::getPreviewColors( void )
{
  TTransferFunction result;

  QGradientStops stops = gradientFrame->getGradientStops( );

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

    QGradientStops stops = gradientFrame->getGradientStops();

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
    gradientFrame->setGradientStops(stops);
    nTGradientFrame->setGradientStops( ntStops );
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
    _result->setGradientStops( nTGradientFrame->getGradientStops( ));
    _tResult = gradientFrame->getGradientStops( );
  }
}

void TransferFunctionWidget::gradientClicked( void )
{
  std::cout << "Gradient clicked" << std::endl;
  if( _dialog && !_dialog->isVisible( ))
  {
    setColorPoints( getColors( ), false);

    _dialog->show( );
  }
}

void TransferFunctionWidget::buttonClicked( QAbstractButton* button )
{
  _dialog->close( );

  if( button == _acceptButton )
  {
    _result->setGradientStops( nTGradientFrame->getGradientStops( ));
    _tResult = gradientFrame->getGradientStops( );
    emit colorChanged( );
  }
  else if( button == _cancelButton )
  {
    if( previewed )
      emit colorChanged( );
  }
}

void TransferFunctionWidget::acceptClicked( void )
{
  _dialog->close( );

  _result->setGradientStops( nTGradientFrame->getGradientStops( ));
  _tResult = gradientFrame->getGradientStops( );
  emit colorChanged( );
}

void TransferFunctionWidget::cancelClicked( void )
{
  _dialog->close( );

  if( previewed )
    emit colorChanged( );
}

void TransferFunctionWidget::previewClicked( void )
{
  previewed = true;
  emit previewColor( );
}

void TransferFunctionWidget::mousePressEvent(QMouseEvent * event_ )
{
  if( event_->button( ) == Qt::LeftButton )
  {
    emit gradientClicked( );
  }
}
