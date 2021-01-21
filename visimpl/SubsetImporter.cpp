/*
 * Copyright (c) 2015-2020 GMRV/URJC.
 *
 * Authors: Sergio E. Galindo <sergio.galindo@urjc.es>
 *
 * This file is part of ViSimpl <https://github.com/gmrvvis/visimpl>
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
#include <QScrollArea>
#include <QCheckBox>
#include <QLabel>

namespace visimpl
{
  SubsetImporter::SubsetImporter( QWidget* parent_, Qt::WindowFlags f_ )
  : QDialog( parent_, f_ )
  , _subsetEventManager( nullptr )
  , _buttonAccept( nullptr )
  , _buttonCancel( nullptr )
  , _layoutSubsets( nullptr )
  {
    init( );

    setWindowIcon(QIcon(tr(":/icons/visimpl-icon-square.png")));
    setWindowTitle(tr("Import Subsets"));
  }

  void SubsetImporter::init( void )
  {
    QVBoxLayout* layoutUpper = new QVBoxLayout( );
    this->setLayout( layoutUpper );

    QGroupBox* gbSubsets = new QGroupBox( "Available subsets" );
    QVBoxLayout* layoutGroupBox = new QVBoxLayout( );
    gbSubsets->setLayout( layoutGroupBox );

    QWidget* header = new QWidget( );
    QGridLayout* layoutHeader = new QGridLayout( );
    header->setLayout( layoutHeader );
    QLabel* checkBoxHeader = new QLabel( tr( "Subset name" ));
    QLabel* labelHeader = new QLabel( tr( "# elements" ));

    layoutHeader->addWidget( checkBoxHeader, 0, 0, 1, 2 );
    layoutHeader->addWidget( labelHeader, 0, 2, 1, 1 );

    QScrollArea* scrollSubsets = new QScrollArea( );
    layoutGroupBox->addWidget( header );
    layoutGroupBox->addWidget( scrollSubsets );

    _layoutSubsets = new QVBoxLayout( );
    _layoutSubsets->setMargin( 0 );
    scrollSubsets->setLayout( _layoutSubsets );

    QWidget* foot = new QWidget( );
    QGridLayout* layoutBottom = new QGridLayout( );
    foot->setLayout( layoutBottom );

    _buttonAccept = new QPushButton( "Accept" );
    _buttonCancel = new QPushButton( "Cancel" );

    QFrame* line = new QFrame( );
    line->setFrameShape( QFrame::HLine );
    line->setFrameShadow( QFrame::Sunken );

    layoutBottom->addWidget( line, 0, 0, 1, 5 );
    layoutBottom->addWidget( _buttonCancel, 1, 1, 1, 1 );
    layoutBottom->addWidget( _buttonAccept, 1, 3, 1, 1 );

    layoutUpper->addWidget( gbSubsets );
    layoutUpper->addWidget( foot );

    connect( _buttonCancel, SIGNAL( clicked( void )), this, SLOT( reject()));
    connect( _buttonAccept, SIGNAL( clicked( void )), this, SLOT( accept( )));
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

      QWidget* container = new QWidget( );
      QGridLayout* layout = new QGridLayout( );
      QCheckBox* checkBox = new QCheckBox( subsetName.c_str( ));
      QLabel* label = new QLabel( QString::number( subset.size( )));

      auto row = std::make_tuple( container, layout, checkBox, label );
      _subsets.insert( std::make_pair( subsetName, row ));

      checkBox->setChecked( true );

      container->setLayout( layout );
      layout->addWidget( checkBox, 0, 0, 1, 2 );
      layout->addWidget( label, 0, 2, 1, 1 );

      _layoutSubsets->addWidget( container );
    };
    std::for_each(names.cbegin(), names.cend(), createSubsetWidgets);
  }

  void SubsetImporter::clear( )
  {
    auto removeSubsetWidgets = [this](std::pair<const std::string, tSubsetLine> &row)
    {
      auto container = std::get< sl_container >( row.second );
      _layoutSubsets->removeWidget( container );

      delete container;
    };
    std::for_each(_subsets.begin(), _subsets.end(), removeSubsetWidgets);

    _subsets.clear( );
  }

  const std::vector< std::string > SubsetImporter::selectedSubsets( ) const
  {
    std::vector< std::string > result;

    auto selectSubsetIfChecked = [&result, this](const std::pair<const std::string, tSubsetLine> &row)
    {
      if( std::get< sl_checkbox >( row.second )->isChecked( ))
        result.push_back( row.first );
    };
    std::for_each(_subsets.cbegin(), _subsets.cend(), selectSubsetIfChecked);

    return result;
  }
}


