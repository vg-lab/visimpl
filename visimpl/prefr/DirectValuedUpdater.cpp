/*
 * SpineRetParticleUpdater.cpp
 *
 *  Created on: 04/12/2014
 *      Author: sgalindo
 */

#include "DirectValuedUpdater.h"

namespace prefr
{

  DirectValuedUpdater::DirectValuedUpdater( const ParticleCollection& particlesArray )
  : ParticleUpdater( particlesArray )
  {}

  DirectValuedUpdater::~DirectValuedUpdater()
  {}

  void DirectValuedUpdater::Update(const tparticle_ptr current, float deltaTime)
  {
    ColorOperationPrototype* cProto =
        dynamic_cast<ColorOperationPrototype*>(
            GetCurrentPrototype( current->id( ) ) );

    DirectValuedEmissionNode* node =
        dynamic_cast<DirectValuedEmissionNode*>( GetCurrentNode( current->id( ) ) );

    float refLife = node->particlesLife( );

//    if (!node->Still())
      current->life( std::max( 0.0f, current->life( ) - deltaTime ));

    current->alive( node->active );

    if (current->alive( ) && !current->newborn( ))
    {

//      refLife = 1.0f - glm::clamp((current->life) *
//                                  (cProto->lifeNormalization), 0.0f, 1.0f);

      if (!node->Still())
      {

        current->velocityModule( cProto->velocity.GetValue( refLife ));

        current->position( 
          current->position( ) + current->velocity( ) * current->velocityModule( ) * deltaTime );
      }

      current->color( glm::clamp(
          cProto->colorop( node->Color( ), cProto->color.GetValue( refLife )),
          0.0f, 1.0f ));

      current->size( cProto->size.GetValue(refLife) + node->Size());
    }

    current->newborn( false );

  }

}

