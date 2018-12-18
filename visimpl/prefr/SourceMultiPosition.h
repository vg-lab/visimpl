/*
 * @file  SourceMultiPosition.h
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */
#ifndef SRC_PREFR_SOURCEMULTIPOSITION_H_
#define SRC_PREFR_SOURCEMULTIPOSITION_H_

#include "../types.h"
#include <prefr/prefr.h>

namespace visimpl
{
  class SourceMultiPosition : public prefr::Source
  {
  public:

    SourceMultiPosition( void );
    ~SourceMultiPosition( void );

    void setIdxTranslation( const tIdxTransMap& idxTranslation );
    void setPositions( const tGidPosMap& positions );

    void removeElements( const prefr::ParticleSet& indices );

    vec3 position( unsigned int idx );

  protected:

    const tGidPosMap* _positions;
    const tIdxTransMap* _idxTranslate;
  };


}


#endif /* SRC_PREFR_SOURCEMULTIPOSITION_H_ */
