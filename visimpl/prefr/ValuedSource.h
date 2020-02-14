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

  };

}

#endif /* __VISIMPL__VALUEDSOURCE__ */
