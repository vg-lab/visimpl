/*
 * @file	DisplayManager.h
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es> 
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *					Do not distribute without further notice.
 */

#ifndef DISPLAYMANAGER_H_
#define DISPLAYMANAGER_H_

#include <sumrice/sumrice.h>

#include <QWidget>
#include <QTableWidget>


namespace stackviz
{

  typedef enum
  {
//    TDM_CONTAINER = 0,
//    TDM_CHECK,
    TDM_H_NAME = 0,
    TDM_H_NUMBER,
    TDM_H_SHOW,
    TDM_H_DELETE,
    TDM_H_MAXCOLUMN
  } TDispMngrHistoName;

  typedef enum
  {
    TDM_E_NAME = 0,
    TDM_E_SHOW,
    TDM_E_DELETE,
    TDM_E_MAXCOLUMN
  } TDispMngrEventName;

//  typedef std::tuple< QWidget*,
//                      /*QCheckBox*,*/
//                      QLabel*,
//                      QLabel*,
//                      QPushButton*,
//                      QPushButton*
//                      > TDisplayMngrTuple;


  class DisplayManagerWidget : public QWidget
  {
    Q_OBJECT;

  public:

    DisplayManagerWidget( );

    void init( const std::vector< visimpl::EventWidget* >* eventData,
               const std::vector< visimpl::HistogramWidget* >* histData );

    void refresh( );

    void dirtyEvents( void );
    void dirtyHistograms( void );

  signals:

    void hiddenRemovedEvent( unsigned int, bool );
    void hiddenRemovedHistogram( unsigned int, bool );

  protected slots:

    void eventCellClicked( int, int );
    void histoCellClicked( int, int );

    void hideEvent( int i );
    void removeEvent( int i );

  protected:

    void refreshEvents( void );
    void refreshHistograms( void );

    QTableWidget* _eventTable;
    QTableWidget* _histoTable;

    const std::vector< visimpl::EventWidget* >* _eventData;
    const std::vector< visimpl::HistogramWidget* >* _histData;

//    std::vector< TDisplayMngrTuple > _events;
//    std::vector< TDisplayMngrTuple > _histograms;

//    QGridLayout* _eventsLayout;
//    QGridLayout* _histogramsLayout;

    bool _dirtyFlagEvents;
    bool _dirtyFlagHistograms;
  };


}

#endif /* DISPLAYMANAGER_H_ */
