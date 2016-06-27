/*
 * SpineRetEmissionNode.cpp
 *
 *  Created on: 03/12/2014
 *      Author: sgalindo
 */

#include "DirectValuedEmissionNode.h"

namespace prefr
{

  DirectValuedEmissionNode::DirectValuedEmissionNode(const ParticleCollection& arrayParticles,
                                       glm::vec3 _position, glm::vec4 color,
                                       bool still)
  : PointEmissionNode( arrayParticles, _position )
  , color_( color )
  , size_ ( 0.0f )
  , still_( still )
  , _particlesLife( 0.0f )
  {}

  DirectValuedEmissionNode::~DirectValuedEmissionNode()
  {}

  void DirectValuedEmissionNode::Color(glm::vec4 color)
  {
    color_ = color;
//    color_.a = 0.0f;
  }

  const glm::vec4& DirectValuedEmissionNode::Color()
  {
    return color_;
  }

  void DirectValuedEmissionNode::Still(bool still)
  {
    still_ = still;
  }

  bool DirectValuedEmissionNode::Still()
  {
    return still_;
  }


  void DirectValuedEmissionNode::Size(float size)
  {
    size_ = size;
  }

  float DirectValuedEmissionNode::Size()
  {
    return size_;
  }

  void DirectValuedEmissionNode::particlesLife( float life )
  {
    _particlesLife = life;
  }

  float DirectValuedEmissionNode::particlesLife( void )
  {
    return _particlesLife;
  }

  void DirectValuedEmissionNode::killParticles( bool changeState )
  {
    for( tparticle it = particles->begin( );
         it != particles->end( ); ++it )
    {
      it.life( 0.0f );

      if( changeState )
        it.alive( false );
      else
        it.alive( true );
    }
  }


}


