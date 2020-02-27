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

    virtual void emitParticle( const Cluster& cluster,
                               const tparticle_ptr current );

    virtual void updateParticle( const Cluster& cluster,
                                 const tparticle_ptr current,
                                 float deltaTime );
  };

}


#endif /* __ValuedUpdater__ */
