//
// Created by gaeqs on 6/28/22.
//

#ifndef VISIMPL_GLEWINITIALIZER_H
#define VISIMPL_GLEWINITIALIZER_H


namespace visimpl
{

  class GlewInitializer
  {

    static bool _initialized;

  public:
    static void init( );

    static bool isInitialized( );

  };
}


#endif //VISIMPL_GLEWINITIALIZER_H
