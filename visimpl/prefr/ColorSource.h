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

#ifndef __VISIMPL__COLOREMISSIONNODE__
#define __VISIMPL__COLOREMISSIONNODE__

// Prefr
#include <prefr/prefr.h>

namespace prefr
{
  class ColorSource : public Source
  {
  public:
    ColorSource( float emissionRate, const glm::vec3& _position,
                 const glm::vec4& color, bool still = false);

    virtual ~ColorSource() {};

    virtual void color( const glm::vec4& color );
    virtual const glm::vec4& color( void );

    virtual void still( bool still );
    virtual bool still( void );

    virtual void size( float size );
    virtual float size( void);

    virtual bool emits() const;

  protected:
    glm::vec4 _color;
    float _size;
    bool _still;
  };
}

#endif /* __VISIMPL__COLOREMISSIONNODE__ */
