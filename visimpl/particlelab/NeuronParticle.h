//
// Created by gaeqs on 21/06/22.
//

#ifndef VISIMPL_NEURONPARTICLE_H
#define VISIMPL_NEURONPARTICLE_H

#include <limits>

#include <glm/vec3.hpp>

namespace visimpl
{

  struct NeuronParticle
  {
    glm::vec3 position;
    float timestamp = -std::numeric_limits<float>::infinity();

    static void enableVAOAttributes( );

  };

}


#endif //VISIMPL_NEURONPARTICLE_H
