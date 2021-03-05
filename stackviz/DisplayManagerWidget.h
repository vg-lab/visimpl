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

#ifndef DISPLAYMANAGERWIDGET_H_
#define DISPLAYMANAGERWIDGET_H_

// Sumrice
#include <sumrice/sumrice.h>

class QGridLayout;
class QWidget;
class QLabel;
class QPushButton;

namespace stackviz
{

  typedef enum
  {
    TDM_E_CONTAINER = 0,
    TDM_E_NAME,
    TDM_E_SHOW,
    TDM_E_DELETE,
//    TDM_E_LINE,
    TDM_E_MAXCOLUMN
  } TDispMngrEventName;

  typedef std::tuple< QWidget*,
                      QLabel*,
                      QPushButton*,
                      QPushButton*
//                      QFrame*
                      > TDisplayEventTuple;

  typedef enum
  {
    TDM_H_CONTAINER = 0,
    TDM_H_NAME,
    TDM_H_NUMBER,
    TDM_H_SHOW,
    TDM_H_DELETE,
    TDM_H_MAXCOLUMN
  } TDispMngrHistoName;

  typedef std::tuple< QWidget*,
                      QLabel*,
                      QLabel*,
                      QPushButton*,
                      QPushButton*
                      > TDisplayHistogramTuple;


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

    void clearWidgets( void );

  signals:

    void eventVisibilityChanged( unsigned int, bool );
    void removeEvent( unsigned int );

    void subsetVisibilityChanged( unsigned int, bool );
    void removeHistogram( unsigned int );

  protected slots:

    void hideEventClicked( );
    void deleteEventClicked( );

    void close( void );

    void hideHistoClicked( );
    void deleteHistoClicked( );

  protected:

    void clearEventWidgets( void );
    void clearHistogramWidgets( void );

    void refreshEvents( void );
    void refreshHistograms( void );

//    QTableWidget* _eventTable;
//    QTableWidget* _histoTable;

    const std::vector< visimpl::EventWidget* >* _eventData;
    const std::vector< visimpl::HistogramWidget* >* _histData;

//    std::vector< visimpl::EventWidget* > _availableEvents;
//    std::vector< visimpl::HistogramWidget* > _availableHistograms;

    std::vector< TDisplayEventTuple > _events;
    std::vector< TDisplayHistogramTuple > _histograms;

    QGridLayout* _eventsLayout;
    QGridLayout* _histogramsLayout;

    bool _dirtyFlagEvents;
    bool _dirtyFlagHistograms;
  };


}

#endif /* DISPLAYMANAGERWIDGET_H_ */
