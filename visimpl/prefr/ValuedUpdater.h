/*
 * @file  ValuedUpdater.h
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */

#ifndef __ValuedUpdater__
#define __ValuedUpdater__

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


#endif /* __ValuedUpdater__ */
