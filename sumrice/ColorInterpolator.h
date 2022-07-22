//
// Created by gaeqs on 6/22/22.
//

#ifndef VISIMPL_COLORINTERPOLATOR_H
#define VISIMPL_COLORINTERPOLATOR_H

#include <vector>
#include <glm/glm.hpp>

struct ColorInterpolator
{

  std::vector< std::pair< float , glm::vec4>> data;

  glm::vec4 getValue( float percentage );

  void insert( float percentage , glm::vec4 value );
};


#endif //VISIMPL_COLORINTERPOLATOR_H
