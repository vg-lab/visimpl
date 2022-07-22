//
// Created by gaeqs on 21/06/22.
//

#include <GL/glew.h>

#include "NeuronParticle.h"

namespace visimpl
{

  void NeuronParticle::enableVAOAttributes( )
  {
    glEnableVertexAttribArray( 1 );
    glVertexAttribPointer( 1 , 3 , GL_FLOAT , GL_FALSE ,
                           sizeof( NeuronParticle ) ,
                           ( void* ) 0 );
    glVertexAttribDivisor( 1 , 1 );

    glEnableVertexAttribArray( 2 );
    glVertexAttribPointer( 2 , 1 , GL_FLOAT , GL_FALSE ,
                           sizeof( NeuronParticle ) ,
                           ( void* ) ( sizeof( float ) * 3 ));
    glVertexAttribDivisor( 2 , 1 );

  }
}