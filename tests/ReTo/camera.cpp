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

#define BOOST_TEST_MODULE reto_camera

#include <vector>

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>
#include <reto/reto.h>

BOOST_AUTO_TEST_CASE( reto_camera_move )
{
  reto::Camera camera;
  reto::FreeCameraController controller( &camera );

  // Check identity
  auto* data = camera.viewMatrix( );
  Eigen::Matrix4f identity = Eigen::Matrix4f::Identity( );
  auto* iData = identity.data( );
  BOOST_CHECK_EQUAL_COLLECTIONS( data , data + 16 , iData , iData + 16 );

  // Check movement
  auto move = Eigen::Vector3f( 10.0f , 0.0f , 0.0f );
  controller.position( move );
  BOOST_CHECK_EQUAL( controller.position( ) , move );
  controller.translate( move );
  BOOST_CHECK_EQUAL( controller.position( ) , move + move );
  controller.localTranslate( move );
  BOOST_CHECK_EQUAL( controller.position( ) , move + move + move );

  // Check movement with rotation
  controller.position( Eigen::Vector3f( 0.0f , 0.0f , 0.0f ));
  controller.rotate( Eigen::Vector3f( 1.570796325f , 0.0f , 0.0f ));
  controller.translate( move );
  BOOST_CHECK_EQUAL( controller.position( ) , move );
  controller.position( Eigen::Vector3f( 0.0f , 0.0f , 0.0f ));
  controller.localTranslate( move );

  auto rotatedMove = Eigen::Vector3f( 0.0f , 0.0f , 10.0f );
  auto distance = std::abs(( controller.position( ) - rotatedMove ).norm( ));
  BOOST_CHECK( distance < 0.0001 );
}