/*
 * SimulationSummaryWidget.cpp
 *
 *  Created on: 16 de feb. de 2016
 *      Author: sgalindo
 */

#include "SimulationSummaryWidget.h"

#include <QPainter>
#include <QBrush>

SimulationSummaryWidget::SimulationSummaryWidget( QWidget* parent_ )
: QFrame( parent_ )
, _maxValueHistogram( 0 )
, _histogramDelta( 0.0f )
, _spikeReport( nullptr )
, _voltageReport( nullptr )
{
  _colorMapper.Insert(0.0f, glm::vec4( 0.0f, 0.0f, 255, 255 ));
//  _colorMapper.Insert(0.0f, glm::vec4( 128, 128, 255, 255 ));
  _colorMapper.Insert(0.0f, glm::vec4( 0.0f, 255, 0.0f, 255 ));
  _colorMapper.Insert(0.0f, glm::vec4( 255, 255, 0.0f, 255 ));
  _colorMapper.Insert(0.0f, glm::vec4( 255, 0.0f, 0.0f, 255 ));
}

SimulationSummaryWidget::SimulationSummaryWidget( QWidget* parent_,
                                                  unsigned int bins_ )
: QFrame( parent_ )
, _maxValueHistogram( 0 )
, _histogramDelta( 0.0f )
, _spikeReport( nullptr )
, _voltageReport( nullptr )
{
  _histogram.resize( bins_, 0 );

//  _colorMapper.Insert(0.0f, glm::vec4( 0.0f, 0.0f, 255, 255 ));
//  _colorMapper.Insert(0.25f, glm::vec4( 128, 128, 255, 255 ));
//  _colorMapper.Insert(0.5f, glm::vec4( 0.0f, 255, 0.0f, 255 ));
//  _colorMapper.Insert(0.75f, glm::vec4( 255, 255, 0.0f, 255 ));
//  _colorMapper.Insert(1.0f, glm::vec4( 255, 0.0f, 0.0f, 255 ));

  _colorMapper.Insert(0.0f, glm::vec4( 0.0f, 0.0f, 255, 255 ));
//  _colorMapper.Insert(0.25f, glm::vec4( 128, 128, 255, 255 ));
  _colorMapper.Insert(0.33f, glm::vec4( 0.0f, 255, 0.0f, 255 ));
  _colorMapper.Insert(0.66f, glm::vec4( 255, 255, 0.0f, 255 ));
  _colorMapper.Insert(1.0f, glm::vec4( 255, 0.0f, 0.0f, 255 ));

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

void SimulationSummaryWidget::CreateSummary( brion::SpikeReport* spikes_ )
{
  _spikeReport = spikes_;

  CreateSummarySpikes( );
  UpdateGradientColors( );

}

//void SimulationSummaryWidget::CreateSummary( brion::CompartmentReport* _voltages )
//{
//
//}

void SimulationSummaryWidget::paintEvent(QPaintEvent* /*e*/)
{
  //TODO
//  std::cout << "Painting..." << std::endl;
  QPainter painter(this);
  QLinearGradient gradient(0, 0, width( ),  0 );
  gradient.setStops( _gradientStops );
  QBrush brush( gradient );
  painter.fillRect(rect(), brush);
}

void SimulationSummaryWidget::CreateSummarySpikes( void )
{
  brion::Spikes spikes = _spikeReport->getSpikes( );

  float startTime = _spikeReport->getStartTime( );
  float endTime = _spikeReport->getEndTime( );
  float totalTime = endTime - startTime ;

  _histogramDelta = totalTime / float( _histogram.size( ));

  std::cout << "Total Time: " << totalTime << " Histogram delta: " << _histogramDelta << std::endl;
//  float currentTime = 0.0f;
//  unsigned int currentBin = 0;
//    brion::Spikes::const_iterator spike = spikes.begin( );

  float invTotal = 1.0f / totalTime;
  for( auto spike : spikes )
  {
    float perc = ( spike.first - startTime ) * invTotal;
    unsigned int position =  _histogram.size( ) * perc;
//    std::cout << "Position: " << position << std::endl;
    assert( position < _histogram.size( ));
    _histogram[ position ]++;
  }

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

void SimulationSummaryWidget::CreateSummaryVoltages( void )
{

}

void SimulationSummaryWidget::UpdateGradientColors( void )
{
  QGradientStops stops;
//  float maxValue = _spikeReport->getSpikes( ).size( );
  float maxValue = _maxValueHistogram;
  std::cout << "Total spikes: " << int(maxValue) << std::endl;
  float relativeTime = 0.0f;
  float delta = 1.0f / _histogram.size( );
  float percentage;
  for( auto bin : _histogram )
  {
    percentage = float( bin ) / maxValue;
//    std::cout << percentage << std::endl;
    glm::vec4 color = _colorMapper.GetValue( percentage );
    stops << qMakePair( relativeTime, QColor( color.r, color.g, color.b, color.a ));

    relativeTime += delta;
  }

  _gradientStops = stops;
}


void SimulationSummaryWidget::bins( unsigned int bins_ )
{
  _histogram.resize( bins_ );

  if( _spikeReport )
    CreateSummarySpikes( );
  else if( _voltageReport )
    CreateSummaryVoltages( );
  else
    return;

  UpdateGradientColors( );
}

unsigned int SimulationSummaryWidget::bins( void )
{
  return _histogram.size( );
}


void SimulationSummaryWidget::colorMapper( const utils::InterpolationSet< glm::vec4 >& colors )
{
  _colorMapper = colors;
}

const utils::InterpolationSet< glm::vec4 >& SimulationSummaryWidget::colorMapper( void )
{
  return _colorMapper;
}
