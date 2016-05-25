/*
 * SpineRetParticleUpdater.cpp
 *
 *  Created on: 04/12/2014
 *      Author: sgalindo
 */

#include "CompositeColorUpdater.h"

namespace prefr
{

  CompositeColorUpdater::CompositeColorUpdater( const ParticleCollection& particlesArray )
  : ParticleUpdater( particlesArray )
  {}

  CompositeColorUpdater::~CompositeColorUpdater()
  {}

  void CompositeColorUpdater::Update(const tparticle_ptr current, float deltaTime)
  {
    ColorOperationPrototype* cProto =
        dynamic_cast<ColorOperationPrototype*>(
            GetCurrentPrototype( current->id ) );

    ColorEmissionNode* node =
        dynamic_cast<ColorEmissionNode*>( GetCurrentNode( current->id ) );

    float refLife = 0;

//    if (!node->Still())
    if( current->life > 0.0f)
    {
      current->life = std::max(0.0f, current->life - deltaTime);
    }
    current->alive = (current->life > 0 || current->alive) && node->active;

    if (current->Alive() && !current->Newborn())
    {

      refLife = 1.0f - glm::clamp((current->life) *
                                  (cProto->lifeNormalization), 0.0f, 1.0f);

      if (!node->Still())
      {

        current->velocityModule = cProto->velocity.GetValue(refLife);

        current->position += current->velocity * current->velocityModule *
                                                                    deltaTime;
      }

      current->color = glm::clamp(
          cProto->colorop( node->Color( ), cProto->color.GetValue( refLife )),
          0.0f, 1.0f );

      current->size = cProto->size.GetValue(refLife) + node->Size();
    }

    current->newborn = false;

  }

}

