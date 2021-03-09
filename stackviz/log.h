/*
 * Copyright (c) 2015-2020 VG-Lab/URJC.
 *
 * Authors: Sergio E. Galindo <sergio.galindo@urjc.es>
 *
 * This file is part of ViSimpl <https://github.com/vg-lab/visimpl>
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
