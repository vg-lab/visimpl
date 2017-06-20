/*
 * @file	DisplayManager.cpp
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es> 
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *					Do not distribute without further notice.
 */

#include "DisplayManager.h"

#include <QGroupBox>

namespace stackviz
{

  DisplayManagerWidget::DisplayManagerWidget( )
  : _eventTable( nullptr )
  , _histoTable( nullptr )
  , _eventData( nullptr )
  , _histData( nullptr )
//  , _eventsLayout( nullptr )
//  , _histogramsLayout( nullptr )
  , _dirtyFlagEvents( true )
  , _dirtyFlagHistograms( true )
  {

  }

  void DisplayManagerWidget::init(  const std::vector< visimpl::EventWidget* >* eventData,
                                    const std::vector< visimpl::HistogramWidget* >* histData )
  {

    setMinimumWidth( 500 );

    _eventData = eventData;
    _histData = histData;

    unsigned int eventNumber = _eventData->size( );
    unsigned int histNumber = _histData->size( );

    _eventTable = new QTableWidget( eventNumber, TDM_E_MAXCOLUMN );
    _histoTable = new QTableWidget( histNumber, TDM_H_MAXCOLUMN );

    _eventTable->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
    _histoTable->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn );

    QStringList eventHeaders = { "Name", "Show", "Delete" };
    QStringList histoHeaders = { "Name", "Size", "Show", "Delete" };
    _eventTable->setHorizontalHeaderLabels( eventHeaders );
    _histoTable->setHorizontalHeaderLabels( histoHeaders );

    QGridLayout* globalLayout = new QGridLayout( );

    QGroupBox* eventGroup = new QGroupBox( );
    eventGroup->setTitle( "Events" );

    QGroupBox* histGroup = new QGroupBox( );
    histGroup->setTitle( "Histograms" );

    QVBoxLayout* eventLayout = new QVBoxLayout( );
    eventLayout->addWidget( _eventTable );
    eventGroup->setLayout( eventLayout );

    QVBoxLayout* histLayout = new QVBoxLayout( );
    histLayout->addWidget( _histoTable );
    histGroup->setLayout( histLayout );

    globalLayout->addWidget( eventGroup );
    globalLayout->addWidget( histGroup );

    this->setLayout( globalLayout );

    connect( _eventTable, SIGNAL( cellClicked( int, int )),
             this, SLOT( eventCellClicked( int, int )));

    connect( _histoTable, SIGNAL( cellClicked( int, int )),
             this, SLOT( histoCellClicked( int, int )));
  }

  void DisplayManagerWidget::dirtyEvents( void )
  {
    _dirtyFlagEvents = true;
  }

  void DisplayManagerWidget::dirtyHistograms( void )
  {
    _dirtyFlagHistograms = true;
  }

  void DisplayManagerWidget::refresh( )
  {
    if( _dirtyFlagEvents )
      refreshEvents( );

    if( _dirtyFlagHistograms )
      refreshHistograms( );

  }

  void DisplayManagerWidget::refreshEvents( void )
  {
    _eventTable->clear( );

    unsigned int eventNumber = _eventData->size( );

    _eventTable->resize( eventNumber, TDM_E_MAXCOLUMN );

    unsigned int row = 0;
    for( const auto& event : *_eventData )
    {

      // Fill name
      QTableWidgetItem* item = new QTableWidgetItem( tr( event->name( ).c_str( )));
      _eventTable->setItem( row, TDM_E_NAME, item );

      item = new QTableWidgetItem( );
      item->setIcon( QIcon( QPixmap( ":icons/hide.png" )));
      _eventTable->setItem( row, TDM_E_SHOW, item );


      item = new QTableWidgetItem( );
      item->setIcon( QIcon( QPixmap( ":icons/trash.png" )));
      _eventTable->setItem( row, TDM_E_DELETE, item );

      ++row;
    }

    _dirtyFlagEvents = false;
  }

  void DisplayManagerWidget::refreshHistograms( void )
  {
    _histoTable->clear( );

    unsigned int histNumber = _histData->size( );

    _histoTable->resize( histNumber, TDM_E_MAXCOLUMN );

    unsigned int row = 0;
    for( const auto& hist : *_histData )
    {

      // Fill name
      QTableWidgetItem* item = new QTableWidgetItem( tr( hist->name( ).c_str( )));
      _histoTable->setItem( row, TDM_H_NAME, item );

      item = new QTableWidgetItem( QString::number( hist->gidsSize( )));
      _histoTable->setItem( row, TDM_H_NUMBER, item );

      item = new QTableWidgetItem( );
      item->setIcon( QIcon( QPixmap( ":icons/hide.png" )));
      _histoTable->setItem( row, TDM_H_SHOW, item );

      item = new QTableWidgetItem( );
      item->setIcon( QIcon( QPixmap( ":icons/trash.png" )));
      _histoTable->setItem( row, TDM_H_DELETE, item );

      ++row;
    }

    _dirtyFlagHistograms = false;

  } // refreshHistograms

  void DisplayManagerWidget::eventCellClicked( int, int )
  {

  }

  void DisplayManagerWidget::histoCellClicked( int, int )
  {

  }

  void DisplayManagerWidget::hideEvent( int )
  {

  }

  void DisplayManagerWidget::removeEvent( int )
  {

  }

}


