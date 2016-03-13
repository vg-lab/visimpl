/*
 * Histogram.h
 *
 *  Created on: 11 de mar. de 2016
 *      Author: sgalindo
 */

#ifndef __HISTOGRAM_H__
#define __HISTOGRAM_H__

#include <brion/brion.h>
#include <prefr/prefr.h>

#include <unordered_set>

#include <QFrame>

typedef std::unordered_set< uint32_t > GIDUSet;
typedef utils::InterpolationSet< glm::vec4 > TColorMapper;

namespace visimpl
{
  class Histogram : public QFrame
  {

    Q_OBJECT;

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

    void CreateHistogram( unsigned int binsNumber = 250 );
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

    virtual void mouseMoveEvent( QMouseEvent* event_ );
    void mousePosition( QPoint* mousePosition_ );

    unsigned int valueAt( float percentage );

signals:

  void mousePositionChanged( QPoint point );

  protected:

    virtual void paintEvent(QPaintEvent* event);

    std::vector< unsigned int > _histogram;
    unsigned int _maxValueHistogramLocal;
    unsigned int _maxValueHistogramGlobal;
    QGradientStops _gradientStops;

    brion::Spikes _spikes;
    float _startTime;
    float _endTime;

    float (*_scaleFunc)( float value, float maxValue);
    TColorScale _colorScale;
    TNormalize_Rule _normRule;

    utils::InterpolationSet< glm::vec4 > _colorMapper;

    GIDUSet _filteredGIDs;

    QPoint* _lastMousePosition;
  };
}



#endif /* __HISTOGRAM_H__ */
