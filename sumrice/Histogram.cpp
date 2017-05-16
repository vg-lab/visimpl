/*
 * @file  Histogram.cpp
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */

#include "Histogram.h"

#include "log.h"

#include <QPainter>
#include <QBrush>

#include <QMouseEvent>

#ifdef VISIMPL_USE_OPENMP
#include <omp.h>
#endif

namespace visimpl
{
  MultiLevelHistogram::MultiLevelHistogram( )
  : QFrame( nullptr )
  , _bins( 50 )
  , _zoomFactor( 1.5f )
  , _spikes( nullptr )
  , _startTime( 0.0f )
  , _endTime( 0.0f )
  , _player( nullptr )
  , _scaleFuncLocal( nullptr )
  , _scaleFuncGlobal( nullptr )
  , _colorScaleLocal( T_COLOR_LINEAR )
  , _colorScaleGlobal( T_COLOR_LOGARITHMIC )
  , _colorLocal( "#e31a1c" )
  , _colorGlobal( "#1f78b4" )
  , _prevColorScaleLocal( T_COLOR_UNDEFINED )
  , _prevColorScaleGlobal( T_COLOR_UNDEFINED )
  , _normRule( T_NORM_MAX )
  , _repMode( T_REP_DENSE )
  , _fillPlots( true )
  , _lastMousePosition( nullptr )
  , _regionPercentage( nullptr )
  , _paintRegion( false )
  , _regionWidth( 0.1f )
  , _gridLinesNumber( 0 )
  , _paintTimeline( false )
  , _pixelsPerCharacter( 10 )
  , _pixelMargin( 5 )
  , _events( nullptr )
  {
//    init( );
  }


  MultiLevelHistogram::MultiLevelHistogram( const simil::Spikes& spikes,
                                 float startTime,
                                 float endTime )
  : QFrame( nullptr )
  , _bins( 50 )
  , _zoomFactor( 1.5f )
  , _spikes( &spikes )
  , _startTime( startTime )
  , _endTime( endTime )
  , _player( nullptr )
  , _scaleFuncLocal( nullptr )
  , _scaleFuncGlobal( nullptr )
  , _colorScaleLocal( T_COLOR_LINEAR )
  , _colorScaleGlobal( T_COLOR_LOGARITHMIC )
  , _colorLocal( "#e31a1c" )
  , _colorGlobal( "#1f78b4" )
  , _prevColorScaleLocal( T_COLOR_UNDEFINED )
  , _prevColorScaleGlobal( T_COLOR_UNDEFINED )
  , _normRule( T_NORM_MAX )
  , _repMode( T_REP_DENSE )
  , _fillPlots( true )
  , _lastMousePosition( nullptr )
  , _regionPercentage( nullptr )
  , _paintRegion( false )
  , _regionWidth( 0.1f )
  , _gridLinesNumber( 0 )
  , _paintTimeline( false )
  , _pixelsPerCharacter( 8 )
  , _pixelMargin( 5 )
  , _events( nullptr )
  {
//    init( );
  }

  MultiLevelHistogram::MultiLevelHistogram( const simil::SpikeData& spikeReport )
  : QFrame( nullptr )
  , _bins( 50 )
  , _zoomFactor( 1.5f )
  , _spikes( &spikeReport.spikes( ))
  , _startTime( spikeReport.startTime( ))
  , _endTime( spikeReport.endTime( ))
  , _player( nullptr )
  , _scaleFuncLocal( nullptr )
  , _scaleFuncGlobal( nullptr )
  , _colorScaleLocal( T_COLOR_LINEAR )
  , _colorScaleGlobal( T_COLOR_LOGARITHMIC )
  , _colorLocal( "#e31a1c" )
  , _colorGlobal( "#1f78b4" )
  , _prevColorScaleLocal( T_COLOR_UNDEFINED )
  , _prevColorScaleGlobal( T_COLOR_UNDEFINED )
  , _normRule( T_NORM_MAX )
  , _repMode( T_REP_DENSE )
  , _fillPlots( true )
  , _lastMousePosition( nullptr )
  , _regionPercentage( nullptr )
  , _paintRegion( false )
  , _regionWidth( 0.1f )
  , _gridLinesNumber( 0 )
  , _paintTimeline( false )
  , _pixelsPerCharacter( 8 )
  , _pixelMargin( 5 )
  , _events( nullptr )
  {
//    init( );
  }


  void MultiLevelHistogram::Spikes( const simil::Spikes& spikes,
                                    float startTime,
                                    float endTime )
  {
    _spikes = &spikes;
    _startTime = startTime;
    _endTime = endTime;
  }

  void MultiLevelHistogram::Spikes( const simil::SpikeData& spikeReport )
  {
    _spikes = &spikeReport.spikes( );
    _startTime = spikeReport.startTime( );
    _endTime = spikeReport.endTime( );
  }

  void MultiLevelHistogram::init( unsigned int binsNumber, float zoomFactor_ )
  {
    _zoomFactor = zoomFactor_;
    bins( binsNumber );

    colorScaleLocal( _colorScaleLocal );
    colorScaleGlobal( _colorScaleGlobal );

    _paintRegion = false;
    setMouseTracking( true );
  }

  void MultiLevelHistogram::BuildHistogram( THistogram histogramNumber )
  {
    Histogram* histogram = &_mainHistogram;
//    Histogram* aux = new Histogram( *histogram );

    if( histogramNumber == T_HIST_FOCUS )
      histogram = &_focusHistogram;

    std::vector< unsigned int > globalHistogram( histogram->size( ), 0 );

    float totalTime = _endTime - _startTime ;

    bool filter = _filteredGIDs.size( ) > 0;

#ifndef VISIMPL_USE_OPENMP

    float deltaTime = ( totalTime ) / histogram->size( );
    float currentTime = _startTime + deltaTime;

    auto spike = _spikes->begin( );
    for( unsigned int& bin: *histogram )
    {
      while( spike != _spikes->end( ) && spike->first <= currentTime )
      {
        if( !filter || _filteredGIDs.find( spike->second ) != _filteredGIDs.end( ))
        {
          bin++;
        }
        spike++;
      }

      currentTime += deltaTime;
    }

#else

    float invTotalTime = 1.0f / totalTime;
    unsigned int numThreads = 4;

    omp_set_dynamic( 0 );
    omp_set_num_threads( numThreads );
    const auto& references = _spikes->refData( );
    for( int i = 0; i < ( int ) references.size( ); i++)
    {
      simil::TSpikes::const_iterator spikeIt = references[ i ];

      float endTime = ( i < ( ( int )references.size( ) - 1 )) ?
                      references[ i + 1]->first :
                      _endTime;

      int bin;
      while( spikeIt->first < endTime && spikeIt != _spikes->end( ))
      {

        if( !filter || _filteredGIDs.find( spikeIt->second ) != _filteredGIDs.end( ))
        {
          float perc =
              std::max( 0.0f,
                        std::min( 1.0f, ( spikeIt->first - _startTime )* invTotalTime ));
          bin = perc * histogram->size( );

          ( *histogram )[ bin ]++;
        }
        ++spikeIt;
      }

    }

#endif // VISIMPL_USE_OPENMP

    unsigned int cont = 0;
//    unsigned int maxPos = 0;
    for( auto bin: *histogram )
    {
      if( bin > histogram->_maxValueHistogramLocal )
      {
        histogram->_maxValueHistogramLocal = bin;
//        maxPos = cont;
      }
      cont++;
    }

    histogram->_maxValueHistogramGlobal = histogram->_maxValueHistogramLocal;

    if( filter )
    {
      for( auto bin : globalHistogram )
      {
        if( bin > histogram->_maxValueHistogramGlobal )
        {
          histogram->_maxValueHistogramGlobal = bin;
        }
      }
    }

    CalculateColors( histogramNumber );

  }

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
      case T_COLOR_LINEAR:

        result = 1.0f / maxValue;

        break;
//      case T_COLOR_EXPONENTIAL:
//
//        result = maxValue;
//
//        break;
      case T_COLOR_LOGARITHMIC:

        result = 1.0f / log10f( maxValue );

        break;
      default:
        VISIMPL_THROW( "Selected color scale function is not available.")
        break;
    }

    return result;
  }

  void MultiLevelHistogram::CalculateColors( THistogram histogramNumber )
  {
    Histogram* histogram = &_mainHistogram;

    if( histogramNumber == T_HIST_FOCUS )
      histogram = &_focusHistogram;

    if( _repMode == T_REP_DENSE )
    {

      QGradientStops stops;

      float maxValue = 0.0f;
      float relativeTime = 0.0f;
      float delta = 1.0f / histogram->size( );
      float percentage;

      maxValue = _normRule == T_NORM_GLOBAL ?
                  histogram->_maxValueHistogramGlobal :
                  histogram->_maxValueHistogramLocal;

      maxValue = maxValueFunc( maxValue, _normRule == T_NORM_GLOBAL ?
                                          _colorScaleGlobal :
                                          _colorScaleLocal );

      for( auto bin: *histogram )
      {
        percentage = _scaleFuncLocal( float( bin ), maxValue );
        percentage = std::max< float >( std::min< float > (1.0f, percentage ), 0.0f);

        glm::vec4 color = _colorMapper.GetValue( percentage );
        stops << qMakePair( relativeTime, QColor( color.r, color.g, color.b, color.a ));

        relativeTime += delta;
      }

      histogram->_gradientStops = stops;
    }
    else if( _repMode == T_REP_CURVE )
    {

      float invMaxValueLocal;
      float invMaxValueGlobal;

      QPolygonF auxLocal;
      QPolygonF auxGlobal;

      auxLocal.reserve( histogram->size( ));
      auxGlobal.reserve( histogram->size( ));

      invMaxValueLocal = maxValueFunc( histogram->_maxValueHistogramLocal,
                                       _colorScaleLocal );

      invMaxValueGlobal = maxValueFunc( histogram->_maxValueHistogramGlobal,
                                        _colorScaleGlobal );

      float currentX;
      float globalY;
      float localY;
      unsigned int counter = 0;
      float invBins = 1.0f / float( histogram->size( ) - 1);

      for( auto bin: *histogram )
      {
        currentX = counter * invBins;

        globalY = 0.0f;
        localY = 0.0f;

        if( bin > 0)
        {
            globalY = _scaleFuncGlobal( float( bin ), invMaxValueGlobal );

            localY = _scaleFuncLocal( float( bin ), invMaxValueLocal );
        }

          auxGlobal.push_back( QPointF( currentX, 1.0f - globalY ));

          auxLocal.push_back( QPointF( currentX, 1.0f - localY ));

        counter++;
      }

      histogram->_curveStopsGlobal = auxGlobal;

      histogram->_curveStopsLocal = auxLocal;
    }
  }

  void MultiLevelHistogram::bins( unsigned int binsNumber )
  {
    if( _bins == binsNumber )
      return;

    _bins = binsNumber;

    _mainHistogram.clear( );
    _mainHistogram.resize( _bins, 0 );
    _mainHistogram._maxValueHistogramLocal = 0;
    _mainHistogram._maxValueHistogramGlobal = 0;

    BuildHistogram( T_HIST_MAIN );

    unsigned int focusBins = _bins * _zoomFactor;

    _focusHistogram.clear( );
    _focusHistogram.resize( focusBins, 0 );
    _focusHistogram._maxValueHistogramLocal = 0;
    _focusHistogram._maxValueHistogramGlobal = 0;

    BuildHistogram( T_HIST_FOCUS );
  }

  unsigned int MultiLevelHistogram::bins( void )
  {
    return _bins;
  }

  void MultiLevelHistogram::zoomFactor( float factor )
  {
    _zoomFactor = factor;

    unsigned int focusBins = _bins * _zoomFactor;

    _focusHistogram.clear( );
    _focusHistogram.resize( focusBins, 0 );
    _focusHistogram._maxValueHistogramLocal = 0;
    _focusHistogram._maxValueHistogramGlobal = 0;

    BuildHistogram( T_HIST_FOCUS );
  }

  float MultiLevelHistogram::zoomFactor( void )
  {
    return _zoomFactor;
  }

  void MultiLevelHistogram::filteredGIDs( const GIDUSet& gids )
  {
    if( gids.size( ) > 0 )
      _filteredGIDs = gids;
  }

  const GIDUSet& MultiLevelHistogram::filteredGIDs( void )
  {
    return _filteredGIDs;
  }

  void MultiLevelHistogram::colorScaleLocal( TColorScale scale )
  {

    _prevColorScaleLocal = _colorScaleLocal;
    _colorScaleLocal = scale;

    switch( _colorScaleLocal )
    {
      case T_COLOR_LINEAR:

        _scaleFuncLocal = linearFunc;

        break;
  //    case T_COLOR_EXPONENTIAL:
  //
  //      _scaleFuncLocal = exponentialFunc;
  //
  //      break;
      case T_COLOR_LOGARITHMIC:

        _scaleFuncLocal = logarithmicFunc;

        break;
      default:
        break;
    }
  }

  void MultiLevelHistogram::colorScaleGlobal( TColorScale scale )
  {

    _prevColorScaleGlobal = _colorScaleGlobal;
    _colorScaleGlobal = scale;

    switch( _colorScaleGlobal )
    {
      case T_COLOR_LINEAR:

        _scaleFuncGlobal = linearFunc;

        break;
  //    case T_COLOR_EXPONENTIAL:
  //
  //      _scaleFuncGlobal = exponentialFunc;
  //
  //      break;
      case T_COLOR_LOGARITHMIC:

        _scaleFuncGlobal = logarithmicFunc;

        break;
      default:
        break;
    }
  }

  TColorScale MultiLevelHistogram::colorScaleLocal( void )
  {
    return _colorScaleLocal;
  }

  void MultiLevelHistogram::normalizeRule(
      TNormalize_Rule normRule )
  {
    _normRule = normRule;
  }

  TNormalize_Rule MultiLevelHistogram::normalizeRule( void )
  {
    return _normRule;
  }


  void MultiLevelHistogram::gridLinesNumber( unsigned int linesNumber )
  {
    _gridLinesNumber = linesNumber;

    _mainHistogram._gridLines.clear( );

    std::vector< float > gridLines;
    if( linesNumber > 0 )
    {

      float current = 0;

      float delta = 1.0f / float( linesNumber + 1 );

      gridLines.push_back( 0.0f );

      for( unsigned int i = 1; i <= linesNumber; ++i )
      {
        current += delta;
        gridLines.push_back( current );
      }
      gridLines.push_back( 1.0f );
    }

    _mainHistogram._gridLines = gridLines;

  }

  unsigned int MultiLevelHistogram::gridLinesNumber( void )
  {
    return _gridLinesNumber;
  }

  void MultiLevelHistogram::representationMode(
      TRepresentation_Mode repMode )
  {
    _repMode = repMode;
  }

  TRepresentation_Mode
  MultiLevelHistogram::representationMode( void )
  {
    return _repMode;
  }

  unsigned int MultiLevelHistogram::histogramSize( void ) const
  {
    return _mainHistogram.size( );
  }

  unsigned int MultiLevelHistogram::focusHistogramSize( void ) const
  {
    return _focusHistogram.size( );
  }


  unsigned int MultiLevelHistogram::maxLocal( void )
  {
    return _mainHistogram._maxValueHistogramLocal;
  }

  unsigned int MultiLevelHistogram::maxGlobal( void )
  {
    return _mainHistogram._maxValueHistogramGlobal;
  }

  unsigned int MultiLevelHistogram::focusMaxLocal( void )
  {
    return _focusHistogram._maxValueHistogramLocal;
  }

  unsigned int MultiLevelHistogram::focusMaxGlobal( void )
  {
    return _focusHistogram._maxValueHistogramGlobal;
  }


  const utils::InterpolationSet< glm::vec4 >&
  MultiLevelHistogram::colorMapper( void )
  {
    return _colorMapper;
  }

  void MultiLevelHistogram::colorMapper(
      const utils::InterpolationSet< glm::vec4 >& colors )
  {
    _colorMapper = colors;
  }

  QColor MultiLevelHistogram::colorLocal( void )
  {
    return _colorLocal;
  }

  void MultiLevelHistogram::colorLocal( const QColor& color )
  {
    _colorLocal = color;
  }

  QColor MultiLevelHistogram::colorGlobal( void )
  {
    return _colorGlobal;
  }

  void MultiLevelHistogram::colorGlobal( const QColor& color )
  {
    _colorGlobal = color;
  }

  const QGradientStops& MultiLevelHistogram::gradientStops( void )
  {
    return _mainHistogram._gradientStops;
  }

  unsigned int MultiLevelHistogram::valueAt( float percentage )
  {
    unsigned int position =
        std::max( 0.0f, std::min( 1.0f, percentage)) * _mainHistogram.size( );

    if( position >= _mainHistogram.size( ))
      position = _mainHistogram.size( ) - 1;

    return _mainHistogram[ position ];
  }

  unsigned int MultiLevelHistogram::focusValueAt( float percentage )
  {
    unsigned int position = percentage * _focusHistogram.size( );

    if( position >= _focusHistogram.size( ))
      position = _focusHistogram.size( ) - 1;

    return _focusHistogram[ position ];
  }

  bool MultiLevelHistogram::isInitialized( void )
  {
    return _mainHistogram._gradientStops.size( ) > 0 ||
           _mainHistogram._curveStopsGlobal.size( ) > 0;
  }

  QPolygonF MultiLevelHistogram::localFunction( void ) const
  {
    return _mainHistogram._curveStopsLocal;
  }

  QPolygonF MultiLevelHistogram::globalFunction( void ) const
  {
    return _mainHistogram._curveStopsGlobal;
  }

  QPolygonF MultiLevelHistogram::focusLocalFunction( void ) const
  {
    return _focusHistogram._curveStopsLocal;
  }

  QPolygonF MultiLevelHistogram::focusGlobalFunction( void ) const
  {
    return _focusHistogram._curveStopsGlobal;
  }

  void MultiLevelHistogram::fillPlots( bool fillPlots_ )
  {
    _fillPlots = fillPlots_;
  }

  void MultiLevelHistogram::mousePressEvent( QMouseEvent* event_ )
  {
    QFrame::mousePressEvent( event_ );

    QPoint position = event_->pos( );

    float percentage = position.x( ) / float( width( ));

    if( event_->modifiers( ) == Qt::ControlModifier ||
        event_->modifiers( ) == Qt::ShiftModifier )
    {
      emit mouseModifierPressed( percentage, event_->modifiers( ));
    }
    else
    {
      emit mousePressed( mapToGlobal( position ), percentage );
    }
  }

  void MultiLevelHistogram::mouseReleaseEvent( QMouseEvent* event_ )
  {
    QPoint position = event_->pos( );

    float percentage = position.x( ) / float( width( ));
    emit mouseReleased( mapToGlobal( position ), percentage );
  }

  void MultiLevelHistogram::mouseMoveEvent( QMouseEvent* event_ )
  {
    QFrame::mouseMoveEvent( event_ );

    QPoint position = event_->pos( );

    position = mapToGlobal( position );

    emit mousePositionChanged( position );
  }

  void MultiLevelHistogram::mousePosition( QPoint* mousePosition_ )
  {
    _lastMousePosition = mousePosition_;
  }

  void MultiLevelHistogram::regionPosition( float* regionPercentage )
  {
    _regionPercentage = regionPercentage;
  }

  void MultiLevelHistogram::simPlayer( simil::SimulationPlayer* player )
  {
    _player = player;
  }

  void MultiLevelHistogram::regionWidth( float region_ )
  {
    _regionWidth = region_;
  }

  float MultiLevelHistogram::regionWidth( void )
  {
    return _regionWidth;
  }

  void MultiLevelHistogram::paintRegion( bool region )
  {
    _paintRegion = region;
  }

  void MultiLevelHistogram::firstHistogram( bool first )
  {
    _paintTimeline = first;
  }

  void MultiLevelHistogram::resizeEvent( QResizeEvent* /*event*/ )
  {
    _mainHistogram._cachedLocalRep = QPainterPath( );
    _mainHistogram._cachedGlobalRep = QPainterPath( );

    _mainHistogram._cachedLocalRep.moveTo( 0, height( ) );
    for( auto point : _mainHistogram._curveStopsLocal )
    {
      _mainHistogram._cachedLocalRep.lineTo( QPoint( point.x( ) * width( ),
                                                     point.y( ) * height( )));
    }
    _mainHistogram._cachedLocalRep.lineTo( width( ), height( ) );

    _mainHistogram._cachedGlobalRep.moveTo( 0, height( ) );
    for( auto point : _mainHistogram._curveStopsGlobal )
    {
      _mainHistogram._cachedGlobalRep.lineTo( QPoint( point.x( ) * width( ),
                                                      point.y( ) * height( )));
    }
    _mainHistogram._cachedGlobalRep.lineTo( width( ), height( ) );

  }

  void MultiLevelHistogram::paintEvent( QPaintEvent* /*e*/)
  {
    QPainter painter( this );
    unsigned int currentHeight = height( );

    QColor penColor;

    if( _repMode == T_REP_DENSE )
    {
      QLinearGradient gradient( 0, 0, width( ), 0 );

      QRect area = rect();

      gradient.setStops( _mainHistogram._gradientStops );
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

      QColor globalColor( _colorGlobal );
      QColor localColor( _colorLocal );


      if( _fillPlots )
      {
        globalColor.setAlpha( 50 );
        localColor.setAlpha( 50 );

        painter.setBrush( QBrush( globalColor, Qt::SolidPattern));
        painter.setPen( Qt::NoPen );
        painter.drawPath( _mainHistogram._cachedGlobalRep );

        painter.setBrush( QBrush( localColor, Qt::SolidPattern));
        painter.setPen( Qt::NoPen );
        painter.drawPath( _mainHistogram._cachedLocalRep );
      }
      else
      {
        globalColor.setAlpha( 100 );
        localColor.setAlpha( 100 );

        painter.setBrush( Qt::NoBrush );
        painter.setPen( QPen( globalColor, Qt::SolidLine ));
        painter.drawPath( _mainHistogram._cachedGlobalRep );

        painter.setBrush( Qt::NoBrush );
        painter.setPen( QPen( localColor, Qt::SolidLine ));
        painter.drawPath( _mainHistogram._cachedLocalRep );
      }

      penColor = QColor( 0, 0, 0 );
    }

    if( _mainHistogram._gridLines.size( ) > 0 )
    {
      for( auto line : _mainHistogram._gridLines )
      {
        int positionX = line * width( );

        QPen pen( penColor );

        if( _paintTimeline )
        {

          float timeValue = ( _endTime - _startTime ) * line + _startTime;

          QString value = QString::number( ( unsigned int ) timeValue );
          int valueLength = value.length( ) * _pixelsPerCharacter;
          if( width( ) - positionX < valueLength )
            _pixelMargin = -valueLength;
          QPoint position ( positionX + _pixelMargin, currentHeight * 0.25 );
          pen.setColor( QColor( 150, 150, 150 ));
          painter.setPen( pen );

          QFont backFont = painter.font( );

          painter.drawText( position, value );

        }

        QLine marker( QPoint( positionX, 0 ), QPoint( positionX, height( )));
        pen.setColor( QColor( 177, 50, 177 ));
        painter.setPen( pen );
        painter.drawLine( marker );
      }

    }


    if( _lastMousePosition )
    {
      QPoint localPosition = mapFromGlobal( *_lastMousePosition );

      localPosition.setX( std::min( width( ), std::max( 0, localPosition.x( ))));
      localPosition.setY( std::min( height( ), std::max( 0, localPosition.y( ))));

      float percentage = float( localPosition.x( )) / float( width( ));

      int positionX = localPosition.x( );
      int margin = 5;

      QString value = QString::number( valueAt( percentage ));
      int valueLength = value.length( ) * 10;
      if( width( ) - positionX < valueLength )
        margin = -valueLength;

      if( _regionPercentage && _paintRegion )
      {

        int regionPosX = width( ) * ( *_regionPercentage );

        int regionW = _regionWidth * width( );
        int start = std::max( 0, regionPosX - regionW );
        if( ( regionPosX + regionW ) > width( ) )
          start = width( ) - regionW * 2;

        QRect region( std::max( 0, start ), 0, regionW * 2, height( ));

        QPen pen( Qt::NoPen );
        QBrush brush( QColor( 30, 30, 30, 30 ));

        painter.setBrush( brush );
        painter.setPen( QColor( 0, 0, 0, 200 ));
        painter.drawRect( region );
      }

      QPen pen( penColor );
      painter.setPen( pen );

//      currentHeight = currentHeight / 2;
      QPoint position ( positionX + margin, height( ) * 0.75f );

      painter.drawText( position, QString::number( valueAt( percentage )));

      QLine marker( QPoint( positionX, 0 ), QPoint( positionX, height( )));
      pen.setColor( QColor( 177, 50, 50 ));
      painter.setPen( pen );
      painter.drawLine( marker );
    }

    if( _events && _repMode == T_REP_CURVE )
    {
      for( const auto& timeFrame : *_events )
      {
        QColor color = timeFrame.color;

        color.setAlpha( 50 );
        painter.setBrush( QBrush( color, Qt::SolidPattern));
        painter.setPen( Qt::NoPen );
        for( const auto& p : timeFrame._cachedCommonRep )
          painter.drawPath( p );
      }
    }

    if( _player )
    {
      int lineX = _player->GetRelativeTime( ) * width( );

      QLine simMarkerLine( QPoint( lineX, 0), QPoint( lineX, height( )));

      QPen pen( QColor( 0, 0, 0, 255 ));
      painter.setPen( pen );
      painter.drawLine( simMarkerLine );

      if( _paintTimeline )
      {
        QString value = QString::number(_player->currentTime( ));

        int valueLength = value.length( ) * _pixelsPerCharacter;
        if( width( ) - lineX < valueLength )
          _pixelMargin = -valueLength;
        QPoint position ( lineX + _pixelMargin, height( ) * 0.5f );
        pen.setColor( QColor( 150, 150, 150 ));
        painter.setPen( pen );

        QFont backFont = painter.font( );

        painter.drawText( position, value );
      }
    }

  }

}


