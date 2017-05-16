/*
 * @file	VisualGroup.cpp
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es> 
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *					Do not distribute without further notice.
 */

#include "VisualGroup.h"

namespace visimpl
{

  float invRGBInt = 1.0f / 255;

  VisualGroup::VisualGroup( )
  : _model( nullptr )
  , _active( false )
  {

  }

  VisualGroup::~VisualGroup( )
  {

  }

  void VisualGroup::gids( const GIDUSet& gids_ )
  {
    _gids = gids_;
  }

  const GIDUSet& VisualGroup::gids( void ) const
  {
    return _gids;
  }

  void VisualGroup::model( prefr::Model* model_ )
  {
    assert( model_ );

    _model = model_;
  }

  prefr::Model* VisualGroup::model( void )
  {
    return _model;
  }

  bool VisualGroup::active( void )
  {
    return _active;
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

     _model->color = gcolors;

   }

  TTransferFunction VisualGroup::colorMapping( void )
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

   TSizeFunction VisualGroup::sizeFunction( void )
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



