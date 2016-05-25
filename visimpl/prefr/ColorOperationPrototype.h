/*
 * SpineRetParticlePrototype.h
 *
 *  Created on: 05/12/2014
 *      Author: sergio
 */

#ifndef SPINERETPARTICLEPROTOTYPE_H_
#define SPINERETPARTICLEPROTOTYPE_H_

#include <prefr/prefr.h>

namespace prefr
{

  enum ColorOperation
  {
    ADDITION = 0,
    SUBSTRACTION,
    MULTIPLICATION,
    DIVISION
  };

  class ColorOperationPrototype : public ParticlePrototype
  {
  public:

    ColorOperation colorOperation;

    glm::vec4 (*colorop)(const glm::vec4& lhs, const glm::vec4& rhs);

    ColorOperationPrototype(float min, float max,
                              const ParticleCollection& particlesArray,
                              ColorOperation colorOp = ADDITION);

    void SetColorOperation(ColorOperation colorOp);

  };


}


#endif /* SPINERETPARTICLEPROTOTYPE_H_ */
