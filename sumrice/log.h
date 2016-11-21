/**
 * @file    log.h
 * @brief
 * @author  Pablo Toharia <pablo.toharia@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
            Do not distribute without further notice.
 */

#ifndef __VISIMPL_ERROR__
#define __VISIMPL_ERROR__

#include <stdexcept>
#include <iostream>

#ifdef DEBUG
  #define VISIMPL_LOG( msg )                                       \
    std::cerr << "VISIMPL "                                        \
              << __FILE__ << "("                                 \
              << __LINE__ << "): "                               \
              << msg << std::endl;
#else
  #define VISIMPL_LOG( msg )
#endif


#define VISIMPL_THROW( msg )                                       \
  {                                                              \
    VISIMPL_LOG( msg );                                            \
    throw std::runtime_error( msg );                             \
  }


#define VISIMPL_CHECK_THROW( cond, errorMsg )                      \
    {                                                            \
      if ( ! (cond) ) VISIMPL_THROW( errorMsg );                   \
    }


#ifdef DEBUG
  #define VISIMPL_DEBUG_CHECK( cond, errorMsg )                    \
{                                                                \
  VISIMPL_CHECK_THROW( cond, errorMsg )                            \
}
#else
  #define VISIMPL_DEBUG_CHECK( cond, errorMsg )
#endif


#endif // __VISIMPL_ERROR__
