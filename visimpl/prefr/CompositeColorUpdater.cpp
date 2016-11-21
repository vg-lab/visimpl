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
      current->life( model->minLife( ));

      current->alive( true );

      SampledValues values;
      source->sample( &values );

      current->position( values.position );
      current->velocity( values.direction );

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

      current->life( std::max(0.0f, current->life( ) - deltaTime ));

      refLife = 1.0f - glm::clamp( current->life( ) * model->inverseMaxLife( ),
                                   0.0f, 1.0f );

      if( !source->still( ))
      {

        current->velocityModule( model->velocity.GetValue( refLife ));

        current->position( current->position( )
                           + current->velocity( ) * current->velocityModule( ) *
                                                                    deltaTime );
      }

      current->color( glm::clamp(
          model->colorop( source->color( ), model->color.GetValue( refLife )),
          0.0f, 1.0f ));

      current->size( model->size.GetValue( refLife ) + source->size( ));
    }

  }

}

