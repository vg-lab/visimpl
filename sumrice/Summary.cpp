/*
 * SimulationSummaryWidget.cpp
 *
 *  Created on: 16 de feb. de 2016
 *      Author: sgalindo
 */

#include "Summary.h"

#include <QPainter>
#include <QBrush>



Summary::Summary( QWidget* parent_ )
: QFrame( parent_ )
, _bins( 250 )
, _spikeReport( nullptr )
, _voltageReport( nullptr )
, _mainHistogram( nullptr )
, _selectionHistogram( nullptr )
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
}

void Summary::CreateSummary( brion::SpikeReport* spikes_ )
{
//  _spikeReport = spikes_;
//
//  _filteredGIDs = gids;
//
//  CreateSummarySpikes( gids );
//  UpdateGradientColors( );

  _mainHistogram = new visimpl::Histogram( *spikes_ );
  _histograms.push_back( _mainHistogram );

  switch( _stackType )
  {
    case T_STACK_FIXED:
    {
//      _mainHistogram = new visimpl::Histogram( *spikes_ );
      _selectionHistogram = new visimpl::Histogram( *spikes_ );
      _histograms.push_back( _selectionHistogram );

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
      _selectionHistogram->colorMapper( colorMapper );

      break;
    }
    case T_STACK_EXPANDABLE:
    {

      break;
    }
    default:
      break;

  }
  //  AddGIDSelection( gids );

  CreateSummarySpikes( );
  UpdateGradientColors( );

}

void Summary::AddGIDSelection( const GIDUSet& gids )
{
  if( _stackType == TStackType::T_STACK_FIXED )
  {
    _selectionHistogram->filteredGIDs( gids );
    CreateSummarySpikes( );
    UpdateGradientColors( );


  }else if( _stackType == TStackType::T_STACK_EXPANDABLE )
  {



    setFixedHeight( (_histograms.size( ) + 1 ) * _heightPerRow );



  }

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
  gradient.setStops( _mainHistogram->gradientStops( ) );
  QBrush brush( gradient );

  QRect area = rect();

  QGradientStops stops;

  switch(_stackType )
  {
  case T_STACK_FIXED:
  {
    area.setHeight( area.height( ) / 2 );
    painter.fillRect( area, brush );

    assert( _selectionHistogram );

     stops = _selectionHistogram->gradientStops( );

    if( stops.size( ) == 0)
      stops = _mainHistogram->gradientStops( );

    QLinearGradient grad( 0, 0, width( ), 0 );
    grad.setStops( stops );
    QBrush b ( grad );
    area.setCoords( 0, height( ) / 2, width( ), height( ));
    painter.fillRect( area, b );

    break;
  }
  case T_STACK_EXPANDABLE:
  {
    area.setHeight( _heightPerRow );
    unsigned int counter = 0;
    unsigned int currentHeight = 0;

    for( auto histogram : _histograms )
    {
      area.setCoords( 0, currentHeight, width( ), currentHeight + _heightPerRow );

      stops = histogram->gradientStops( );

      currentHeight += _heightPerRow;
      counter++;

    }

    break;
  }
  }

}

void Summary::CreateSummarySpikes( )
{

  for( auto histogram : _histograms )
  {
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
    histogram->CalculateColors( );
  }

//  _mainHistogram->CalculateColors( );
//
//  _selectionHistogram->CalculateColors( );

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


/*****************************************************************************/
/******************************** HISTOGRAM **********************************/
/*****************************************************************************/


visimpl::Histogram::Histogram( const brion::Spikes& spikes,
                               float startTime,
                               float endTime )
: _maxValueHistogram( 0 )
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
: _maxValueHistogram( 0 )
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
  _maxValueHistogram = 0;

  float totalTime = _endTime - _startTime ;

  bool filter = _filteredGIDs.size( ) > 0;

  std::cout << "Filtered GIDs: " << _filteredGIDs.size( ) << std::endl;

  unsigned int counter = 0;
  float invTotal = 1.0f / totalTime;
  for( auto spike : _spikes )
  {
    if( filter && _filteredGIDs.find( spike.second ) == _filteredGIDs.end( ))
      continue;

    float perc = ( spike.first - _startTime ) * invTotal;
    unsigned int position =  _histogram.size( ) * perc;

    assert( position < _histogram.size( ));
    _histogram[ position ]++;
    counter++;
  }

  std::cout << "Total spikes: " << counter << std::endl;

  unsigned int cont = 0;
  unsigned int maxPos = 0;
  for( auto bin : _histogram )
  {
    if( bin > _maxValueHistogram )
    {
      _maxValueHistogram = bin;
      maxPos = cont;
    }
    cont++;
  }

  std::cout << "Bin with maximum value " << _maxValueHistogram << " at " << maxPos << std::endl;
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


  switch( _colorScale )
  {
    case visimpl::Histogram::T_COLOR_LINEAR:

      maxValue = 1.0f / _maxValueHistogram;

      break;
    case visimpl::Histogram::T_COLOR_EXPONENTIAL:

      maxValue = 1.0f / logf( _maxValueHistogram );

      break;
    case visimpl::Histogram::T_COLOR_LOGARITHMIC:

      maxValue = 1.0f / log10f( _maxValueHistogram );

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
