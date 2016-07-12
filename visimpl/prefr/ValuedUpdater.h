/*
 * SpineRetParticleUpdater.h
 *
 *  Created on: 04/12/2014
 *      Author: sgalindo
 */

#ifndef __DirectValuedUpdater__
#define __DirectValuedUpdater__

#include <prefr/prefr.h>
#include "ColorOperationModel.h"
#include "ValuedSource.h"

namespace prefr
{
  class ValuedUpdater : public Updater
  {
  public:

    ValuedUpdater( );

    ~ValuedUpdater( );

    virtual void Emit( const Cluster& cluster, const tparticle_ptr current );

    virtual void Update( const Cluster& cluster, const tparticle_ptr current,
                         float deltaTime );
  };

}


#endif /* __SpineRetParticleUpdater__ */
