/*
 * Copyright (c) 2015-2020 VG-Lab/URJC.
 *
 * Authors: Sergio E. Galindo <sergio.galindo@urjc.es>
 *
 * This file is part of ViSimpl <https://github.com/vg-lab/visimpl>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3.0 as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#ifndef __HISTOGRAM_H__
#define __HISTOGRAM_H__

#include <simil/simil.h>
#include <sumrice/api.h>

#include <unordered_set>

#include <QFrame>

#include "ColorInterpolator.h"
#include "types.h"

namespace visimpl
{

  class SUMRICE_API HistogramWidget : public QFrame
  {
    friend class Summary;

    Q_OBJECT;

  protected:

    class Histogram : public std::vector< unsigned int >
    {
    public:

      Histogram( )
      : std::vector< unsigned int >( )
      , _maxValueHistogramLocal( 0 )
      , _maxValueHistogramGlobal( 0 )
      { }

      unsigned int _maxValueHistogramLocal;
      unsigned int _maxValueHistogramGlobal;
      QGradientStops _gradientStops;
      QPolygonF _curveStopsLocal;
      QPolygonF _curveStopsGlobal;

      QPainterPath _cachedLocalRep;
      QPainterPath _cachedGlobalRep;

      std::vector< float > _gridLines;
    };

  public:

    typedef enum
    {
      T_HIST_MAIN = 0,
      T_HIST_FOCUS
    } THistogram;

    HistogramWidget( void );
    HistogramWidget( const simil::Spikes& spikes,
                         float startTime,
                         float endTime );

    HistogramWidget( const simil::SpikeData& spikeReport );

    virtual void init( unsigned int binsNumber = 250, float zoomFactor = 1.5f );
    bool empty( void ) const;

    void Spikes( const simil::Spikes& spikes, float startTime, float endTime );

    void Spikes( const simil::SpikeData& spikeReport  );

    void Update( THistogram histogramNumber = T_HIST_MAIN );

    void BuildHistogram( THistogram histogram = T_HIST_MAIN );

    void CalculateColors( THistogram histogramNumber = T_HIST_MAIN );

    unsigned int gidsSize( void );

    void name( const std::string& name_ );
    const std::string& name( void ) const;

    void bins( unsigned int binsNumber );
    unsigned int bins( void ) const;

    void zoomFactor( float factor );
    float zoomFactor( void ) const;

    void filteredGIDs( const GIDUSet& gids );
    const GIDUSet& filteredGIDs( void ) const;

    void colorScaleLocal( TColorScale scale );
    TColorScale colorScaleLocal( void ) const;

    void colorScaleGlobal( TColorScale scale );
    TColorScale colorScaleGlobal( void ) const;

    QColor colorLocal( void ) const;
    void colorLocal( const QColor& );

    QColor colorGlobal( void ) const;
    void colorGlobal( const QColor& );

    void normalizeRule( TNormalize_Rule normRule );
    TNormalize_Rule normalizeRule( void ) const;

    void representationMode( TRepresentation_Mode repType );
    TRepresentation_Mode representationMode( void ) const;

    void gridLinesNumber( unsigned int linesNumber );
    unsigned int gridLinesNumber( void ) const;

    unsigned int histogramSize( void ) const;
    unsigned int maxLocal( void ) const;
    unsigned int maxGlobal( void ) const;

    unsigned int focusHistogramSize( void ) const;
    unsigned int focusMaxLocal( void ) const;
    unsigned int focusMaxGlobal( void ) const;

    const ColorInterpolator& colorMapper( void );
    void colorMapper(const ColorInterpolator& colors );

    const QGradientStops& gradientStops( void );

    virtual void mousePressEvent( QMouseEvent* event_ );
    virtual void mouseReleaseEvent( QMouseEvent* event_ );
    virtual void mouseMoveEvent( QMouseEvent* event_ );
    void mousePosition( QPoint* mousePosition_ );
//    void regionPosition( QPoint* regionPosition_ );
    void regionPosition( float* regionPercentage );

    void simPlayer( simil::SimulationPlayer* player );

    void regionWidth( float region_ );
    float regionWidth( void );
    void paintRegion( bool region = false );
    void firstHistogram( bool first = false );

    unsigned int valueAt( float percentage );
    unsigned int focusValueAt( float percentage );
    float timeAt( float percentage );

    bool isInitialized( void );

    QPolygonF localFunction( void ) const;
    QPolygonF globalFunction( void ) const;

    QPolygonF focusLocalFunction( void ) const;
    QPolygonF focusGlobalFunction( void ) const;

    void fillPlots( bool fillPlots_ );

signals:

    void mousePositionChanged( QPoint point );
    void mousePressed( QPoint coordinates, float position );
    void mouseReleased( QPoint coordinates, float position );
    void mouseModifierPressed( float position,  Qt::KeyboardModifiers modifiers );

  protected:

    void updateCachedRep( void );

    virtual void resizeEvent( QResizeEvent* event );
    virtual void paintEvent( QPaintEvent* event );

    Histogram _mainHistogram;
    Histogram _focusHistogram;

    std::string _name;
    unsigned int _bins;
    float _zoomFactor;

    const simil::Spikes* _spikes;
    float _startTime;
    float _endTime;

    simil::SimulationPlayer* _player;

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

    bool _fillPlots;

    ColorInterpolator _colorMapper;

    GIDUSet _filteredGIDs;

    QPoint* _lastMousePosition;
    float* _regionPercentage;

    bool _paintRegion;
    float _regionWidth;

    unsigned int _gridLinesNumber;
    bool _paintTimeline;

    unsigned int _pixelsPerCharacter;
    unsigned int _pixelMargin;

    std::vector< TEvent >* _events;

    bool _autoBuildHistogram;
    bool _autoCalculateColors;
  };
}



#endif /* __HISTOGRAM_H__ */
