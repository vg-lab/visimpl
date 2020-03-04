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

#include "ValuedSource.h"

namespace prefr
{

  ValuedSource::ValuedSource( float emissionRate, const glm::vec3& position_,
                              const glm::vec4& color_, bool still_ )
  : Source( emissionRate, position_ )
  , _color( color_ )
  , _size ( 0.0f )
  , _still( still_ )
  , _particlesRelLife( 0.0f )
  {}

  ValuedSource::~ValuedSource()
  {}

  void ValuedSource::color( const glm::vec4& color_ )
  {
    _color = color_;
  }

  const glm::vec4& ValuedSource::color()
  {
    return _color;
  }

  void ValuedSource::still( bool still_ )
  {
    _still = still_;
  }

  bool ValuedSource::still()
  {
    return _still;
  }


  void ValuedSource::size( float size_ )
  {
    _size = size_;
  }

  float ValuedSource::size()
  {
    return _size;
  }

  void ValuedSource::_prepareParticles( void )
  {
    auto particlesRange = _cluster->particles( );

    _particlesToEmit.clear( );

    for( auto particle : particlesRange )
    {
      if( !particle.alive( ))
      {
        _particlesToEmit.push_back( particle.id( ));
        ++_lastFrameAliveParticles;
      }
    }

    _deadParticles.clear( );

    _emittedParticles += _lastFrameAliveParticles;
  }

  void ValuedSource::particlesRelLife( float relativeLife )
  {
    auto particlesRange = cluster( )->particles( );

    float lifeRange = cluster( )->model( )->lifeInterval( );

    float lifeValue = relativeLife * lifeRange + cluster( )->model( )->minLife( );

    for( auto particle : particlesRange )
    {
      particle.set_life( lifeValue );
    }

  }

  void ValuedSource::particlesLife( float life )
  {
    life = std::max( 0.0f, life );

    auto particlesRange = cluster( )->particles( );

    for( auto particle : particlesRange )
    {
      particle.set_life( life );
    }

  }

  bool ValuedSource::emits( void ) const
  {
    return true;
  }

}


