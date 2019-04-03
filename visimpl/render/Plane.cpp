/*
 * @file  Plane.cpp
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */

#include "Plane.h"

#include <GL/glew.h>

namespace visimpl
{

  Plane::Plane( )
  : _vao( 0 )
  , _vboVertex( 0 )
  , _camera( nullptr )
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

  void Plane::render( reto::ShaderProgram* program_ )
  {
    assert( _camera );

    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    program_->use( );

    glBindVertexArray( _vao );

    glDisable( GL_CULL_FACE );

    program_->sendUniform4m( "viewProj", _camera->viewProjectionMatrix( ));
//    program_->sendUniform4m( "rotation", _planeRotation.data( ));
//    program_->sendUniform3v( "center", center.data( ));
//    program_->sendUniformf( "height", planeSize.x( ));
//    program_->sendUniformf( "width", planeSize.y( ));

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glBindVertexArray( 0 );

    glEnable( GL_CULL_FACE );

    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

    program_->unuse( );

  }


}

