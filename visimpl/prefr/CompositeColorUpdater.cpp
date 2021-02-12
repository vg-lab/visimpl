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

// ViSimpl
#include "CompositeColorUpdater.h"

namespace prefr
{
  CompositeColorUpdater::CompositeColorUpdater( )
  : Updater( )
  {}

  void CompositeColorUpdater::emitParticle( const Cluster& cluster,
                                            const tparticle_ptr current )
  {
    auto model = dynamic_cast< ColorOperationModel* >( cluster.model( ));
    auto source = dynamic_cast< ColorSource* >( cluster.source( ));

    if( !current->alive( ))
    {
      current->set_life( model->minLife( ));

      current->set_alive( true );

      SampledValues values;
      source->sample( &values );

      current->set_position( values.position );
      current->set_velocity( values.direction );

    }
  }

  void CompositeColorUpdater::updateParticle( const Cluster& cluster,
                                      const tparticle_ptr current,
                                      float deltaTime )
  {
    auto model = dynamic_cast< ColorOperationModel* >( cluster.model( ));
    auto source = dynamic_cast< ColorSource* >( cluster.source( ));

    float refLife = 0;

    if( current->alive( ))
    {
      current->set_life( std::max(0.0f, current->life( ) - deltaTime ));

      refLife = 1.0f - glm::clamp( current->life( ) * model->inverseMaxLife( ),
                                   0.0f, 1.0f );

      if( !source->still( ))
      {
        current->set_velocityModule( model->velocity.GetValue( refLife ));

        current->set_position( current->position( )
                           + current->velocity( ) * current->velocityModule( ) *
                                                                    deltaTime );
      }

      current->set_color( glm::clamp(
          model->colorop( source->color( ), model->color.GetValue( refLife )),
          0.0f, 1.0f ));

      current->set_size( model->size.GetValue( refLife ) + source->size( ));
    }
  }
}
