/*
 * SimulationSummaryWidget.h
 *
 *  Created on: 16 de feb. de 2016
 *      Author: sgalindo
 */

#ifndef SRC_SIMULATIONSUMMARYWIDGET_H_
#define SRC_SIMULATIONSUMMARYWIDGET_H_

#include <brion/brion.h>
#include <prefr/prefr.h>

#include <QFrame>

class SimulationSummaryWidget : public QFrame
{

  Q_OBJECT;

public:

  SimulationSummaryWidget( QWidget* parent = 0 );
  SimulationSummaryWidget( QWidget* parent = 0, unsigned int bins = 1000);

  void CreateSummary( brion::SpikeReport* _spikes );
//  void CreateSummary( brion::CompartmentReport* _voltages );

  virtual void paintEvent(QPaintEvent* event);

  void bins( unsigned int bins_ );
  unsigned int bins( void );

  void colorMapper( const utils::InterpolationSet< glm::vec4 >& colors );
  const utils::InterpolationSet< glm::vec4 >& colorMapper( void );

protected:

  void CreateSummarySpikes( void );
  void CreateSummaryVoltages( void );
  void UpdateGradientColors( void );

  std::vector< unsigned int > _histogram;
  unsigned int _maxValueHistogram;
  QGradientStops _gradientStops;

  float _histogramDelta;

  brion::SpikeReport* _spikeReport;
  brion::CompartmentReport* _voltageReport;

  utils::InterpolationSet< glm::vec4 > _colorMapper;

};





#endif /* SRC_SIMULATIONSUMMARYWIDGET_H_ */
