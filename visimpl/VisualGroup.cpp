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

// Visimpl
#include "VisualGroup.h"

namespace visimpl
{
  constexpr float invRGBInt = 1.0f / 255;

  unsigned int VisualGroup::_counter = 0;

  VisualGroup::VisualGroup( )
  : _idx( _counter++ )
  , _name( "" )
  , _cluster( nullptr )
  , _model( nullptr )
  , _source( nullptr )
  , _active( false )
  , _cached( false )
  , _dirty( false )
  , _custom( false )
  { }

  VisualGroup::VisualGroup( const std::string& name )
  : _idx( _counter++ )
  , _name( name )
  , _cluster( nullptr )
  , _model( nullptr )
  , _source( nullptr )
  , _active( false )
  , _cached( false )
  , _dirty( false )
  , _custom( false )
  { }

  VisualGroup::~VisualGroup( )
  {
    if( _cluster )
      delete _cluster;

    if( _source )
      delete _source;

    if( _model )
      delete _model;
  }

  unsigned int VisualGroup::id( void )
  {
    return _idx;
  }

  void VisualGroup::name( const std::string& name_ )
  {
    _name = name_;
  }

  const std::string& VisualGroup::name( void ) const
  {
    return _name;
  }

  void VisualGroup::gids( const GIDUSet& gids_ )
  {
    _gids = gids_;
  }

  const GIDUSet& VisualGroup::gids( void ) const
  {
    return _gids;
  }

  bool VisualGroup::active( void ) const
  {
    return _active;
  }

  void VisualGroup::active( bool state, bool updateSourceState )
  {
    _active = state;

    if( updateSourceState )
      _source->active( state );
  }

  void VisualGroup::cached( bool state )
  {
    _cached = state;
  }

  bool VisualGroup::cached( void ) const
  {
    return _cached;
  }

  void VisualGroup::dirty( bool state )
  {
    _dirty = state;
  }

  bool VisualGroup::dirty( void ) const
  {
    return _dirty;
  }

  void VisualGroup::custom( bool state )
  {
    _custom = state;
  }

  bool VisualGroup::custom( void ) const
  {
    return _custom;
  }

  void VisualGroup::cluster( prefr::Cluster* cluster_ )
  {
    assert( cluster_ );

    _cluster = cluster_;
  }

  prefr::Cluster* VisualGroup::cluster( void ) const
  {
    return _cluster;
  }

  void VisualGroup::model( prefr::Model* model_ )
  {
    assert( model_ );

    _model = model_;
  }

  prefr::Model* VisualGroup::model( void ) const
  {
    return _model;
  }

  void VisualGroup::source( SourceMultiPosition* source_ )
  {
    assert( source_ );

    _source = source_;
  }

  SourceMultiPosition* VisualGroup::source( void ) const
  {
    return _source;
  }

  QColor VisualGroup::color( void ) const
  {
    return _color;
  }

  void VisualGroup::colorMapping( const TTransferFunction& colors )
   {

     prefr::vectortvec4 gcolors;

     for( auto c : colors )
     {
       glm::vec4 gColor( c.second.red( ) * invRGBInt,
                         c.second.green( ) * invRGBInt,
                         c.second.blue( ) * invRGBInt,
                         c.second.alpha( ) * invRGBInt );

       gcolors.Insert( c.first, gColor );
     }

     _color = colors[ 0 ].second;
     _model->color = gcolors;

   }

  TTransferFunction VisualGroup::colorMapping( void ) const
   {
     TTransferFunction result;

     prefr::vectortvec4 colors = _model->color;

     auto timeValue = _model->color.times.begin( );
     for( auto c : _model->color.values )
     {
       QColor color( c.r * 255, c.g * 255, c.b * 255, c.a * 255 );
       result.push_back( std::make_pair( *timeValue, color ));

       ++timeValue;
     }


     return result;
   }

   void VisualGroup::sizeFunction( const TSizeFunction& sizes )
   {

     utils::InterpolationSet< float > newSize;
     for( auto s : sizes )
     {
       newSize.Insert( s.first, s.second );
     }
     _model->size = newSize;

   }

   TSizeFunction VisualGroup::sizeFunction( void ) const
   {
     TSizeFunction result;
     auto sizeValue = _model->size.values.begin( );
     for( auto s : _model->size.times)
     {
       result.push_back( std::make_pair( s, *sizeValue ));
       ++sizeValue;
     }

     return result;
   }
}
