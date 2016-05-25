/*
 * SpineRetParticleEmitter.cpp
 *
 *  Created on: 04/12/2014
 *      Author: sgalindo
 */

#include "DirectValuedEmitter.h"


namespace prefr
{

  static float invRandMax = 1.0f / RAND_MAX;

  DirectValuedEmitter::
  DirectValuedEmitter( const ParticleCollection& particlesArray,
                           float _emissionRate, bool _loop)
  : ParticleEmitter( particlesArray, _emissionRate, _loop )
  {}

  DirectValuedEmitter::~DirectValuedEmitter()
  {}

  void DirectValuedEmitter::EmitFunction(const tparticle_ptr current, bool override)
  {
    ColorOperationPrototype* cProto =
            dynamic_cast<ColorOperationPrototype*>(
                GetCurrentPrototype( current->id ) );

    DirectValuedEmissionNode* node =
        dynamic_cast<DirectValuedEmissionNode*>( GetCurrentNode( current->id ) );

    if (cProto && (!current->Alive() || override))
    {
     current->life = glm::clamp(rand() * invRandMax, 0.0f, 1.0f) *
         cProto->lifeInterval + cProto->minLife;

     current->velocity = node->GetEmissionVelocityDirection();
     current->position = node->GetEmissionPosition();

     current->color = glm::clamp(
         cProto->colorop(node->Color(), cProto->color.GetFirstValue()),
         0.0f, 1.0f);

     current->velocityModule = cProto->velocity.GetFirstValue();

     current->size = cProto->size.GetFirstValue();

     current->newborn = false;


    }
  }


}

