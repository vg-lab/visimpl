/*
 * SpineRetParticleUpdater.h
 *
 *  Created on: 04/12/2014
 *      Author: sgalindo
 */

#ifndef __StillParticleUpdater__
#define __StillParticleUpdater__

#include <prefr/prefr.h>
#include "ColorEmissionNode.h"
#include "ColorOperationPrototype.h"

namespace prefr
{
  class CompositeColorUpdater : public ParticleUpdater
  {
  public:

    CompositeColorUpdater( const ParticleCollection& particlesArray );
    ~CompositeColorUpdater();

    virtual void Update(const tparticle_ptr i, float deltaTime);
  };

}


#endif /* __SpineRetParticleUpdater__ */
