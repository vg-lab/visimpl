/*
 * SpineRetParticleEmitter.h
 *
 *  Created on: 04/12/2014
 *      Author: sgalindo
 */

#ifndef __DirectValuedEmitter__
#define __DirectValuedEmitter__

#include <prefr/prefr.h>

#include "DirectValuedEmissionNode.h"
#include "ColorOperationPrototype.h"

namespace prefr
{

  class DirectValuedEmitter : public ParticleEmitter
  {
  public:

    DirectValuedEmitter( const ParticleCollection& particlesArray,
                           float _emissionRate, bool _loop);

    virtual ~DirectValuedEmitter();

    virtual void EmitFunction(const tparticle_ptr current,
                              bool override = false);

  };

}

#endif /* __SpineRetParticleEmitter__ */
