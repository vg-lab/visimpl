/*
 * @file  SelectionManagerWidget.cpp
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */

#include "SelectionManagerWidget.h"

#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>


namespace visimpl
{

  SelectionManagerWidget::SelectionManagerWidget( QWidget* parent_ )
  : QWidget( parent_ )
  , _listViewAvailable( nullptr )
  , _listViewSelected( nullptr )
  , _modelAvailable( nullptr )
  , _modelSelected( nullptr )
  , _buttonAddToSelection( nullptr )
  , _buttonRemoveFromSelection( nullptr )
  {
    init( );
  }

  SelectionManagerWidget::~SelectionManagerWidget( void )
  {

  }

  void SelectionManagerWidget::init( void )
  {

    QVBoxLayout* layoutTop = new QVBoxLayout( );

    this->setLayout( layoutTop );

    QTabWidget* tabWidget = new QTabWidget( );
    QWidget* containerFoot = new QWidget( );
    QGridLayout* layoutFoot = new QGridLayout( );
    containerFoot->setLayout( layoutFoot );

    layoutTop->addWidget( tabWidget );
    layoutTop->addWidget( containerFoot );

    QWidget* containerSelection = new QWidget( );
    QWidget* containerExport = new QWidget( );

    tabWidget->addTab( containerSelection, "Selection" );
    tabWidget->addTab( containerExport, "Export" );

    QGridLayout* layoutSelection = new QGridLayout( );
    containerSelection->setLayout( layoutSelection );

    _labelAvailable = new QLabel( "Available GIDs: (0)" );
    _labelSelection = new QLabel( "Selected GIDs: (0)" );

    layoutSelection->addWidget( _labelAvailable, 0, 0, 1, 1 );
    layoutSelection->addWidget( _labelSelection, 0, 2, 1, 1 );

    _listViewAvailable = new QListView( );
    _listViewAvailable->setSelectionMode( QAbstractItemView::ExtendedSelection );
    _listViewSelected = new QListView( );
    _listViewSelected->setSelectionMode( QAbstractItemView::ExtendedSelection );

    _modelAvailable = new QStandardItemModel( );
    _modelSelected = new QStandardItemModel( );

    _listViewAvailable->setModel( _modelAvailable );
    _listViewSelected->setModel( _modelSelected );

    _buttonAddToSelection = new QPushButton( "-->" );
    _buttonAddToSelection->setToolTip( tr( "Add to selected GIDs" ));

    _buttonRemoveFromSelection = new QPushButton( "<--" );
    _buttonRemoveFromSelection->setToolTip( tr( "Remove from selected GIDs" ));

    layoutSelection->addWidget( _listViewAvailable, 1, 0, 5, 1 );
    layoutSelection->addWidget( _buttonAddToSelection, 2, 1, 1, 1 );
    layoutSelection->addWidget( _buttonRemoveFromSelection, 4, 1, 1, 1 );
    layoutSelection->addWidget( _listViewSelected, 1, 2, 5, 1 );

    connect( _buttonAddToSelection, SIGNAL( clicked( void )),
             this, SLOT( addToSelected( void )));

    connect( _buttonRemoveFromSelection, SIGNAL( clicked( void )),
             this, SLOT( removeFromSelected( void )));

    // Foot

    QPushButton* buttonCancel = new QPushButton( "Cancel" );
    buttonCancel->setMaximumWidth( 100 );
    QPushButton* buttonAccept = new QPushButton( "Accept" );
    buttonAccept->setMaximumWidth( 100 );

    layoutFoot->addWidget( buttonCancel, 0, 0, 1, 1 );
    layoutFoot->addWidget( buttonAccept, 0, 2, 1, 1 );

    connect( buttonCancel, SIGNAL( clicked( void )),
             this, SLOT( cancelClicked( void )));
    connect( buttonAccept, SIGNAL( clicked( void )),
             this, SLOT( acceptClicked( void )));
  }

  void SelectionManagerWidget::setGIDs( const TGIDSet& all_,
                                        const TGIDUSet& selected_ )
  {
    _gidsAll.clear( );
    _gidsAll.insert( all_.begin( ), all_.end( ));

    _fillLists( );

    setSelected( selected_ );

  }

  const TGIDUSet& SelectionManagerWidget::selected( void ) const
  {
    return _gidsSelected;
  }

  void SelectionManagerWidget::clearSelection( void )
  {
    _gidsSelected.clear( );
    _gidsAvailable = _gidsAll;

    _reloadLists( );
  }

  void SelectionManagerWidget::setSelected( const TGIDUSet& selected_ )
  {
    if( selected_ == _gidsSelected && !_gidsAvailable.empty( ))
      return;

    _gidsSelected = selected_;

    _gidsAvailable.clear( );
    for( auto gid : _gidsAll )
    {
      if( _gidsSelected.find( gid ) == _gidsSelected.end( ))
        _gidsAvailable.insert( gid );
    }

    _reloadLists( );
  }

  void SelectionManagerWidget::_fillLists( void )
  {
    _modelAvailable->clear( );
    _modelSelected->clear( );

    unsigned int index = 0;
    GIDVec gids( _gidsAll.size( ));
    std::copy( _gidsAll.begin( ), _gidsAll.end( ), gids.begin( ));
    std::sort( gids.begin( ), gids.end( ));

    for( auto gid : gids )
    {
      QStandardItem* itemAvailable = new QStandardItem( );
      QVariant value = QVariant::fromValue( gid );

      itemAvailable->setData( value, Qt::DisplayRole );

      QStandardItem* itemSelected = new QStandardItem( );
      itemSelected->setData( value, Qt::DisplayRole );

      _modelAvailable->appendRow( itemAvailable );
      _modelSelected->appendRow( itemSelected );

      _gidIndex.insert( std::make_pair( gid, index ));

      ++index;
    }

  }

  void SelectionManagerWidget::_reloadLists( void )
  {

    for( auto gid : _gidsAll )
    {
      bool stateSelected = ( _gidsSelected.find( gid ) != _gidsSelected.end( ));

      auto gidIndex = _gidIndex.find( gid );
      assert( gidIndex != _gidIndex.end( ));

      unsigned int row = gidIndex->second;

      _listViewAvailable->setRowHidden( row, stateSelected );
      _listViewSelected->setRowHidden( row, !stateSelected );
    }
  }

  void SelectionManagerWidget::addToSelected( void )
  {

    auto selectedIndices =
        _listViewAvailable->selectionModel( )->selectedIndexes( );

    if( selectedIndices.size( ) <= 0 )
      return;

    for( auto index : selectedIndices )
    {
      if( index.row( ) < 0 )
        continue;

      auto item = _modelAvailable->itemFromIndex( index );

      bool ok;
      unsigned int gid = item->data( Qt::DisplayRole ).toUInt( &ok );

      _gidsAvailable.erase( gid );
      _gidsSelected.insert( gid );

      auto gidIndex = _gidIndex.find( gid );
      assert( gidIndex != _gidIndex.end( ));



      _listViewAvailable->setRowHidden( gidIndex->second, true );
      _listViewSelected->setRowHidden( gidIndex->second, false );

    }

    _listViewAvailable->selectionModel( )->clearSelection( );

  }

  void SelectionManagerWidget::removeFromSelected( void )
  {

    auto selectedIndices =
          _listViewSelected->selectionModel( )->selectedIndexes( );

    for( auto index : selectedIndices )
    {
      if( index.row( ) < 0 )
        continue;

      auto item = _modelSelected->itemFromIndex( index );
      unsigned int gid = item->data( Qt::DisplayRole ).toUInt( );

      _gidsSelected.erase( gid );
      _gidsAvailable.insert( gid );

      auto gidIndex = _gidIndex.find( gid );
      assert( gidIndex != _gidIndex.end( ));

      _listViewAvailable->setRowHidden( gidIndex->second, false );
      _listViewSelected->setRowHidden( gidIndex->second, true );

    }

    _listViewSelected->selectionModel( )->clearSelection( );

  }

  void SelectionManagerWidget::saveToFile( const std::string& ,//filePath,
                                           const std::string& ,//separator,
                                           const std::string& ,//prefix,
                                           const std::string& )//suffix)
  {

  }

  void SelectionManagerWidget::acceptClicked( void )
  {
    this->close( );

    emit selectionChanged( );
  }

  void SelectionManagerWidget::cancelClicked( void )
  {
    this->close( );
  }


}
