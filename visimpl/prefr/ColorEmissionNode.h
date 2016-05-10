/*
 * SpineRetEmissionNode.h
 *
 *  Created on: 03/12/2014
 *      Author: sgalindo
 */

#ifndef COLOREMISSIONNODE_H_
#define COLOREMISSIONNODE_H_

#include <prefr/prefr.h>

namespace prefr
{

  class ColorEmissionNode : public PointEmissionNode
  {
  public:


    ColorEmissionNode( const ParticleCollection& arrayParticles,
                       glm::vec3 _position, glm::vec4 color,
                       bool still = false);
    virtual ~ColorEmissionNode();

    virtual void killParticles( bool changeState = true );

    virtual void Color(glm::vec4 color);
    virtual const glm::vec4& Color();

    virtual void Still( bool still );
    virtual bool Still();

    virtual void Size( float size );
    virtual float Size();

    virtual bool Emits();

  protected:

    glm::vec4 color_;
    float size_;
    bool still_;


  };

}

#endif /* SPINERETEMISSIONNODE_H_ */
