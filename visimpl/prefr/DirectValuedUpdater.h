/*
 * SpineRetParticleUpdater.h
 *
 *  Created on: 04/12/2014
 *      Author: sgalindo
 */

#ifndef __DirectValuedUpdater__
#define __DirectValuedUpdater__

#include <prefr/prefr.h>
#include "DirectValuedEmissionNode.h"
#include "ColorOperationPrototype.h"

namespace prefr
{
  class DirectValuedUpdater : public ParticleUpdater
  {
  public:

    DirectValuedUpdater( const ParticleCollection& particlesArray );
    ~DirectValuedUpdater();

    virtual void Update(const tparticle_ptr i, float deltaTime);
  };

}


#endif /* __SpineRetParticleUpdater__ */
