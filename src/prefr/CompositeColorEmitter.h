/*
 * SpineRetParticleEmitter.h
 *
 *  Created on: 04/12/2014
 *      Author: sgalindo
 */

#ifndef __SpineRetParticleEmitter__
#define __SpineRetParticleEmitter__

#include <prefr/prefr.h>

#include "ColorEmissionNode.h"
#include "ColorOperationPrototype.h"

namespace prefr
{

  class CompositeColorEmitter : public ParticleEmitter
  {
  public:

    CompositeColorEmitter( const ParticleCollection& particlesArray,
                           float _emissionRate, bool _loop);

    virtual ~CompositeColorEmitter();

    virtual void EmitFunction(const tparticle_ptr current,
                              bool override = false);

  };

}

#endif /* __SpineRetParticleEmitter__ */
