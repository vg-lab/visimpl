/*
 * SimulationSummaryWidget.cpp
 *
 *  Created on: 16 de feb. de 2016
 *      Author: sgalindo
 */

#include "Summary.h"

#include <QPainter>
#include <QBrush>
#include <QMouseEvent>



Summary::Summary( QWidget* parent_ )
: QFrame( parent_ )
, _bins( 250 )
, _spikeReport( nullptr )
, _voltageReport( nullptr )
, _mainHistogram( nullptr )
, _selectionHistogram( nullptr )
, _stackType( T_STACK_FIXED )
, _heightPerRow( 50 )
{ }

Summary::Summary( QWidget* parent_,
                  TStackType stackType )
: QFrame( parent_ )
, _bins( 250 )
, _spikeReport( nullptr )
, _voltageReport( nullptr )
, _mainHistogram( nullptr )
, _selectionHistogram( nullptr )
, _stackType( stackType )
, _heightPerRow( 50 )
{

  QPixmap pixmap(20, 20);
  QPainter painter(&pixmap);
  painter.fillRect(0, 0, 10, 10, Qt::lightGray);
  painter.fillRect(10, 10, 10, 10, Qt::lightGray);
  painter.fillRect(0, 10, 10, 10, Qt::darkGray);
  painter.fillRect(10, 0, 10, 10, Qt::darkGray);
  painter.end();
  QPalette pal = palette();
  pal.setBrush(backgroundRole(), QBrush(pixmap));
  setPalette(pal);
  setAutoFillBackground(true);

  setMouseTracking( true );
}

void Summary::CreateSummary( brion::SpikeReport* spikes_, brion::GIDSet gids )
{
  _spikeReport = spikes_;
  _gids = GIDUSet( gids.begin( ), gids.end( ));
//
//  _filteredGIDs = gids;
//
//  CreateSummarySpikes( gids );
//  UpdateGradientColors( );

  _mainHistogram = new visimpl::Histogram( *spikes_ );
  _histograms.push_back( _mainHistogram );

  TColorMapper colorMapper;
//  colorMapper.Insert(0.0f, glm::vec4( 0.0f, 0.0f, 255, 255 ));
//  colorMapper.Insert(0.25f, glm::vec4( 128, 128, 255, 255 ));
//  colorMapper.Insert(0.33f, glm::vec4( 0.0f, 255, 0.0f, 255 ));
//  colorMapper.Insert(0.66f, glm::vec4( 255, 255, 0.0f, 255 ));
//  colorMapper.Insert(1.0f, glm::vec4( 255, 0.0f, 0.0f, 255 ));

//  colorMapper.Insert(0.0f, glm::vec4( 157, 206, 111, 255 ));
//  colorMapper.Insert(0.25f, glm::vec4( 125, 195, 90, 255 ));
//  colorMapper.Insert(0.50f, glm::vec4( 109, 178, 113, 255 ));
//  colorMapper.Insert(0.75f, glm::vec4( 76, 165, 86, 255 ));
//  colorMapper.Insert(1.0f, glm::vec4( 63, 135, 61, 255 ));

  colorMapper.Insert(0.0f, glm::vec4( 255, 170, 170, 255 ));
  colorMapper.Insert(0.25f, glm::vec4( 212, 106, 106, 255 ));
  colorMapper.Insert(0.50f, glm::vec4( 170, 57, 57, 255 ));
  colorMapper.Insert(0.75f, glm::vec4( 128, 21, 21, 255 ));
  colorMapper.Insert(1.0f, glm::vec4( 85, 0, 0, 255 ));

  _mainHistogram->colorMapper( colorMapper );

  switch( _stackType )
  {
    case T_STACK_FIXED:
    {
//      _mainHistogram = new visimpl::Histogram( *spikes_ );
      _selectionHistogram = new visimpl::Histogram( *spikes_ );
      _histograms.push_back( _selectionHistogram );

      _selectionHistogram->colorMapper( colorMapper );

      break;
    }
    case T_STACK_EXPANDABLE:
    {
      this->setMinimumHeight( _heightPerRow );
      break;
    }
    default:
      break;

  }
  //  AddGIDSelection( gids );

  CreateSummarySpikes( );
  UpdateGradientColors( );
  update( );
}

void Summary::AddGIDSelection( const GIDUSet& gids )
{
  if( gids.size( ) == _gids.size( ) || gids.size( ) == 0)
    return;

  std::cout << "Adding new selection with size " << gids.size( ) << std::endl;
  if( _stackType == TStackType::T_STACK_FIXED )
  {
    _selectionHistogram->filteredGIDs( gids );

  }else if( _stackType == TStackType::T_STACK_EXPANDABLE )
  {

    visimpl::Histogram* histogram = new visimpl::Histogram( *_spikeReport );
    histogram->filteredGIDs( gids );
    histogram->colorMapper( _mainHistogram->colorMapper( ));
    histogram->colorScale( visimpl::Histogram::T_COLOR_EXPONENTIAL );
    histogram->normalizeRule( visimpl::Histogram::T_NORM_GLOBAL );
    _histograms.push_back( histogram );


    unsigned int newHeight = (_histograms.size( )) * _heightPerRow;
    setMinimumHeight( newHeight );
    std::cout << newHeight << std::endl;
//    resize( width( ), newHeight );

    this->parentWidget( )->resize( width( ), newHeight + 2 );

  }

  CreateSummarySpikes( );
  UpdateGradientColors( );

  update( );
}

//void SimulationSummaryWidget::CreateSummary( brion::CompartmentReport* _voltages )
//{
//
//}

void Summary::paintEvent(QPaintEvent* /*e*/)
{

  QPainter painter( this );
  QLinearGradient gradient( 0, 0, width( ), 0 );

  QRect area = rect();

  QGradientStops stops;

  if( _mainHistogram)
  {
    switch(_stackType )
    {
    case T_STACK_FIXED:
    {
      gradient.setStops( _mainHistogram->gradientStops( ) );
      QBrush brush( gradient );

      area.setHeight( area.height( ) / 2 );
      painter.fillRect( area, brush );

      assert( _selectionHistogram );

      stops = _selectionHistogram->gradientStops( );

      if( stops.size( ) == 0)
        stops = _mainHistogram->gradientStops( );

      gradient.setStops( stops );
      brush = QBrush ( gradient );
      area.setCoords( 0, height( ) / 2, width( ), height( ));
      painter.fillRect( area, brush );

      break;
    }
    case T_STACK_EXPANDABLE:
    {
      area.setHeight( _heightPerRow );
      unsigned int counter = 0;
      unsigned int currentHeight = 0;

      for( auto histogram : _histograms )
      {
  //      std::cout << "Histogram stops: " <<  histogram->gradientStops( ).size( ) << std::endl;
        gradient.setStops( histogram->gradientStops( ));
        QBrush brush( gradient );

        area.setCoords( 0, currentHeight, width( ), currentHeight + _heightPerRow );

        painter.fillRect( area, brush );

        currentHeight += _heightPerRow;
        counter++;
      }

      break;
    }
    }

    unsigned int currentHeight;
    for( unsigned int i = 1; i < _histograms.size( ); i++ )
    {
      currentHeight = i * _heightPerRow;

      QLine line( QPoint( 0, currentHeight), QPoint( width( ), currentHeight ));
      painter.drawLine( line );
    }


    if( _showMarker )
    {
      float percentage = float( _lastMousePosition.x( )) / float( width( ));
      int positionX = _lastMousePosition.x( );
      int margin = 5;

      if( width( ) - positionX < 50 )
        margin = -50;

      QPen pen( QColor( 255, 255, 255 ));
      painter.setPen( pen );

      currentHeight = _heightPerRow / 2;
      QPoint position ( positionX + margin, currentHeight );
      for( auto histogram : _histograms )
      {
        painter.drawText( position,
                          QString::number( histogram->valueAt( percentage )));

        position.setY( position.y( ) + _heightPerRow );
      }

      QLine marker( QPoint( positionX, 0 ), QPoint( positionX, height( )));
      pen.setColor( QColor( 177, 50, 50 ));
      painter.setPen( pen );
      painter.drawLine( marker );
    }
  }
}

void Summary::mouseMoveEvent( QMouseEvent* event_ )
{
  QFrame::mouseMoveEvent( event_ );

  QPoint position = event_->pos( );
  QRect bounds = rect( );

  if( position != _lastMousePosition &&
      bounds.contains( position ))
  {
    _lastMousePosition = position;
    _showMarker = true;
    update( );
  }
  else
  {
    if( _showMarker )
    {
      _showMarker = false;
      update( );
    }
    _showMarker = false;
  }

}

void Summary::CreateSummarySpikes( )
{
  std::cout << "Creating histograms... Number of bins: " << _bins << std::endl;
  for( auto histogram : _histograms )
  {
    if( histogram->gradientStops( ).size( ) == 0)
      histogram->CreateHistogram( _bins );
  }

//  _mainHistogram->CreateHistogram( _bins );
//
//  _selectionHistogram->CreateHistogram( _bins );


}

//void Summary::CreateSummaryVoltages( void )
//{
//
//}

void Summary::UpdateGradientColors( void )
{
  for( auto histogram : _histograms )
  {
    if( histogram->gradientStops( ).size( ) == 0)
      histogram->CalculateColors( );
  }

//  _mainHistogram->CalculateColors( );
//
//  _selectionHistogram->CalculateColors( );

}

unsigned int Summary::histogramsNumber( void )
{
  return _histograms.size( );
}


void Summary::bins( unsigned int bins_ )
{
  _bins = bins_;

  CreateSummarySpikes( );
  UpdateGradientColors( );
}

unsigned int Summary::bins( void )
{
  return _bins;
}

void Summary::heightPerRow( unsigned int height_ )
{
  _heightPerRow = height_;
}

unsigned int Summary::heightPerRow( void )
{
  return _heightPerRow;
}

void Summary::showMarker( bool show_ )
{
  _showMarker = show_;
}

/*****************************************************************************/
/******************************** HISTOGRAM **********************************/
/*****************************************************************************/


visimpl::Histogram::Histogram( const brion::Spikes& spikes,
                               float startTime,
                               float endTime )
: _maxValueHistogramLocal( 0 )
, _spikes( spikes )
, _startTime( startTime )
, _endTime( endTime )
, _scaleFunc( nullptr )
, _colorScale( visimpl::Histogram::T_COLOR_LOGARITHMIC )
, _normRule( visimpl::Histogram::T_NORM_MAX )
{
  colorScale( _colorScale );
}

visimpl::Histogram::Histogram( const brion::SpikeReport& spikeReport )
: _maxValueHistogramLocal( 0 )
, _spikes( spikeReport.getSpikes( ))
, _startTime( spikeReport.getStartTime( ))
, _endTime( spikeReport.getEndTime( ))
, _scaleFunc( nullptr )
, _colorScale( visimpl::Histogram::T_COLOR_LOGARITHMIC )
, _normRule( visimpl::Histogram::T_NORM_MAX )
{
  colorScale( _colorScale );

}

void visimpl::Histogram::CreateHistogram( unsigned int binsNumber )
{

  _histogram.clear( );
  _histogram.resize( binsNumber, 0 );
  _maxValueHistogramLocal = 0;
  _maxValueHistogramGlobal = 0;

  std::vector< unsigned int > globalHistogram;
  globalHistogram.resize( binsNumber, 0 );

  float totalTime = _endTime - _startTime ;

  bool filter = _filteredGIDs.size( ) > 0;

  std::cout << "Filtered GIDs: " << _filteredGIDs.size( ) << std::endl;

  unsigned int counter = 0;
  float invTotal = 1.0f / totalTime;
  for( auto spike : _spikes )
  {
    float perc = ( spike.first - _startTime ) * invTotal;
    unsigned int position =  _histogram.size( ) * perc;

    globalHistogram[ position ]++;

    if( filter && _filteredGIDs.find( spike.second ) == _filteredGIDs.end( ))
    {
      continue;
    }

    assert( position < _histogram.size( ));
    _histogram[ position ]++;
    counter++;
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
}

namespace visimpl
{

  // All these functions consider a maxValue = 1.0f / <calculated_maxValue >
  float linearFunc( float value, float maxValue )
  {
    return value * maxValue;
  }

  float exponentialFunc( float value, float maxValue )
  {
    return ( logf( value) * maxValue );
  }

  float logarithmicFunc( float value, float maxValue )
  {
    return ( log10f( value) * maxValue );
  }

}

void visimpl::Histogram::CalculateColors( void )
{
  QGradientStops stops;

  float maxValue = 0.0f;
  float relativeTime = 0.0f;
  float delta = 1.0f / _histogram.size( );
  float percentage;

  maxValue = _normRule == T_NORM_GLOBAL ?
             _maxValueHistogramGlobal :
             _maxValueHistogramLocal;

  switch( _colorScale )
  {
    case visimpl::Histogram::T_COLOR_LINEAR:

      maxValue = 1.0f / maxValue;

      break;
    case visimpl::Histogram::T_COLOR_EXPONENTIAL:

      maxValue = 1.0f / logf( maxValue );

      break;
    case visimpl::Histogram::T_COLOR_LOGARITHMIC:

      maxValue = 1.0f / log10f( maxValue );

      break;
  }

  for( auto bin : _histogram )
  {
    percentage = _scaleFunc( float( bin ), maxValue );
    percentage = std::max< float >( std::min< float > (1.0f, percentage ), 0.0f);
//    std::cout << percentage << std::endl;
    glm::vec4 color = _colorMapper.GetValue( percentage );
    stops << qMakePair( relativeTime, QColor( color.r, color.g, color.b, color.a ));

    relativeTime += delta;
  }

  _gradientStops = stops;
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

void visimpl::Histogram::colorScale( visimpl::Histogram::TColorScale scale )
{
  _colorScale = scale;

  switch( _colorScale )
  {
    case visimpl::Histogram::T_COLOR_LINEAR:

      _scaleFunc = linearFunc;

      break;
    case visimpl::Histogram::T_COLOR_EXPONENTIAL:

      _scaleFunc = exponentialFunc;

      break;
    case visimpl::Histogram::T_COLOR_LOGARITHMIC:

      _scaleFunc = logarithmicFunc;

      break;
  }
}

visimpl::Histogram::TColorScale visimpl::Histogram::colorScale( void )
{
  return _colorScale;
}

void visimpl::Histogram::normalizeRule(
    visimpl::Histogram::TNormalize_Rule normRule )
{
  _normRule = normRule;
}

visimpl::Histogram::TNormalize_Rule visimpl::Histogram::normalizeRule( void )
{
  return _normRule;
}

const std::vector< unsigned int>& visimpl::Histogram::histogram( void )
{
  return _histogram;
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
