/*
 * @file  ValuedSource.h
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */

#ifndef __VISIMPL__VALUEDSOURCE__
#define __VISIMPL__VALUEDSOURCE__

#include <prefr/prefr.h>

namespace prefr
{

  class ValuedSource : public Source
  {
  public:


    ValuedSource( float emissionRate, const glm::vec3& position,
                  const glm::vec4& color, bool still = false );

    virtual ~ValuedSource();

    virtual void color( const glm::vec4& color);
    virtual const glm::vec4& color();

    virtual void still( bool still );
    virtual bool still();

    virtual void size( float size );
    virtual float size();

    virtual void particlesRelLife( float relLife );

    virtual void particlesLife( float life );

    bool emits( void ) const;

  protected:

    void _prepareParticles( void );

    glm::vec4 _color;
    float _size;
    bool _still;

    float _particlesRelLife;
    float _particlesLife;

  };

}

#endif /* __VISIMPL__VALUEDSOURCE__ */
