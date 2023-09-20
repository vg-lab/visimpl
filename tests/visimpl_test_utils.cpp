/*
 *  Copyright (c) 2015-2022 VG-Lab/URJC.
 *
 *  Authors: Gael Rial Costas  <gael.rial.costas@urjc.es>
 *
 *  This file is part of ViSimpl <https://github.com/vg-lab/visimpl>
 *
 *  This library is free software; you can redistribute it and/or modify it under
 *  the terms of the GNU Lesser General Public License version 3.0 as published
 *  by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this library; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

//
// Created by gaeqs on 2/11/22.
//

#include <iostream>
#include <GL/glew.h>
#include <GL/freeglut.h>

namespace test_utils
{

  void initOpenGLContext( )
  {
    int argc = 0;
    glutInit( &argc , nullptr );
    glutInitContextVersion( 3 , 3 );
    glutInitContextFlags( GLUT_FORWARD_COMPATIBLE );
    glutInitContextProfile( GLUT_CORE_PROFILE );

    glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH );
    glutInitWindowSize( 1 , 1 );
    glutInitWindowPosition( 0 , 0 );
    glutCreateWindow( "GLUT example" );

    glewExperimental = GL_TRUE;
    GLenum err = glewInit( );
    if ( GLEW_OK != err )
    {
      std::cout << "Error: " << glewGetErrorString( err ) << std::endl;
      exit( -1 );
    }
    const GLubyte* oglVersion = glGetString( GL_VERSION );
    std::cout << "This system supports OpenGL Version: "
              << oglVersion << std::endl;
  }

  void terminateOpenGLContext( )
  {
    glutExit( );
  }

}