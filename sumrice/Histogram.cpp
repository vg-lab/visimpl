/*
 * Histogram.cpp
 *
 *  Created on: 11 de mar. de 2016
 *      Author: sgalindo
 */

#include "Histogram.h"

#include "log.h"

#include <QPainter>
#include <QBrush>

#include <QMouseEvent>

visimpl::Histogram::Histogram( )
: QFrame( nullptr )
, _maxValueHistogramLocal( 0 )
, _startTime( 0.0f )
, _endTime( 0.0f )
, _scaleFuncLocal( nullptr )
, _scaleFuncGlobal( nullptr )
, _colorScaleLocal( visimpl::T_COLOR_LINEAR )
, _colorScaleGlobal( visimpl::T_COLOR_LOGARITHMIC )
, _prevColorScaleLocal( visimpl::T_COLOR_UNDEFINED )
, _prevColorScaleGlobal( visimpl::T_COLOR_UNDEFINED )
, _normRule( visimpl::T_NORM_MAX )
, _repMode( visimpl::T_REP_DENSE )
, _lastMousePosition( nullptr )
{
  init( );
}


visimpl::Histogram::Histogram( const brion::Spikes& spikes,
                               float startTime,
                               float endTime )
: QFrame( nullptr )
, _maxValueHistogramLocal( 0 )
, _spikes( spikes )
, _startTime( startTime )
, _endTime( endTime )
, _scaleFuncLocal( nullptr )
, _scaleFuncGlobal( nullptr )
, _colorScaleLocal( visimpl::T_COLOR_LINEAR )
, _colorScaleGlobal( visimpl::T_COLOR_LOGARITHMIC )
, _prevColorScaleLocal( visimpl::T_COLOR_UNDEFINED )
, _prevColorScaleGlobal( visimpl::T_COLOR_UNDEFINED )
, _normRule( visimpl::T_NORM_MAX )
, _repMode( visimpl::T_REP_DENSE )
, _lastMousePosition( nullptr )
{
  init( );
}

visimpl::Histogram::Histogram( const brion::SpikeReport& spikeReport )
: QFrame( nullptr )
, _maxValueHistogramLocal( 0 )
, _spikes( spikeReport.getSpikes( ))
, _startTime( spikeReport.getStartTime( ))
, _endTime( spikeReport.getEndTime( ))
, _scaleFuncLocal( nullptr )
, _scaleFuncGlobal( nullptr )
, _colorScaleLocal( visimpl::T_COLOR_LINEAR )
, _colorScaleGlobal( visimpl::T_COLOR_LOGARITHMIC )
, _prevColorScaleLocal( visimpl::T_COLOR_UNDEFINED )
, _prevColorScaleGlobal( visimpl::T_COLOR_UNDEFINED )
, _normRule( visimpl::T_NORM_MAX )
, _repMode( visimpl::T_REP_DENSE )
, _lastMousePosition( nullptr )
{
  init( );
}

void visimpl::Histogram::Spikes( const brion::Spikes& spikes, float startTime, float endTime )
{
  _spikes = spikes;
  _startTime = startTime;
  _endTime = endTime;
}
void visimpl::Histogram::Spikes( const brion::SpikeReport& spikeReport )
{
  _spikes = spikeReport.getSpikes( );
  _startTime = spikeReport.getStartTime( );
  _endTime = spikeReport.getEndTime( );
}

void visimpl::Histogram::init( void )
{
  colorScaleLocal( _colorScaleLocal );
  colorScaleGlobal( _colorScaleGlobal );

//  QPixmap pixmap(20, 20);
//  QPainter painter(&pixmap);
//  painter.fillRect(0, 0, 10, 10, Qt::lightGray);
//  painter.fillRect(10, 10, 10, 10, Qt::lightGray);
//  painter.fillRect(0, 10, 10, 10, Qt::darkGray);
//  painter.fillRect(10, 0, 10, 10, Qt::darkGray);
//  painter.end();
//  QPalette pal = palette();
//  pal.setBrush(backgroundRole(), QBrush(pixmap));
//  setPalette(pal);
//  setAutoFillBackground(true);
  _paintRegion = false;
  setMouseTracking( true );
}

void visimpl::Histogram::CreateHistogram( unsigned int binsNumber )
{

//  _histogram.clear( );
  _histogram.resize( binsNumber, 0 );
  _maxValueHistogramLocal = 0;
  _maxValueHistogramGlobal = 0;

  std::vector< unsigned int > globalHistogram;
  globalHistogram.resize( binsNumber, 0 );

  float totalTime = _endTime - _startTime ;

  bool filter = _filteredGIDs.size( ) > 0;

  unsigned int counter = 0;

  float deltaTime = ( totalTime ) / _histogram.size( );
  float currentTime = _startTime + deltaTime;

  auto spike = _spikes.begin( );
  std::vector< unsigned int >::iterator globalIt = globalHistogram.begin( );
  for( unsigned int& bin : _histogram )
  {
    while( spike != _spikes.end( ) && spike->first <= currentTime )
    {
      ( *globalIt )++;
      if( !filter ||  _filteredGIDs.find( spike->second ) != _filteredGIDs.end( ))
        bin++;

      spike++;
      counter++;
    }

    currentTime += deltaTime;
    globalIt++;
  }

  std::cout << "Total spikes: " << counter << std::endl;

  unsigned int cont = 0;
  unsigned int maxPos = 0;
  for( auto bin : _histogram )
  {
    if( bin > _maxValueHistogramLocal )
    {
      _maxValueHistogramLocal = bin;
      maxPos = cont;
    }
    cont++;
  }

  _maxValueHistogramGlobal = _maxValueHistogramLocal;

  if( filter )
  {
    for( auto bin : globalHistogram )
    {
      if( bin > _maxValueHistogramGlobal )
      {
        _maxValueHistogramGlobal = bin;
      }
    }
  }

  std::cout << "Bin with local maximum value " << _maxValueHistogramLocal
            << " at " << maxPos
            << std::endl;

  CalculateColors( );

}

namespace visimpl
{
  float base = 1.0001f;

  // All these functions consider a maxValue = 1.0f / <calculated_maxValue >
  float linearFunc( float value, float invMaxValue )
  {
    return value * invMaxValue;
  }

  float exponentialFunc( float value, float maxValue )
  {
    return ( powf( base, value - maxValue ));
  }

  float logarithmicFunc( float value, float invMaxValue )
  {
    return ( log10f( value) * invMaxValue );
  }

  float maxValueFunc( float maxValue,  TColorScale colorScale )
  {
    float result = 0.0f;
    switch( colorScale )
    {
      case visimpl::T_COLOR_LINEAR:

        result = 1.0f / maxValue;

        break;
//      case visimpl::T_COLOR_EXPONENTIAL:
//
//        result = maxValue;
//
//        break;
      case visimpl::T_COLOR_LOGARITHMIC:

        result = 1.0f / log10f( maxValue );

        break;
      default:
        VISIMPL_THROW( "Selected color scale function is not available.")
        break;
    }

    return result;
  }

}

void visimpl::Histogram::CalculateColors( void )
{
  if( _repMode == T_REP_DENSE )
  {

    QGradientStops stops;

    float maxValue = 0.0f;
    float relativeTime = 0.0f;
    float delta = 1.0f / _histogram.size( );
    float percentage;

    maxValue = _normRule == T_NORM_GLOBAL ?
               _maxValueHistogramGlobal :
               _maxValueHistogramLocal;

    maxValue = maxValueFunc( maxValue, _normRule == T_NORM_GLOBAL ?
                                        _colorScaleGlobal :
                                        _colorScaleLocal );

    for( auto bin : _histogram )
    {
      percentage = _scaleFuncLocal( float( bin ), maxValue );
      percentage = std::max< float >( std::min< float > (1.0f, percentage ), 0.0f);
  //    std::cout << percentage << std::endl;
      glm::vec4 color = _colorMapper.GetValue( percentage );
      stops << qMakePair( relativeTime, QColor( color.r, color.g, color.b, color.a ));

      relativeTime += delta;
    }

    _gradientStops = stops;
  }
  else if( _repMode == T_REP_CURVE )
  {
//    bool updateGlobal = _prevColorScaleGlobal != _colorScaleGlobal;
//    bool updateLocal = _prevColorScaleLocal != _colorScaleLocal;
//
//    if( !updateGlobal && !updateLocal )
//      return;

//    float maxValue;
    float invMaxValueLocal;
    float invMaxValueGlobal;

    QPolygonF auxLocal;
    QPolygonF auxGlobal;

    invMaxValueLocal = maxValueFunc( _maxValueHistogramLocal, _colorScaleLocal );
    invMaxValueGlobal = maxValueFunc( _maxValueHistogramGlobal, _colorScaleGlobal );

    float currentX;
    float globalY;
    float localY;
    unsigned int counter = 0;
    float invBins = 1.0f / float( _histogram.size( ) - 1);

    for( auto bin : _histogram )
    {
      currentX = counter * invBins;

      globalY = 0.0f;
      localY = 0.0f;

      if( bin > 0)
      {
//        if( updateGlobal )
          globalY = _scaleFuncGlobal( float( bin ), invMaxValueGlobal );

//        if( updateLocal )
          localY = _scaleFuncLocal( float( bin ), invMaxValueLocal );
      }

//      if( updateGlobal )
        auxGlobal.push_back( QPointF( currentX, 1.0f - globalY ));

//      if( updateLocal )
        auxLocal.push_back( QPointF( currentX, 1.0f - localY ));

      counter++;
    }

//    if( updateGlobal )
      _curveStopsGlobal = auxGlobal;

//    if( updateLocal )
    _curveStopsLocal = auxLocal;


  }
}

void visimpl::Histogram::filteredGIDs( const GIDUSet& gids )
{
  if( gids.size( ) > 0 )
    _filteredGIDs = gids;
}

const GIDUSet& visimpl::Histogram::filteredGIDs( void )
{
  return _filteredGIDs;
}

void visimpl::Histogram::colorScaleLocal( visimpl::TColorScale scale )
{

  _prevColorScaleLocal = _colorScaleLocal;
  _colorScaleLocal = scale;

  switch( _colorScaleLocal )
  {
    case visimpl::T_COLOR_LINEAR:

      _scaleFuncLocal = linearFunc;

      break;
//    case visimpl::T_COLOR_EXPONENTIAL:
//
//      _scaleFuncLocal = exponentialFunc;
//
//      break;
    case visimpl::T_COLOR_LOGARITHMIC:

      _scaleFuncLocal = logarithmicFunc;

      break;
    default:
      break;
  }
}

void visimpl::Histogram::colorScaleGlobal( visimpl::TColorScale scale )
{

  _prevColorScaleGlobal = _colorScaleGlobal;
  _colorScaleGlobal = scale;

  switch( _colorScaleGlobal )
  {
    case visimpl::T_COLOR_LINEAR:

      _scaleFuncGlobal = linearFunc;

      break;
//    case visimpl::T_COLOR_EXPONENTIAL:
//
//      _scaleFuncGlobal = exponentialFunc;
//
//      break;
    case visimpl::T_COLOR_LOGARITHMIC:

      _scaleFuncGlobal = logarithmicFunc;

      break;
    default:
      break;
  }
}

visimpl::TColorScale visimpl::Histogram::colorScaleLocal( void )
{
  return _colorScaleLocal;
}

void visimpl::Histogram::normalizeRule(
    visimpl::TNormalize_Rule normRule )
{
  _normRule = normRule;
}

visimpl::TNormalize_Rule visimpl::Histogram::normalizeRule( void )
{
  return _normRule;
}

void visimpl::Histogram::representationMode(
    visimpl::TRepresentation_Mode repMode )
{
  _repMode = repMode;
}

visimpl::TRepresentation_Mode
visimpl::Histogram::representationMode( void )
{
  return _repMode;
}

const std::vector< unsigned int>& visimpl::Histogram::histogram( void ) const
{
  return _histogram;
}

unsigned int visimpl::Histogram::maxLocal( void )
{
  return _maxValueHistogramLocal;
}

unsigned int visimpl::Histogram::maxGlobal( void )
{
  return _maxValueHistogramGlobal;
}

const utils::InterpolationSet< glm::vec4 >&
visimpl::Histogram::colorMapper( void )
{
  return _colorMapper;
}

void visimpl::Histogram::colorMapper(
    const utils::InterpolationSet< glm::vec4 >& colors )
{
  _colorMapper = colors;
}

QColor visimpl::Histogram::colorLocal( void )
{
  return _colorLocal;
}

void visimpl::Histogram::colorLocal( const QColor& color )
{
  _colorLocal = color;
}

QColor visimpl::Histogram::colorGlobal( void )
{
  return _colorGlobal;
}

void visimpl::Histogram::colorGlobal( const QColor& color )
{
  _colorGlobal = color;
}

const QGradientStops& visimpl::Histogram::gradientStops( void )
{
  return _gradientStops;
}

unsigned int visimpl::Histogram::valueAt( float percentage )
{
  unsigned int position = percentage * _histogram.size( );

  if( position >= _histogram.size( ))
    position = _histogram.size( ) - 1;

  return _histogram[ position ];
}

bool visimpl::Histogram::isInitialized( void )
{
  return _gradientStops.size( ) > 0 || _curveStopsGlobal.size( ) > 0;
}

QPolygonF visimpl::Histogram::localFunction( void ) const
{
  return _curveStopsLocal;
}

QPolygonF visimpl::Histogram::globalFunction( void ) const
{
  return _curveStopsGlobal;
}

void visimpl::Histogram::mousePressEvent( QMouseEvent* event_ )
{
  QFrame::mousePressEvent( event_ );

  QPoint position = event_->pos( );

  float percentage = position.x( ) / float( width( ));

  emit mouseClicked( percentage );
}

void visimpl::Histogram::mouseMoveEvent( QMouseEvent* event_ )
{
  QFrame::mouseMoveEvent( event_ );

  QPoint position = event_->pos( );

//  position.setX( position.x( ) + x( ));
//  position.setY( position.y( ) + y( ));
//  QRect rect = geometry( );
  position = mapToGlobal( position );

  emit mousePositionChanged( position );
}

void visimpl::Histogram::mousePosition( QPoint* mousePosition_ )
{
  _lastMousePosition = mousePosition_;
}

void visimpl::Histogram::regionWidth( float region_ )
{
  _regionWidth = region_;
}

float visimpl::Histogram::regionWidth( void )
{
  return _regionWidth;
}

void visimpl::Histogram::paintRegion( bool region )
{
  _paintRegion = region;
}

void visimpl::Histogram::paintEvent(QPaintEvent* /*e*/)
{
  QPainter painter( this );
  unsigned int currentHeight = height( );

  QColor penColor;

  if( _repMode == T_REP_DENSE )
  {
  //  std::cout << "Painting... " << this << std::endl;
    QLinearGradient gradient( 0, 0, width( ), 0 );

    QRect area = rect();

    gradient.setStops( _gradientStops );
    QBrush brush( gradient );

    painter.fillRect( area, brush );

    QLine line( QPoint( 0, currentHeight), QPoint( width( ), currentHeight ));
    painter.drawLine( line );

    penColor = QColor( 255, 255, 255 );

  }
  else if( _repMode == T_REP_CURVE )
  {

    painter.setRenderHint( QPainter::Antialiasing );

    painter.fillRect( rect( ), QBrush( QColor( 255, 255, 255, 255 ),
                                       Qt::SolidPattern ));

    QPainterPath path;
    path.moveTo( 0, height( ) );
    for( auto point : _curveStopsLocal )
    {
      path.lineTo( QPoint( point.x( ) * width( ), point.y( ) * height( )));
    }
    path.lineTo( width( ), height( ) );

    QPainterPath globalPath;
    globalPath.moveTo( 0, height( ) );
    for( auto point : _curveStopsGlobal )
    {
      globalPath.lineTo( QPoint( point.x( ) * width( ), point.y( ) * height( )));
    }
    globalPath.lineTo( width( ), height( ) );

    painter.setBrush( QBrush( QColor( 255, 0, 0, 100 ), Qt::SolidPattern));
    painter.setPen( Qt::NoPen );
    painter.drawPath( globalPath );
    painter.setBrush( QBrush( QColor( 0, 0, 128, 50 ), Qt::SolidPattern));
    painter.drawPath( path );

    penColor = QColor( 0, 0, 0 );
  }


  if( _lastMousePosition )
  {
    QPoint localPosition = mapFromGlobal( *_lastMousePosition );

    if( localPosition.x( ) > width( ))
      localPosition.setX( width( ));
    else if ( localPosition.x( ) < 0 )
      localPosition.setX( 0 );

    if( localPosition.y( ) > height( ))
      localPosition.setY( height( ));
    else if ( localPosition.y( ) < 0 )
      localPosition.setY( 0 );


    float percentage = float( localPosition.x( )) / float( width( ));
//    percentage = std::min( 1.0f, std::max( 0.0f, percentage));
    int positionX = localPosition.x( );
    int margin = 5;

    if( width( ) - positionX < 50 )
      margin = -50;

    if( _paintRegion )
    {
      int regionW = _regionWidth * width( );
      int start = std::max( 0, positionX - regionW );
      if( ( positionX + regionW ) > width( ) )
        start = width( ) - regionW * 2;

      QRect region( std::max( 0, start ),
                    0,
                    regionW * 2,
                    height( ));

      QPen pen( Qt::NoPen );
      QBrush brush( QColor( 30, 30, 30, 30 ));
      painter.drawRect( region );
    }

    QPen pen( penColor );
    painter.setPen( pen );

    currentHeight = currentHeight / 2;
    QPoint position ( positionX + margin, currentHeight );

    painter.drawText( position, QString::number( valueAt( percentage )));

    QLine marker( QPoint( positionX, 0 ), QPoint( positionX, height( )));
    pen.setColor( QColor( 177, 50, 50 ));
    painter.setPen( pen );
    painter.drawLine( marker );
  }

}


