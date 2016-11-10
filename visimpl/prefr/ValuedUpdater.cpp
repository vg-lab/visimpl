/*
 * SpineRetParticleUpdater.cpp
 *
 *  Created on: 04/12/2014
 *      Author: sgalindo
 */

#include "ValuedUpdater.h"

namespace prefr
{

  static float invRandMax = 1.0f / RAND_MAX;

  ValuedUpdater::ValuedUpdater( )
  : Updater( )
  { }

  ValuedUpdater::~ValuedUpdater( )
  { }

  void ValuedUpdater::Emit( const Cluster& cluster, const tparticle_ptr current )
  {
    ColorOperationModel* model =
            dynamic_cast< ColorOperationModel* >( cluster.model( ));

    ValuedSource* source = dynamic_cast< ValuedSource* >( cluster.source( ));

    if( model && ( !current->alive( )))
    {
     current->life( glm::clamp(rand( ) * invRandMax, 0.0f, 1.0f) *
         model->lifeInterval( ) + model->minLife( ) );

     SampledValues values;
     source->sample( &values );

     current->position( values.position );
     current->velocity( values.direction );

    }
  }


  void ValuedUpdater::Update( const Cluster& cluster,
                              const tparticle_ptr current,
                              float deltaTime )
  {
    ColorOperationModel* cProto =
        dynamic_cast< ColorOperationModel* >( cluster.model( ));

    ValuedSource* node = dynamic_cast< ValuedSource* >( cluster.source( ));

    float refLife = node->particlesLife( );

    current->life( std::max( 0.0f, current->life( ) - deltaTime ));

    current->alive( cluster.active( ));

    if( current->alive( ))
    {
      if( !node->still( ))
      {

        current->velocityModule( cProto->velocity.GetValue( refLife ));

        current->position( current->position( ) +
                           current->velocity( ) *
                           current->velocityModule( ) * deltaTime );
      }

      current->color( glm::clamp(
          cProto->colorop( node->color( ),
                           cProto->color.GetValue( refLife )), 0.0f, 1.0f ));

      current->size( cProto->size.GetValue( refLife ) + node->size());
    }

  }

}

