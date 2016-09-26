/*
 * SimulationSummaryWidget.h
 *
 *  Created on: 16 de feb. de 2016
 *      Author: sgalindo
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

#include "FocusFrame.h"
#include "Histogram.h"

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
}
class Summary : public QWidget
{

  Q_OBJECT;

public:

  typedef enum
  {
    T_STACK_FIXED = 0,
    T_STACK_EXPANDABLE

  } TStackType;

  Summary( QWidget* parent = 0, TStackType stackType = T_STACK_FIXED);
  virtual ~Summary( ){};

  void Init( simil::SpikeData* spikes_, const simil::TGIDSet& gids_ );

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

#ifdef VISIMPL_USE_ZEROEQ

protected slots:

  void deferredInsertion( void );

protected:

  std::list< visimpl::Selection > _pendingSelections;
  QTimer _insertionTimer;

#endif

  void Init( void );

  void CreateSummarySpikes( );
  void InsertSummary( const visimpl::Selection& selection );
//  void CreateSummaryVoltages( void );

  void UpdateGradientColors( bool replace = false );

  void SetFocusRegionPosition( const QPoint& localPosition );

  unsigned int _bins;
  float _zoomFactor;

  unsigned int _gridLinesNumber;

//  brion::SpikeReport* _spikeReport;
  simil::SpikeData* _spikeReport;
  brion::CompartmentReport* _voltageReport;

  GIDUSet _gids;

  visimpl::MultiLevelHistogram* _mainHistogram;
  visimpl::MultiLevelHistogram* _detailHistogram;
  visimpl::MultiLevelHistogram* _focusedHistogram;

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

  unsigned int _maxColumns;
  unsigned int _summaryColumns;
  unsigned int _heightPerRow;

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

};



#endif /* __SIMULATIONSUMMARYWIDGET_H__ */
