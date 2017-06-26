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
//  : _eventTable( nullptr )
//  , _histoTable( nullptr )
  : _eventData( nullptr )
  , _histData( nullptr )
  , _eventsLayout( nullptr )
  , _histogramsLayout( nullptr )
  , _dirtyFlagEvents( true )
  , _dirtyFlagHistograms( true )
  {

  }

  void DisplayManagerWidget::init(  const std::vector< visimpl::EventWidget* >* eventData,
                                    const std::vector< visimpl::HistogramWidget* >* histData )
  {

    setMinimumWidth( 500 );

    setWindowTitle( "Visibility manager" );

    _eventData = eventData;
    _histData = histData;

//    unsigned int eventNumber = _eventData->size( );
//    unsigned int histNumber = _histData->size( );

//    _eventTable = new QTableWidget( eventNumber, TDM_E_MAXCOLUMN );
//    _histoTable = new QTableWidget( histNumber, TDM_H_MAXCOLUMN );
//
//    _eventTable->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
//    _histoTable->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn );

    QStringList eventHeaders = { "Name", "Show", "Delete" };
    QStringList histoHeaders = { "Name", "Size", "Show", "Delete" };
//    _eventTable->setHorizontalHeaderLabels( eventHeaders );
//    _histoTable->setHorizontalHeaderLabels( histoHeaders );

    QGridLayout* globalLayout = new QGridLayout( );


    // Events
    _eventsLayout = new QGridLayout( );

    QWidget* eventScrollContainer = new QWidget( );
    eventScrollContainer->setLayout( _eventsLayout );

    QScrollArea* eventScroll = new QScrollArea( );
    eventScroll->setWidget( eventScrollContainer );
    eventScroll->setWidgetResizable( true );
    eventScroll->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );

    QGroupBox* eventGroup = new QGroupBox( );
    eventGroup->setTitle( "Events" );
    eventGroup->setMinimumHeight( 300 );
    eventGroup->setMaximumHeight( 300 );
    eventGroup->setLayout( new QVBoxLayout( ));

    QGridLayout* headerEventLayout = new QGridLayout( );
    headerEventLayout->addWidget( new QLabel( "Name" ), 0, 0, 1, 2);
    headerEventLayout->addWidget( new QLabel( "Visible" ), 0, 2, 1, 1);
    headerEventLayout->addWidget( new QLabel( "Delete" ), 0, 3, 1, 1);

    QWidget* eventHeader = new QWidget( );
    eventHeader->setLayout( headerEventLayout );

    eventGroup->layout( )->addWidget( eventHeader );
    eventGroup->layout( )->addWidget( eventScroll );

    // Histograms
    _histogramsLayout = new QGridLayout( );

    QWidget* histoScrollContainer = new QWidget( );
    histoScrollContainer->setLayout( _histogramsLayout );

    QScrollArea* histoScroll = new QScrollArea( );
    histoScroll->setWidget( histoScrollContainer );
    histoScroll->setWidgetResizable( true );
    histoScroll->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );

    QGroupBox* histGroup = new QGroupBox( );
    histGroup->setTitle( "Histograms" );
    histGroup->setMinimumHeight( 300 );
    histGroup->setMaximumHeight( 300 );
    histGroup->setLayout(  new QVBoxLayout( ) );

    QGridLayout* headerHistoLayout = new QGridLayout( );
    headerHistoLayout->addWidget( new QLabel( "Name" ), 0, 0, 1, 2);
    headerHistoLayout->addWidget( new QLabel( "Size" ), 0, 2, 1, 1);
    headerHistoLayout->addWidget( new QLabel( "Visible" ), 0, 3, 1, 1);
    headerHistoLayout->addWidget( new QLabel( "Delete" ), 0, 4, 1, 1);

    QWidget* histoHeader = new QWidget( );
    histoHeader->setLayout( headerHistoLayout );

    histGroup->layout( )->addWidget( histoHeader );
    histGroup->layout( )->addWidget( histoScroll );

    QPushButton* closeButton = new QPushButton( "Close", this );
    connect( closeButton, SIGNAL( clicked( )), this, SLOT( close( )));

    globalLayout->addWidget( eventGroup, 0, 0, 5, 5 );
    globalLayout->addWidget( histGroup, 5, 0, 5, 5 );
    globalLayout->addWidget( closeButton, 10, 2, 1, 1 );

    this->setLayout( globalLayout );

//    connect( _eventTable, SIGNAL( cellClicked( int, int )),
//             this, SLOT( eventCellClicked( int, int )));
//
//    connect( _histoTable, SIGNAL( cellClicked( int, int )),
//             this, SLOT( histoCellClicked( int, int )));
  }

  void DisplayManagerWidget::close( void )
  {
//    setVisible( false );
    hide( );
  }

  void DisplayManagerWidget::dirtyEvents( void )
  {
    _dirtyFlagEvents = true;
  }

  void DisplayManagerWidget::dirtyHistograms( void )
  {
    _dirtyFlagHistograms = true;
  }

  void DisplayManagerWidget::clearWidgets( void )
  {
    clearEventWidgets( );

    clearHistogramWidgets( );
  }

  void DisplayManagerWidget::clearEventWidgets( void )
  {
    for( auto e : _events )
    {
      auto container = std::get< TDM_E_CONTAINER >( e );
      _eventsLayout->removeWidget( container );

      delete container;

//      auto layout = container->layout( );

//      layout->removeWidget( std::get< TDM_E_NAME >( e ));
//      layout->removeWidget( std::get< TDM_E_SHOW >( e ));
//      layout->removeWidget( std::get< TDM_E_DELETE >( e ));
//
//      delete std::get< TDM_E_NAME >( e );
//      delete std::get< TDM_E_SHOW >( e );
//      delete std::get< TDM_E_DELETE >( e );

    }

    QLayoutItem* item;
    while(( item = _eventsLayout->takeAt( 0 )) != nullptr )
    {
      delete item->widget( );
      delete item;
    }

    _events.clear( );
  }

  void DisplayManagerWidget::clearHistogramWidgets( void )
  {
    for( auto e : _histograms)
    {
      auto container = std::get< TDM_H_CONTAINER >( e );
      _histogramsLayout->removeWidget( container );

      delete container;
    }
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

    clearEventWidgets( );

//    unsigned int eventNumber = _eventData->size( );

//    _eventTable->resize( eventNumber, TDM_E_MAXCOLUMN );

    unsigned int row = 0;
    for( const auto& event : _availableEvents )
    {

      TDisplayEventTuple pointers;

      QWidget* container = new QWidget( );
      // Fill name
      QGridLayout* contLayout = new QGridLayout( );
      container->setLayout( contLayout );

      QLabel* nameLabel = new QLabel( tr( event->name( ).c_str( )), container);
      QPushButton* hideButton = new QPushButton( container );
      hideButton->setIcon( QIcon( QPixmap( ":icons/show.png" )));
      hideButton->setCheckable( true );
      hideButton->setChecked( true );
      hideButton->setWhatsThis( "Click to show/hide the row in main view." );

      QPushButton* deleteButton = new QPushButton( container );
      deleteButton->setIcon( QIcon( QPixmap( ":icons/trash.png" )));

      contLayout->addWidget( nameLabel, row, 0, 1, 2 );
      contLayout->addWidget( hideButton, row, 2, 1, 1 );
      contLayout->addWidget( deleteButton, row, 3, 1, 1 );

      _eventsLayout->addWidget( container );

      if( row < _availableEvents.size( ) - 1 )
      {
        QFrame* line = new QFrame( container );
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);

        _eventsLayout->addWidget( line );
      }

      connect( hideButton, SIGNAL( clicked( )),
               this, SLOT( hideEventClicked( )));

      connect( deleteButton, SIGNAL( clicked( )),
               this, SLOT( deleteEventClicked( )));


      _events.push_back( std::make_tuple( container, nameLabel, hideButton,
                                          deleteButton ));

//      connect( deleteButton)

//      QTableWidgetItem* item = new QTableWidgetItem( tr( event->name( ).c_str( )));
//      _eventTable->setItem( row, TDM_E_NAME, item );

//      item = new QTableWidgetItem( );
//      item->setIcon( QIcon( QPixmap( ":icons/hide.png" )));
//      _eventTable->setItem( row, TDM_E_SHOW, item );


//      item = new QTableWidgetItem( );
//      item->setIcon( QIcon( QPixmap( ":icons/trash.png" )));
//      _eventTable->setItem( row, TDM_E_DELETE, item );

      ++row;
    }

    _dirtyFlagEvents = false;
  }

  void DisplayManagerWidget::refreshHistograms( void )
  {
    clearHistogramWidgets( );

    unsigned int row = 0;
    for( const auto& hist : _availableHistograms )
    {

      TDisplayEventTuple pointers;

      QWidget* container = new QWidget( );
      // Fill name
      QGridLayout* contLayout = new QGridLayout( );
      container->setLayout( contLayout );

      QLabel* nameLabel = new QLabel( tr( hist->name( ).c_str( )), container);

      QLabel* numberLabel =
          new QLabel( QString::number( hist->gidsSize( )), container);

      QPushButton* hideButton = new QPushButton( container );
      hideButton->setIcon( QIcon( QPixmap( ":icons/show.png" )));
      hideButton->setCheckable( true );
      hideButton->setChecked( true );
      hideButton->setWhatsThis( "Click to show/hide the row in main view." );

      QPushButton* deleteButton = new QPushButton( container );
      deleteButton->setIcon( QIcon( QPixmap( ":icons/trash.png" )));

      contLayout->addWidget( nameLabel, row, 0, 1, 2 );
      contLayout->addWidget( numberLabel, row, 2, 1, 1 );
      contLayout->addWidget( hideButton, row, 3, 1, 1 );
      contLayout->addWidget( deleteButton, row, 4, 1, 1 );

      _histogramsLayout->addWidget( container );

      if( row < _eventData->size( ) - 1 )
      {
        QFrame* line = new QFrame( container );
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);

        _histogramsLayout->addWidget( line );
      }

      connect( hideButton, SIGNAL( clicked( )),
               this, SLOT( hideEventClicked( )));

      _histograms.push_back( std::make_tuple( container, nameLabel, numberLabel,
                                              hideButton, deleteButton ));
      ++row;
   }
//    _histoTable->clearWidgets( );
//
//    unsigned int histNumber = _histData->size( );
//
//    _histoTable->resize( histNumber, TDM_E_MAXCOLUMN );
//
//    unsigned int row = 0;
//    for( const auto& hist : *_histData )
//    {
//
//      // Fill name
//      QTableWidgetItem* item = new QTableWidgetItem( tr( hist->name( ).c_str( )));
//      _histoTable->setItem( row, TDM_H_NAME, item );
//
//      item = new QTableWidgetItem( QString::number( hist->gidsSize( )));
//      _histoTable->setItem( row, TDM_H_NUMBER, item );
//
//      item = new QTableWidgetItem( );
//      item->setIcon( QIcon( QPixmap( ":icons/hide.png" )));
//      _histoTable->setItem( row, TDM_H_SHOW, item );
//
//      item = new QTableWidgetItem( );
//      item->setIcon( QIcon( QPixmap( ":icons/trash.png" )));
//      _histoTable->setItem( row, TDM_H_DELETE, item );
//
//      ++row;
//    }
//
    _dirtyFlagHistograms = false;

  } // refreshHistograms

  void DisplayManagerWidget::eventCellClicked( int, int )
  {

  }

  void DisplayManagerWidget::histoCellClicked( int, int )
  {

  }

  void DisplayManagerWidget::hideRow( int )
  {

  }

  void DisplayManagerWidget::removeRow( int )
  {

  }

  void DisplayManagerWidget::hideEventClicked( )
  {
    auto author = dynamic_cast< QWidget* >( sender( ));

    unsigned int counter = 0;
    for( auto t : _events )
    {
      auto container = std::get< TDM_E_CONTAINER >( t );

      if( author->parent( ) == container )
      {
        auto button = std::get< TDM_E_SHOW >( t );

        bool hidden = button->isChecked( );

        button->setIcon( QIcon( hidden ? ":icons/show.png" : ":icons/hide.png" ));

        emit( hiddenRemovedEvent( counter, true ));

        break;
      }

      ++counter;
    }

  }

  void DisplayManagerWidget::deleteEventClicked( )
  {
    auto author = dynamic_cast< QWidget* >( sender( ));

    unsigned int counter = 0;
    for( auto t : _events )
    {
      auto container = std::get< TDM_E_CONTAINER >( t );

      if( author->parent( ) == container )
      {

//        _eventsLayout->removeWidget( container );
//        _eventsLayout->removeWidget( std::get< TDM_E_LINE >( t ));

//        delete container;

        _availableEvents.erase( _availableEvents.begin( ) + counter );

        _dirtyFlagEvents = true;
        refreshEvents( );

        emit( hiddenRemovedEvent( counter, false ));

        break;
      }

     ++counter;
    }

  }

  void hideHistoClicked( )
  {

  }

  void deleteHistoClicked( )
  {

  }

}


