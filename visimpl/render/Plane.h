/*
 * Copyright (c) 2015-2020 VG-Lab/URJC.
 *
 * Authors: Sergio E. Galindo <sergio.galindo@urjc.es>
 *
 * This file is part of ViSimpl <https://github.com/vg-lab/visimpl>
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

#ifndef VISIMPL_RENDER_PLANE_H_
#define VISIMPL_RENDER_PLANE_H_

// ReTo
#include <reto/reto.h>

// Visimpl
#include "../types.h"

namespace visimpl
{
  class Plane
  {
  public:
    Plane( );

    void points( evec3 first, evec3 second, evec3 third, evec3 fourth );

    const std::vector< evec3 >& points( void ) const;

    void init( reto::Camera* camera );
    void render( reto::ShaderProgram* program_ );

    void color( evec4 color_ );

    unsigned int _vao;
    unsigned int _vboVertex;

    reto::Camera* _camera;

    evec4 _color;

    std::vector< evec3 > _points;
    std::vector< float > _vertices;
  };
}

#endif /* VISIMPL_RENDER_PLANE_H_ */
