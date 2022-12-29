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

// ViSimpl
#include "SubsetImporter.h"

// Qt
#include <QGroupBox>
#include <QListWidget>
#include <QCheckBox>
#include <QLabel>

namespace visimpl
{
  SubsetImporter::SubsetImporter( QWidget* parent_, Qt::WindowFlags f_ )
  : QDialog( parent_, f_ )
  , _subsetEventManager( nullptr )
  , _buttonAccept( nullptr )
  , _buttonCancel( nullptr )
  , _selectGroups{nullptr}
  , _selectConnections{nullptr}
  {
    init( );

    setWindowIcon(QIcon(tr(":/visimpl.png")));
    setWindowTitle(tr("Import Subsets"));
    setMinimumSize(600, 400);
  }

  void SubsetImporter::init( void )
  {
    auto layoutUpper = new QVBoxLayout( );
    this->setLayout( layoutUpper );

    auto gbSubsets = new QGroupBox( tr("Available subsets") );
    auto layoutGroupBox = new QVBoxLayout( );
    gbSubsets->setLayout( layoutGroupBox );

    auto header = new QWidget( );
    auto layoutHeader = new QGridLayout( );
    header->setLayout( layoutHeader );
    auto checkBoxHeader = new QLabel( tr( "Subset name" ));
    auto labelHeader = new QLabel( tr( "# elements" ));

    layoutHeader->addWidget( checkBoxHeader, 0, 0, 1, 2 );
    layoutHeader->addWidget( labelHeader, 0, 2, 1, 1 );

    _listSubsets = new QListWidget( );
    layoutGroupBox->addWidget( header );
    layoutGroupBox->addWidget( _listSubsets );

    _listSubsets->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    _listSubsets->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAsNeeded);

    auto foot = new QWidget( );
    auto layoutBottom = new QGridLayout( );
    foot->setLayout( layoutBottom );

    _buttonAccept = new QPushButton( tr("Accept") );
    _buttonCancel = new QPushButton( tr("Cancel") );
    _selectGroups = new QPushButton(tr("Select groups"));
    _selectConnections = new QPushButton(tr("Select connections"));
    _selectAll = new QPushButton(tr("Select all"));

    auto line = new QFrame( );
    line->setFrameShape( QFrame::HLine );
    line->setFrameShadow( QFrame::Sunken );

    layoutBottom->addWidget(_selectGroups, 0,0,1,2);
    layoutBottom->addWidget(_selectConnections, 0,2,1,2);
    layoutBottom->addWidget(_selectAll, 0,4,1,2);
    layoutBottom->addWidget( line, 1, 0, 1, 6 );
    layoutBottom->addWidget( _buttonCancel, 2, 1, 1, 2 );
    layoutBottom->addWidget( _buttonAccept, 2, 3, 1, 2 );

    layoutUpper->addWidget( gbSubsets );
    layoutUpper->addWidget( foot );

    connect( _buttonCancel, SIGNAL( clicked( void )), this, SLOT( reject()));
    connect( _buttonAccept, SIGNAL( clicked( void )), this, SLOT( accept( )));
    connect( _selectGroups, SIGNAL( clicked( void )), this, SLOT( selectSubsets()));
    connect( _selectConnections, SIGNAL( clicked( void )), this, SLOT( selectSubsets( )));
    connect( _selectAll, SIGNAL( clicked( void )), this, SLOT( selectSubsets( )));

  }

  void SubsetImporter::reload( const simil::SubsetEventManager* subsetEventMngr )
  {
    _subsetEventManager = subsetEventMngr;

    if( !_subsetEventManager )
      return;

    clear();

    const auto names = _subsetEventManager->subsetNames( );

    auto createSubsetWidgets = [this](const std::string &subsetName)
    {
      const auto subset = _subsetEventManager->getSubset( subsetName );

      auto container = new QWidget( );
      auto layout = new QGridLayout( );
      auto checkBox = new QCheckBox( subsetName.c_str( ));
      auto label = new QLabel( QString::number( subset.size( )));

      const auto row = std::make_tuple( container, layout, checkBox, label );
      _subsets.insert( std::make_pair( subsetName, row ));

      checkBox->setChecked( true );

      container->setLayout( layout );
      container->setFixedHeight(40);
      layout->addWidget( checkBox, 0, 0, 1, 2 );
      layout->addWidget( label, 0, 2, 1, 1 );

      auto item = new QListWidgetItem(_listSubsets, Qt::UserRole+1);

      _listSubsets->addItem(item);
      item->setSizeHint(QSize(0, 35));
      _listSubsets->setItemWidget(item, container);
    };
    std::for_each(names.cbegin(), names.cend(), createSubsetWidgets);
  }

  void SubsetImporter::clear( )
  {
    _listSubsets->clear();
    _subsets.clear( );
  }

  const std::vector< std::string > SubsetImporter::selectedSubsets( ) const
  {
    std::vector< std::string > result;

    auto selectSubsetIfChecked = [&result](const std::pair<const std::string, tSubsetLine> &row)
    {
      if( std::get< sl_checkbox >( row.second )->isChecked( ))
        result.push_back( row.first );
    };
    std::for_each(_subsets.cbegin(), _subsets.cend(), selectSubsetIfChecked);

    return result;
  }

  void SubsetImporter::selectSubsets()
  {
    auto button = qobject_cast<QPushButton *>(sender());

    if(button == _selectGroups)
    {
      for(auto subset: _subsets)
      {
        auto isGroup = subset.first.find("group") != std::string::npos;
        std::get< sl_checkbox >( subset.second )->setChecked(isGroup);
      }
    }
    else
    {
      if(button == _selectConnections)
      {
        for(auto subset: _subsets)
        {
          auto isConnection = subset.first.find("connection") != std::string::npos;
          std::get< sl_checkbox >( subset.second )->setChecked(isConnection);
        }
      }
      else
      {
        for(auto subset: _subsets)
        {
          std::get< sl_checkbox >( subset.second )->setChecked(true);
        }
      }
    }
  }

}
