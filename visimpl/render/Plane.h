/*
 * @file  Plane.h
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */
#ifndef VISIMPL_RENDER_PLANE_H_
#define VISIMPL_RENDER_PLANE_H_

#include <reto/reto.h>

#include "../types.h"

namespace visimpl
{
  class Plane
  {
  public:

    Plane( );

    void points( evec3 first, evec3 second, evec3 third, evec3 fourth );

    void init( reto::Camera* camera );
    void render( reto::ShaderProgram* program_ );

    unsigned int _vao;
    unsigned int _vboVertex;

    reto::Camera* _camera;

    std::vector< evec3 > _points;
    std::vector< float > _vertices;
  };


}



#endif /* VISIMPL_RENDER_PLANE_H_ */
