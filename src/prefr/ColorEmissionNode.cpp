/*
 * SpineRetEmissionNode.cpp
 *
 *  Created on: 03/12/2014
 *      Author: sgalindo
 */

#include "ColorEmissionNode.h"

namespace prefr
{

  ColorEmissionNode::ColorEmissionNode(const ParticleCollection& arrayParticles,
                                       glm::vec3 _position, glm::vec4 color,
                                       bool still)
  : PointEmissionNode( arrayParticles, _position )
  , color_( color )
  , size_ ( 0.0f )
  , still_( still )
  {}

  ColorEmissionNode::~ColorEmissionNode()
  {}

  void ColorEmissionNode::Color(glm::vec4 color)
  {
    color_ = color;
//    color_.a = 0.0f;
  }

  const glm::vec4& ColorEmissionNode::Color()
  {
    return color_;
  }

  void ColorEmissionNode::Still(bool still)
  {
    still_ = still;
  }

  bool ColorEmissionNode::Still()
  {
    return still_;
  }


  void ColorEmissionNode::Size(float size)
  {
    size_ = size;
  }

  float ColorEmissionNode::Size()
  {
    return size_;
  }

  void ColorEmissionNode::killParticles( bool changeState )
  {
    for( tparticleContainer::iterator it = particles->start;
         it != particles->end; it++ )
    {
      ( *it )->life = 0.0f;

      if( changeState )
        ( *it )->alive = false;
//      else
//        ( *it )->alive = true;
    }
  }


}


