/*
 * @file  Histogram.h
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */
#ifndef __HISTOGRAM_H__
#define __HISTOGRAM_H__

#include <brion/brion.h>
#include <prefr/prefr.h>
#include <simil/simil.h>

#include <unordered_set>

#include <QFrame>

#include "types.h"

namespace visimpl
{

  class MultiLevelHistogram : public QFrame
  {
    friend class Summary;

    Q_OBJECT;

  protected:

    class Histogram : public std::vector< unsigned int >
    {
    public:

      unsigned int _maxValueHistogramLocal;
      unsigned int _maxValueHistogramGlobal;
      QGradientStops _gradientStops;
      QPolygonF _curveStopsLocal;
      QPolygonF _curveStopsGlobal;
      std::vector< float > _gridLines;
    };

  public:

    typedef enum
    {
      T_HIST_MAIN = 0,
      T_HIST_FOCUS
    } THistogram;

    MultiLevelHistogram( void );
    MultiLevelHistogram( const brion::Spikes& spikes, float startTime, float endTime );
//    MultiLevelHistogram( const brion::SpikeReport& spikeReport );
    MultiLevelHistogram( const simil::SpikeData& spikeReport );

    virtual void init( unsigned int binsNumber = 250, float zoomFactor = 1.5f );

    void Spikes( const brion::Spikes& spikes, float startTime, float endTime );
//    void Spikes( const brion::SpikeReport& spikeReport );
    void Spikes( const simil::SpikeData& spikeReport  );

    void BuildHistogram( THistogram histogram = T_HIST_MAIN );

    void CalculateColors( THistogram histogramNumber = T_HIST_MAIN );

    void bins( unsigned int binsNumber );
    unsigned int bins( void );

    void zoomFactor( float factor );
    float zoomFactor( void );

    void filteredGIDs( const GIDUSet& gids );
    const GIDUSet& filteredGIDs( void );

    void colorScaleLocal( TColorScale scale );
    TColorScale colorScaleLocal( void );

    void colorScaleGlobal( TColorScale scale );
    TColorScale colorScaleGlobal( void );

    QColor colorLocal( void );
    void colorLocal( const QColor& );

    QColor colorGlobal( void );
    void colorGlobal( const QColor& );

    void normalizeRule( TNormalize_Rule normRule );
    TNormalize_Rule normalizeRule( void );

    void representationMode( TRepresentation_Mode repType );
    TRepresentation_Mode representationMode( void );

    void gridLinesNumber( unsigned int linesNumber );
    unsigned int gridLinesNumber( void );

    unsigned int histogramSize( void ) const;
    unsigned int maxLocal( void );
    unsigned int maxGlobal( void );

    unsigned int focusHistogramSize( void ) const;
    unsigned int focusMaxLocal( void );
    unsigned int focusMaxGlobal( void );

    const utils::InterpolationSet< glm::vec4 >& colorMapper( void );
    void colorMapper(const utils::InterpolationSet< glm::vec4 >& colors );

    const QGradientStops& gradientStops( void );

    virtual void mousePressEvent( QMouseEvent* event_ );
    virtual void mouseReleaseEvent( QMouseEvent* event_ );
    virtual void mouseMoveEvent( QMouseEvent* event_ );
    void mousePosition( QPoint* mousePosition_ );
//    void regionPosition( QPoint* regionPosition_ );
    void regionPosition( float* regionPercentage );

    void regionWidth( float region_ );
    float regionWidth( void );
    void paintRegion( bool region = false );
    void firstHistogram( bool first = false );

    unsigned int valueAt( float percentage );
    unsigned int focusValueAt( float percentage );

    bool isInitialized( void );

    QPolygonF localFunction( void ) const;
    QPolygonF globalFunction( void ) const;

    QPolygonF focusLocalFunction( void ) const;
    QPolygonF focusGlobalFunction( void ) const;

signals:

    void mousePositionChanged( QPoint point );
    void mousePressed( QPoint coordinates, float position );
    void mouseReleased( QPoint coordinates, float position );
    void mouseModifierPressed( float position,  Qt::KeyboardModifiers modifiers );

  protected:

    virtual void paintEvent( QPaintEvent* event );

    Histogram _mainHistogram;
    Histogram _focusHistogram;

    unsigned int _bins;
    float _zoomFactor;

    const brion::Spikes* _spikes;
    float _startTime;
    float _endTime;

    float (*_scaleFuncLocal)( float value, float maxValue);
    float (*_scaleFuncGlobal)( float value, float maxValue);

    TColorScale _colorScaleLocal;
    TColorScale _colorScaleGlobal;

    QColor _colorLocal;
    QColor _colorGlobal;

    TColorScale _prevColorScaleLocal;
    TColorScale _prevColorScaleGlobal;

    TNormalize_Rule _normRule;
    TRepresentation_Mode _repMode;

    utils::InterpolationSet< glm::vec4 > _colorMapper;

    GIDUSet _filteredGIDs;

    QPoint* _lastMousePosition;
//    QPoint* _regionPosition;
    float* _regionPercentage;

    bool _paintRegion;
    float _regionWidth;

    unsigned int _gridLinesNumber;
    bool _paintTimeline;

    std::vector< TimeFrame >* _timeFrames;
  };
}



#endif /* __HISTOGRAM_H__ */
