/*
 *  Copyright (c) 2015-2022 VG-Lab/URJC.
 *
 *  Authors: Gael Rial Costas  <gael.rial.costas@urjc.es>
 *
 *  This file is part of ViSimpl <https://github.com/vg-lab/visimpl>
 *
 *  This library is free software; you can redistribute it and/or modify it under
 *  the terms of the GNU Lesser General Public License version 3.0 as published
 *  by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this library; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#define BOOST_TEST_MODULE sumrice_color_interpolator

#include <vector>

#include <boost/test/unit_test.hpp>
#include <sumrice/ColorInterpolator.h>

namespace boost
{
  namespace test_tools
  {
    namespace tt_detail
    {
      template< >
      struct print_log_value< glm::vec4 >
      {
        void operator()( std::ostream& os , const glm::vec4& tc )
        {
          os << "(" << tc.x << ", " << tc.y << ", "
             << tc.z << ", " << tc.w << ")";
        }
      };
    }
  }
}

BOOST_AUTO_TEST_CASE( sumrice_color_interpolator )
{
  ColorInterpolator interpolator;
  glm::vec4 r = glm::vec4( 1.0f , 0.0f , 0.0f , 1.0f );
  glm::vec4 g = glm::vec4( 0.0f , 1.0f , 0.0f , 1.0f );
  glm::vec4 b = glm::vec4( 0.0f , 0.0f , 1.0f , 1.0f );

  interpolator.insert( 0 , r );
  interpolator.insert( 1 , g );
  interpolator.insert( 2 , b );

  BOOST_CHECK_EQUAL( interpolator.getValue( -1.0f ) , r );
  BOOST_CHECK_EQUAL( interpolator.getValue( 0.0f ) , r );
  BOOST_CHECK_EQUAL( interpolator.getValue( 1.0f ) , g );
  BOOST_CHECK_EQUAL( interpolator.getValue( 2.0f ) , b );
  BOOST_CHECK_EQUAL( interpolator.getValue( 3.0f ) , b );

  glm::vec4 interpolated = glm::vec4( 0.5f , 0.5f , 0.0f , 1.0f );
  glm::vec4 interpolated2 = glm::vec4( 0.0f , 0.5f , 0.5f , 1.0f );
  BOOST_CHECK_EQUAL( interpolator.getValue( 0.5f ) , interpolated );
  BOOST_CHECK_EQUAL( interpolator.getValue( 1.5f ) , interpolated2 );
}