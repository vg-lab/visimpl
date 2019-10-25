/*
 * @file	SubsetManager.cpp
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es> 
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *					Do not distribute without further notice.
 */

#include "SubsetImporter.h"

#include <QGroupBox>
#include <QScrollArea>
#include <QCheckBox>
#include <QLabel>

namespace visimpl
{

  SubsetImporter::SubsetImporter( QWidget* parent_ )
  : QWidget( parent_ )
  , _subsetEventManager( nullptr )
  , _buttonAccept( nullptr )
  , _buttonCancel( nullptr )
  , _layoutSubsets( nullptr )
  {
    init( );
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

    connect( _buttonCancel, SIGNAL( clicked( void )),
             this, SLOT( closeDialog( void )));

    connect( _buttonAccept, SIGNAL( clicked( void )),
             this, SLOT( closeDialog( void )));
  }

  SubsetImporter::~SubsetImporter( )
  { }

  void SubsetImporter::closeDialog( void )
  {
    this->close( );

    if( sender( ) == _buttonAccept )
      emit clickedAccept( );
  }


  void SubsetImporter::reload( const simil::SubsetEventManager* subsetEventMngr )
  {
    _subsetEventManager = subsetEventMngr;

    if( !_subsetEventManager )
      return;

    clear( );

    for( auto subsetName : _subsetEventManager->subsetNames( ))
    {
      auto subset = _subsetEventManager->getSubset( subsetName );

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
    }
  }

  void SubsetImporter::clear( void )
  {
    for( auto row : _subsets )
    {
      auto container = std::get< sl_container >( row.second );
      _layoutSubsets->removeWidget( container );

      delete container;
    }

    _subsets.clear( );
  }

  const std::vector< std::string > SubsetImporter::selectedSubsets( void ) const
  {
    std::vector< std::string > result;

    for( auto subset : _subsets )
    {
      if( std::get< sl_checkbox >( subset.second )->isChecked( ))
        result.push_back( subset.first );
    }

    return result;
  }



}


