/*
 * @file  Summary.h
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */

#ifndef __SIMULATIONSUMMARYWIDGET_H__
#define __SIMULATIONSUMMARYWIDGET_H__

#include <brion/brion.h>
#include <prefr/prefr.h>
#include <simil/simil.h>

#include <QWidget>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include <QTimer>
#include <QScrollArea>

#include "FocusFrame.h"
#include "Histogram.h"
#include "SubsetEventWidget.h"

namespace visimpl
{
  class MultiLevelHistogram;


  struct Selection
  {
  public:

    Selection( void )
    {
      id = _counter;
      _counter++;
    }

    unsigned int currentID( void )
    {
      return _counter;
    }

    unsigned int id;
    std::string name;
    GIDUSet gids;

  private:
    static unsigned int _counter;
  };

  class Summary : public QWidget
  {

    Q_OBJECT;

  public:

    Summary( QWidget* parent = 0, TStackType stackType = T_STACK_FIXED);
    virtual ~Summary( ){};

    void Init( simil::SpikeData* spikes_,
               const simil::TGIDSet& gids_,
               simil::SubsetEventManager* subsets_ = nullptr );

    void AddNewHistogram( const visimpl::Selection& selection
  #ifdef VISIMPL_USE_ZEROEQ
                         , bool deferredInsertion = false
  #endif
                         );

    virtual void mouseMoveEvent( QMouseEvent* event_ );

    unsigned int bins( void );
    float zoomFactor( void );

    unsigned int histogramsNumber( void );

    void heightPerRow( unsigned int height_ );
    unsigned int heightPerRow( void );

    void showMarker( bool show_ );

    void colorScaleLocal( visimpl::TColorScale colorScale );
    visimpl::TColorScale colorScaleLocal( void );

    void colorScaleGlobal( visimpl::TColorScale colorScale );
    visimpl::TColorScale colorScaleGlobal( void );

    void regionWidth( float region );
    float regionWidth( void );

    const GIDUSet& gids( void );

    unsigned int gridLinesNumber( void );

  signals:

    void histogramClicked( float );
    void histogramClicked( visimpl::MultiLevelHistogram* );

  protected slots:

    void childHistogramPressed( const QPoint&, float );
    void childHistogramReleased( const QPoint&, float );
    void childHistogramClicked( float percentage,
                                Qt::KeyboardModifiers modifiers );

    void removeSelections( void );

    void colorScaleLocal( int value );
    void colorScaleGlobal( int value );

    void gridLinesNumber( int linesNumber );

    void updateMouseMarker( QPoint point );

  public slots:

    void bins( int bins_ );
    void zoomFactor( double zoom );

    void toggleAutoNameSelections( void )
    {
      _autoNameSelection = !_autoNameSelection;
    }
  protected:

    struct StackRow
    {
    public:

      StackRow( )
      : histogram( nullptr )
      , label( nullptr )
      , checkBox( nullptr )
      { }

      ~StackRow( )
      { }

      visimpl::MultiLevelHistogram* histogram;
      QLabel* label;
      QCheckBox* checkBox;

    };

    struct TimeFrameRow
    {
    public:

      TimeFrameRow( )
      : widget( nullptr )
      , label( nullptr )
      , checkBox( nullptr )
      { }

      ~TimeFrameRow( )
      { }

      visimpl::SubsetEventWidget* widget;
      QLabel* label;
      QCheckBox* checkBox;

    };

  #ifdef VISIMPL_USE_ZEROEQ

  protected slots:

    void deferredInsertion( void );
    void moveScrollSync( int newPos );

  protected:

    std::list< visimpl::Selection > _pendingSelections;
    QTimer _insertionTimer;

  #endif

  protected:

    void Init( void );

    void insertSubset( const Selection& selection );
    void insertSubset( const std::string& name, const GIDUSet& subset );

    void CreateSummarySpikes( );
    void InsertSummarySpikes( const GIDUSet& gids );
  //  void CreateSummaryVoltages( void );

    void UpdateGradientColors( bool replace = false );

    void updateRegionBounds( void );
    void SetFocusRegionPosition( const QPoint& localPosition );

    virtual void wheelEvent( QWheelEvent* event );

    unsigned int _bins;
    float _zoomFactor;

    unsigned int _gridLinesNumber;

  //  brion::SpikeReport* _spikeReport;
    simil::SpikeData* _spikeReport;
    brion::CompartmentReport* _voltageReport;

    simil::SimulationPlayer* _player;

    GIDUSet _gids;

    visimpl::MultiLevelHistogram* _mainHistogram;
    visimpl::MultiLevelHistogram* _detailHistogram;
    visimpl::MultiLevelHistogram* _focusedHistogram;
    QScrollArea* _histogramScroll;

    bool _mousePressed;

    TStackType _stackType;

    visimpl::TColorScale _colorScaleLocal;
    visimpl::TColorScale _colorScaleGlobal;

    QColor _colorLocal;
    QColor _colorGlobal;

    std::vector< visimpl::MultiLevelHistogram* > _histograms;
    std::vector< StackRow > _rows;

    FocusFrame* _focusWidget;

    QGridLayout* _mainLayout;
    QWidget* _body;
    QWidget* _localColorWidget;
    QWidget* _globalColorWidget;
    QLabel* _currentValueLabel;
    QLabel* _globalMaxLabel;
    QLabel* _localMaxLabel;

    simil::SubsetEventManager* _subsetEventManager;
    std::vector< TimeFrame > _timeFrames;
    QGridLayout* _timeFramesLayout;
    QScrollArea* _subsetScroll;
    std::vector< SubsetEventWidget* > _subsetEventWidgets;
    std::vector< TimeFrameRow > _subsetRows;

    bool _syncScrollsVertically;

    unsigned int _maxColumns;
    unsigned int _summaryColumns;
    unsigned int _heightPerRow;
    unsigned int _maxLabelWidth;
    unsigned int _currentCentralMinWidth;

    QPoint _lastMousePosition;
    QPoint _regionGlobalPosition;
    QPoint _regionLocalPosition;
    bool _showMarker;

    float _regionPercentage;
    float _regionWidth;
    int _regionWidthPixels;

    bool _overRegionEdgeLower;
    bool _selectedEdgeLower;
    int _regionEdgePointLower;
    float _regionEdgeLower;

    bool _overRegionEdgeUpper;
    bool _selectedEdgeUpper;
    int _regionEdgePointUpper;
    float _regionEdgeUpper;

    bool _autoNameSelection;

    std::vector< QColor > _subsetEventColorPalette;

  };

}

#endif /* __SIMULATIONSUMMARYWIDGET_H__ */
