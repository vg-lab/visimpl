//
// Created by gaeqs on 6/28/22.
//

#include "GlewInitializer.h"
#include <GL/glew.h>

namespace visimpl {

  bool GlewInitializer::_initialized = false;

  void GlewInitializer::init( void )
  {
    if ( !_initialized )
    {
      glewExperimental = GL_TRUE;
      glewInit( );
      _initialized = true;
    }
  }

  bool GlewInitializer::isInitialized( void )
  {
    return _initialized;
  }

}