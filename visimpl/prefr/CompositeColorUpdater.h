#ifndef __VISIMPL__COMPOSITECOLORUPDATER__
#define __VISIMPL__COMPOSITECOLORUPDATER__

#include <prefr/prefr.h>
#include "ColorOperationModel.h"
#include "ColorSource.h"

namespace prefr
{
  class CompositeColorUpdater : public Updater
  {
  public:

    CompositeColorUpdater( );

    ~CompositeColorUpdater();

    virtual void Emit( const Cluster& cluster, const tparticle_ptr current );

    virtual void Update( const Cluster& cluster, const tparticle_ptr current,
                         float deltaTime );
  };

}


#endif /* __VISIMPL__COMPOSITECOLORUPDATER__ */
