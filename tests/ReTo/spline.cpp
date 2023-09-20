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

#define BOOST_TEST_MODULE reto_spline

#include <reto/reto.h>
#include <boost/test/unit_test.hpp>

using namespace reto;

BOOST_AUTO_TEST_CASE( reto_spline_example )
{
  std::vector<Eigen::Vector3f> v;
  v.push_back(Eigen::Vector3f{0.0f, 0.0f, 0.0f});
  v.push_back(Eigen::Vector3f{0.0f, 200.0f, 0.0f});
  v.push_back(Eigen::Vector3f{150.0f, 150.0f, 0.0f});
  v.push_back(Eigen::Vector3f{150.0f, 50.0f, 0.0f});
  v.push_back(Eigen::Vector3f{250.0f, 100.0f, 0.0f});
  v.push_back(Eigen::Vector3f{250.0f, 300.0f, 0.0f});
  v.push_back(Eigen::Vector3f{150.0f, 50.0f, 0.0f});

  Spline sp(v);

  BOOST_CHECK_EQUAL( sp.evaluate( 0.0f ),
    Eigen::Vector3f(0.0f, 0.0f, 0.0f) );
  BOOST_CHECK_EQUAL( sp.evaluate( 0.5f ),
    Eigen::Vector3f(150.0f, 50.0f, 0.0f) );

  BOOST_CHECK_EQUAL( sp.evaluate( 1.1f ),
    Eigen::Vector3f(150.0f, 50.0f, 0.0f) );

  sp.evaluate( 0.8f );
  BOOST_CHECK_EQUAL( sp.angleBetweenPoints( ), 0.0f );
  BOOST_CHECK_EQUAL( sp.angleBetweenPoints( 0.0f, 0.1f ), 0.054831136f );

  auto tg0 =  sp.getTangent( );
  BOOST_CHECK_EQUAL( tg0( 0 ), -0.862098992f );
  auto tg1 =  sp.getTangent( 0.0f, 0.1f );
  BOOST_CHECK_EQUAL( tg1( 0 ), -0.0838161781f );
}
