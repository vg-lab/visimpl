/*
 * SpineRetEmissionNode.h
 *
 *  Created on: 03/12/2014
 *      Author: sgalindo
 */

#ifndef __DIRECTVALUEDEMISSIONNODE_H__
#define __DIRECTVALUEDEMISSIONNODE_H__

#include <prefr/prefr.h>

namespace prefr
{

  class DirectValuedEmissionNode : public PointEmissionNode
  {
  public:


    DirectValuedEmissionNode( const ParticleCollection& arrayParticles,
                       glm::vec3 _position, glm::vec4 color,
                       bool still = false);
    virtual ~DirectValuedEmissionNode();

    virtual void killParticles( bool changeState = true );

    virtual void Color(glm::vec4 color);
    virtual const glm::vec4& Color();

    virtual void Still( bool still );
    virtual bool Still();

    virtual void Size( float size );
    virtual float Size();

    virtual void particlesLife( float life );
    virtual float particlesLife( void );

  protected:

    glm::vec4 color_;
    float size_;
    bool still_;

    float _particlesLife;

  };

}

#endif /* SPINERETEMISSIONNODE_H_ */
