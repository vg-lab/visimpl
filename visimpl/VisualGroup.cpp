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

  VisualGroup::VisualGroup(
    const std::shared_ptr< plab::ICamera >& camera ,
    const std::shared_ptr< reto::ClippingPlane >& leftPlane ,
    const std::shared_ptr< reto::ClippingPlane >& rightPlane ,
    const std::shared_ptr< plab::Renderer >& renderer ,
    bool enableClipping )
    : _idx( _counter++ )
    , _name( "" )
    , _cluster( std::make_shared< plab::Cluster< NeuronParticle >>( ))
    , _model( std::make_shared< StaticGradientModel >(
      camera , leftPlane , rightPlane , TSizeFunction( ) , TColorVec( ) ,
      true , enableClipping , 0.0f, 1.0f ))
    , _active( true )
  {
    _cluster->setModel( _model );
    _cluster->setRenderer( renderer );
  }

  VisualGroup::VisualGroup(
    const std::string& name ,
    const std::shared_ptr< plab::ICamera >& camera ,
    const std::shared_ptr< reto::ClippingPlane >& leftPlane ,
    const std::shared_ptr< reto::ClippingPlane >& rightPlane ,
    const std::shared_ptr< plab::Renderer >& renderer ,
    bool enableClipping )
    : _idx( _counter++ )
    , _name( name )
    , _cluster( std::make_shared< plab::Cluster< NeuronParticle >>( ))
    , _model( std::make_shared< StaticGradientModel >(
      camera , leftPlane , rightPlane , TSizeFunction( ) ,
      TColorVec( ) , true , enableClipping , 0.0f, 1.5f ))
    , _active( true )
  {
    TColorVec vec;
    vec.emplace_back( 0.0f , glm::vec4( 1.0f , 0.0f , 0.0f , 1.0f ));
    vec.emplace_back( 1.0f , glm::vec4( 0.4f , 0.0f , 0.0f , 1.0f ));
    _model->setGradient( vec );

    _cluster->setModel( _model );
    _cluster->setRenderer( renderer );
  }

  VisualGroup::~VisualGroup( )
  {
    _cluster->setModel( _model );
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

  const std::vector< uint32_t >& VisualGroup::getGids( ) const
  {
    return _gids;
  }

  bool VisualGroup::active( void ) const
  {
    return _active;
  }

  void VisualGroup::active( bool state )
  {
    _active = state;
  }

  void VisualGroup::setParticles( const std::vector< uint32_t >& gids ,
                                  const std::vector< NeuronParticle >& particles )
  {
    _gids = gids;
    _cluster->setParticles( particles );
  }

  void
  VisualGroup::setRenderer( const std::shared_ptr< plab::Renderer >& renderer )
  {
    _cluster->setRenderer( renderer );
  }

  const std::shared_ptr< plab::Cluster< NeuronParticle >>
  VisualGroup::getCluster( ) const
  {
    return _cluster;
  }

  const std::shared_ptr< StaticGradientModel > VisualGroup::getModel( ) const
  {
    return _model;
  }

  void VisualGroup::colorMapping( const TTransferFunction& colors )
  {
    TColorVec gradient;

    auto insertColor = [ &gradient ]( const TTFColor& c )
    {
      glm::vec4 gColor( c.second.red( ) * invRGBInt ,
                        c.second.green( ) * invRGBInt ,
                        c.second.blue( ) * invRGBInt ,
                        c.second.alpha( ) * invRGBInt );

      gradient.emplace_back( c.first , gColor );
    };
    std::for_each( colors.cbegin( ) , colors.cend( ) , insertColor );

    _model->setGradient( gradient );
  }

  TTransferFunction VisualGroup::colorMapping( ) const
  {
    TTransferFunction result;

    for ( const auto& pair: _model->getGradient( ))
    {
      auto c = pair.second;
      QColor color( c.r * 255 , c.g * 255 , c.b * 255 , c.a * 255 );
      result.push_back( std::make_pair( pair.first , color ));
    }

    return result;
  }

  void VisualGroup::sizeFunction( const TSizeFunction& sizes )
  {
    _model->setParticleSize( sizes );
  }

  TSizeFunction VisualGroup::sizeFunction( ) const
  {
    return _model->getParticleSize( );
  }
}
