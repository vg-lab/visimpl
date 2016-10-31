#include "CompositeColorUpdater.h"

namespace prefr
{

  CompositeColorUpdater::CompositeColorUpdater( )
  : Updater( )
  {}

  CompositeColorUpdater::~CompositeColorUpdater()
  {}

  void CompositeColorUpdater::Emit( const Cluster& cluster,
                                    const tparticle_ptr current )
  {

    ColorOperationModel* cProto =
        dynamic_cast< ColorOperationModel* >( cluster.model( ));

    ColorSource* node =
        dynamic_cast< ColorSource* >( cluster.source( ));

    if( !current->alive( ))
    {
      current->life( cProto->minLife( ));

      current->alive( true );

      current->velocity( node->GetEmissionVelocityDirection( ));
      current->position( node->GetEmissionPosition( ));

      current->color( glm::clamp(
          cProto->colorop( node->color( ), cProto->color.GetValue( 1.f )),
          0.0f, 1.0f ));

      current->velocityModule( cProto->velocity.GetValue( 0.f ));

      current->size( cProto->size.GetValue( 0.f ));

//
//      std::cout << "E Particle: " << current->id( )
//                << " life " << current->life( )
//                << " color " << current->color( ).x
//                << ", " << current->color( ).y
//                << ", " << current->color( ).z
//                << std::endl;

      current->newborn( true );
    }
  }

  void CompositeColorUpdater::Update( const Cluster& cluster,
                                      const tparticle_ptr current,
                                      float deltaTime )
  {
    ColorOperationModel* cProto =
        dynamic_cast< ColorOperationModel* >( cluster.model( ));

    ColorSource* node =
        dynamic_cast< ColorSource* >( cluster.source( ));

    float refLife = 0;

    if (current->alive( ))
    {

      current->life( std::max(0.0f, current->life( ) - deltaTime ));

      refLife = 1.0f - glm::clamp( current->life( ) * cProto->inverseMaxLife( ),
                                   0.0f, 1.0f );

      if( !node->still( ))
      {

        current->velocityModule( cProto->velocity.GetValue( refLife ));

        current->position( current->position( )
                           + current->velocity( ) * current->velocityModule( ) *
                                                                    deltaTime );
      }

      current->color( glm::clamp(
          cProto->colorop( node->color( ), cProto->color.GetValue( refLife )),
          0.0f, 1.0f ));

      current->size( cProto->size.GetValue( refLife ) + node->size( ));

//      if( current->newborn( ))
//      std::cout << "Particle: " << current->id( )
//                << " life " << current->life( ) << " - " << refLife
//                << " color " << current->color( ).x
//                << ", " << current->color( ).y
//                << ", " << current->color( ).z
//                << std::endl;
    }

    current->newborn( false );

  }

}

