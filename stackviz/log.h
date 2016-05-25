/**
 * @file    error.h
 * @brief
 * @author  Pablo Toharia <pablo.toharia@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
            Do not distribute without further notice.
 */
#ifndef __STACKVIZ_ERROR__
#define __STACKVIZ_ERROR__

#include <stdexcept>
#include <iostream>

#ifdef DEBUG
  #define STACKVIZ_LOG( msg )                                       \
    std::cerr << "STACKVIZ "                                        \
              << __FILE__ << "("                                 \
              << __LINE__ << "): "                               \
              << msg << std::endl;
#else
  #define STACKVIZ_LOG( msg )
#endif


#define STACKVIZ_THROW( msg )                                       \
  {                                                              \
    STACKVIZ_LOG( msg );                                            \
    throw std::runtime_error( msg );                             \
  }


#define STACKVIZ_CHECK_THROW( cond, errorMsg )                      \
    {                                                            \
      if ( ! (cond) ) STACKVIZ_THROW( errorMsg );                   \
    }


#ifdef DEBUG
  #define STACKVIZ_DEBUG_CHECK( cond, errorMsg )                    \
{                                                                \
  STACKVIZ_CHECK_THROW( cond, errorMsg )                            \
}
#else
  #define STACKVIZ_DEBUG_CHECK( cond, errorMsg )
#endif


#endif // __STACKVIZ_ERROR__
