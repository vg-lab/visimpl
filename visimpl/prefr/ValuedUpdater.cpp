/*
 * @file  ValuedUpdater.cpp
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */

#include "ValuedUpdater.h"

namespace prefr
{

//  static float invRandMax = 1.0f / RAND_MAX;

  ValuedUpdater::ValuedUpdater( )
  : Updater( )
  { }

  ValuedUpdater::~ValuedUpdater( )
  { }

  void ValuedUpdater::emitParticle( const Cluster& cluster,
                                    const tparticle_ptr current )
  {
    ColorOperationModel* model =
            dynamic_cast< ColorOperationModel* >( cluster.model( ));

    ValuedSource* source = dynamic_cast< ValuedSource* >( cluster.source( ));

    if( model && ( !current->alive( )))
    {
     current->set_life( 0.0f );
     current->set_alive( true );

     SampledValues values;
     source->sample( &values );

     current->set_position( values.position );
     current->set_velocity( values.direction );

    }
  }


  void ValuedUpdater::updateParticle( const Cluster& cluster,
                                      const tparticle_ptr current,
                                      float deltaTime )
  {
    ColorOperationModel* model =
        dynamic_cast< ColorOperationModel* >( cluster.model( ));

    ValuedSource* node = dynamic_cast< ValuedSource* >( cluster.source( ));

    current->set_life( std::max( 0.0f, current->life( ) - deltaTime ));

//    float refLife = (current->life( ) - model->minLife( )) * model->inverseMaxLife() ;
    float refLife = 1.0f - glm::clamp( current->life( ) * model->inverseMaxLife( ),
                                 0.0f, 1.0f );
//    current->alive( cluster.active( ));

    if( current->alive( ))
    {
      if( !node->still( ))
      {

        current->set_velocityModule( model->velocity.GetValue( refLife ));

        current->set_position( current->position( ) +
                           current->velocity( ) *
                           current->velocityModule( ) * deltaTime );
      }

      current->set_color( glm::clamp(
          model->colorop( node->color( ),
                           model->color.GetValue( refLife )), 0.0f, 1.0f ));

      current->set_size( model->size.GetValue( refLife ) + node->size());
    }

  }

}

