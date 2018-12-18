/*
 * @file  ValuedSource.cpp
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
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


