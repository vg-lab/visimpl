/*
 * @file  CompositeColorUpdater.cpp
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */

#include "CompositeColorUpdater.h"

namespace prefr
{

  CompositeColorUpdater::CompositeColorUpdater( )
  : Updater( )
  {}

  CompositeColorUpdater::~CompositeColorUpdater()
  {}

  void CompositeColorUpdater::emitParticle( const Cluster& cluster,
                                            const tparticle_ptr current )
  {

    ColorOperationModel* model =
        dynamic_cast< ColorOperationModel* >( cluster.model( ));

    ColorSource* source =
        dynamic_cast< ColorSource* >( cluster.source( ));

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
    ColorOperationModel* model =
        dynamic_cast< ColorOperationModel* >( cluster.model( ));

    ColorSource* source =
        dynamic_cast< ColorSource* >( cluster.source( ));

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

