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

#include "Plane.h"

#include <GL/glew.h>

namespace visimpl
{

  Plane::Plane( )
  : _vao( 0 )
  , _vboVertex( 0 )
  , _camera( nullptr )
  , _color( 1.0, 1.0, 1.0, 1.0 )
  {
    _points.resize( 4, evec3::Zero( ));
    _vertices = { -1.0, -1.0, 0.0,
                  1.0, -1.0, 0.0,
                  -1.0, 1.0, 0.0,
                  1.0, 1.0, 0.0 };
  }

  void Plane::init( reto::Camera* camera )
  {
    _camera = camera;

    glGenVertexArrays( 1, &_vao );
    glBindVertexArray( _vao );

    glGenBuffers( 1, &_vboVertex );

    // Assign vertices
    glBindBuffer( GL_ARRAY_BUFFER, _vboVertex );
    glBufferData( GL_ARRAY_BUFFER, sizeof( GLfloat ) * _vertices.size( ),
                  _vertices.data( ), GL_DYNAMIC_DRAW );
    glVertexAttribPointer( (GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

  }

  void Plane::color( evec4 color_ )
  {
    _color = color_;
  }

  void Plane::points( evec3 first, evec3 second, evec3 third, evec3 fourth )
  {
    _points[ 0 ] = first;
    _points[ 1 ] = second;
    _points[ 2 ] = third;
    _points[ 3 ] = fourth;

    _vertices[ 0 ] = first.x( );
    _vertices[ 1 ] = first.y( );
    _vertices[ 2 ] = first.z( );
    _vertices[ 3 ] = second.x( );
    _vertices[ 4 ] = second.y( );
    _vertices[ 5 ] = second.z( );
    _vertices[ 6 ] = third.x( );
    _vertices[ 7 ] = third.y( );
    _vertices[ 8 ] = third.z( );
    _vertices[ 9 ] = fourth.x( );
    _vertices[ 10 ] = fourth.y( );
    _vertices[ 11 ] = fourth.z( );

    glBindVertexArray( _vao );

    // Update positions buffer
    glBindBuffer( GL_ARRAY_BUFFER, _vboVertex );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof( GLfloat ) * _vertices.size( ),
                     &_vertices.front( ));

    glBindVertexArray( 0 );
  }

  const std::vector< evec3 >& Plane::points( void ) const
  {
    return _points;
  }

  void Plane::render( reto::ShaderProgram* program_ )
  {
    assert( _camera );

    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    program_->use( );

    glBindVertexArray( _vao );

    glDisable( GL_CULL_FACE );

    program_->sendUniform4m( "viewProj", _camera->projectionViewMatrix( ));
    program_->sendUniform4v( "inColor", _color.data( ));

    glDrawArrays(GL_LINE_LOOP, 0, 4);

    glBindVertexArray( 0 );

    glEnable( GL_CULL_FACE );

    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

    program_->unuse( );

  }


}

