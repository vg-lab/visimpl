/*
 * Copyright (c) 2015-2020 GMRV/URJC.
 *
 * Authors: Pablo Toharia <pablo.toharia@urjc.es>
 *
 * This file is part of ViSimpl <https://github.com/gmrvvis/visimpl>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3.0 as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
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
