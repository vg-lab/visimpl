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
#include <unordered_set>

typedef std::unordered_set< uint32_t > GIDUSet;
typedef utils::InterpolationSet< glm::vec4 > TColorMapper;

namespace visimpl
{
  class Histogram;
}
class SimulationSummaryWidget : public QFrame
{

  Q_OBJECT;

public:

  SimulationSummaryWidget( QWidget* parent = 0 );
  SimulationSummaryWidget( QWidget* parent = 0, unsigned int bins = 1000);

  void CreateSummary( brion::SpikeReport* _spikes, const GIDUSet& gids );
  void SetSelectionGIDs( const GIDUSet& gids );
//  void CreateSummary( brion::CompartmentReport* _voltages );

  virtual void paintEvent(QPaintEvent* event);

  void bins( unsigned int bins_ );
  unsigned int bins( void );

//  void colorMapper( const utils::InterpolationSet< glm::vec4 >& colors );
//  const utils::InterpolationSet< glm::vec4 >& colorMapper( void );
//
//  void filterGIDs( const GIDUSet& gids );
//  const GIDUSet& filterGIDs( void );

protected:

  void CreateSummarySpikes( );
  void CreateSummaryVoltages( void );
  void UpdateGradientColors( void );

  unsigned int _bins;

//  std::vector< unsigned int > _histogram;
//  unsigned int _maxValueHistogram;
//  QGradientStops _gradientStops;
//
  brion::SpikeReport* _spikeReport;
  brion::CompartmentReport* _voltageReport;
//
//  utils::InterpolationSet< glm::vec4 > _colorMapper;
//
//  GIDUSet _filteredGIDs;

  visimpl::Histogram* _mainHistogram;
  visimpl::Histogram* _selectionHistogram;

};

namespace visimpl
{
  class Histogram
  {

  public:

    typedef enum
    {
      T_COLOR_LINEAR = 0,
      T_COLOR_EXPONENTIAL,
      T_COLOR_LOGARITHMIC
    } TColorScale;

    typedef enum
    {
      T_NORM_GLOBAL = 0,
      T_NORM_MAX
    } TNormalize_Rule;

    Histogram( const brion::Spikes& spikes, float startTime, float endTime );
    Histogram( const brion::SpikeReport& spikeReport );

    void CreateHistogram( unsigned int binsNumber = 10000 );
    void CalculateColors( void );

    void filteredGIDs( const GIDUSet& gids );
    const GIDUSet& filteredGIDs( void );

    void colorScale( TColorScale scale );
    TColorScale colorScale( void );

    void normalizeRule( TNormalize_Rule normRule );
    TNormalize_Rule normalizeRule( void );

    const std::vector< unsigned int>& histogram( void );

    const utils::InterpolationSet< glm::vec4 >& colorMapper( void );
    void colorMapper(const utils::InterpolationSet< glm::vec4 >& colors );

    const QGradientStops& gradientStops( void );

  protected:

    std::vector< unsigned int > _histogram;
    unsigned int _maxValueHistogram;
    QGradientStops _gradientStops;

    brion::Spikes _spikes;
    float _startTime;
    float _endTime;

    float (*_scaleFunc)( float value, float maxValue);
    TColorScale _colorScale;
    TNormalize_Rule _normRule;

    utils::InterpolationSet< glm::vec4 > _colorMapper;

    GIDUSet _filteredGIDs;
  };
}



#endif /* SRC_SIMULATIONSUMMARYWIDGET_H_ */
