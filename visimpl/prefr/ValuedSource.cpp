/*
 * SpineRetEmissionNode.cpp
 *
 *  Created on: 03/12/2014
 *      Author: sgalindo
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
  , _particlesLife( 0.0f )
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

  void ValuedSource::particlesLife( float life )
  {
    _particlesLife = life;
  }

  float ValuedSource::particlesLife( void )
  {
    return _particlesLife;
  }

}


