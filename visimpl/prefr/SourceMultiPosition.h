/*
 * Copyright (c) 2015-2020 GMRV/URJC.
 *
 * Authors: Sergio E. Galindo <sergio.galindo@urjc.es>
 *
 * This file is part of ViSimpl <https://github.com/gmrvvis/visimpl>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3.0 as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#ifndef SRC_PREFR_SOURCEMULTIPOSITION_H_
#define SRC_PREFR_SOURCEMULTIPOSITION_H_

// Visimpl
#include "../types.h"

// Prefr
#include <prefr/prefr.h>

namespace visimpl
{
  class SourceMultiPosition : public prefr::Source
  {
  public:
    SourceMultiPosition( void );

    virtual ~SourceMultiPosition( ) {};

    void setIdxTranslation( const tUintUMap& idxTranslation );
    void setPositions( const tGidPosMap& positions );

    void removeElements( const prefr::ParticleSet& indices );

    vec3 position( unsigned int idx );

  protected:
    const tGidPosMap* _positions;
    const tUintUMap* _idxTranslate;
  };
}

#endif /* SRC_PREFR_SOURCEMULTIPOSITION_H_ */
