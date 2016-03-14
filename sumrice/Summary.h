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

#include <mutex>

#include <QWidget>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include <QTimer>

#include "Histogram.h"

namespace visimpl
{
  class Histogram;


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

  Summary( QWidget* parent = 0 );
  Summary( QWidget* parent = 0, TStackType stackType = T_STACK_FIXED);

  void Init( brion::SpikeReport* _spikes, brion::GIDSet gids );
  void AddNewHistogram( const visimpl::Selection& selection
#ifdef VISIMPL_USE_ZEQ
                       , bool deferredInsertion = false
#endif
                       );
//  void CreateSummary( brion::CompartmentReport* _voltages );

  virtual void mouseMoveEvent( QMouseEvent* event_ );

  void bins( unsigned int bins_ );
  unsigned int bins( void );

  unsigned int histogramsNumber( void );

  void heightPerRow( unsigned int height_ );
  unsigned int heightPerRow( void );

  void showMarker( bool show_ );

protected slots:

  void updateMouseMarker( QPoint point );

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
    {
//      delete histogram;
//      delete label;
//      delete checkBox;
    }

    visimpl::Histogram* histogram;
    QLabel* label;
    QCheckBox* checkBox;

  };

#ifdef VISIMPL_USE_ZEQ

protected slots:

  void deferredInsertion( void );

protected:

  std::list< visimpl::Selection > _pendingSelections;
  std::mutex _mutex;
  QTimer _insertionTimer;

#endif

  void CreateSummarySpikes( );
  void InsertSummarySpikes( const GIDUSet& gids );
//  void CreateSummaryVoltages( void );

  void UpdateGradientColors( void );

  unsigned int _bins;

  brion::SpikeReport* _spikeReport;
  brion::CompartmentReport* _voltageReport;

  GIDUSet _gids;

  visimpl::Histogram* _mainHistogram;
  visimpl::Histogram* _selectionHistogram;

  TStackType _stackType;

  std::vector< visimpl::Histogram* > _histograms;
  std::vector< QCheckBox* > _checkBoxes;
  std::vector< StackRow > _rows;

  QGridLayout* _mainLayout;
  QWidget* _body;

  unsigned int _maxColumns;
  unsigned int _summaryColumns;
  unsigned int _heightPerRow;

  QPoint _lastMousePosition;
  bool _showMarker;

};



#endif /* __SIMULATIONSUMMARYWIDGET_H__ */
