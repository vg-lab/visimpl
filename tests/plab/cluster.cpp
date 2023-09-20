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

#define BOOST_TEST_MODULE plab_cluster

#include <vector>

#include <boost/test/unit_test.hpp>
#include <visimpl_test_utils.h>
#include <plab/plab.h>

BOOST_AUTO_TEST_CASE( plab_cluster_fill )
{
  std::vector< BasicParticle > particles;

  for ( uint32_t i = 0; i < 100; ++i )
  {
    particles.push_back( {{ i , i , i }} );
  }

  test_utils::initOpenGLContext( );
  plab::Cluster< BasicParticle > cluster;

  // Test normal allocation
  cluster.setParticles( particles );
  BOOST_CHECK_EQUAL( 100 , cluster.size( ));

  auto* map = cluster.mapData( );
  for ( uint32_t i = 0; i < cluster.size( ); ++i )
  {
    BOOST_CHECK_EQUAL( i , ( map + i )->position.x );
    BOOST_CHECK_EQUAL( i , ( map + i )->position.y );
    BOOST_CHECK_EQUAL( i , ( map + i )->position.z );
  }
  cluster.unmapData( );

  cluster.setParticles( { } );
  BOOST_CHECK_EQUAL( 0 , cluster.size( ));

  // Test raw allocation
  cluster.allocateBuffer( 100 );
  BOOST_CHECK_EQUAL( 100 , cluster.size( ));

  // The buffer should be filled with invalid data.

  cluster.allocateBuffer( 0 );
  BOOST_CHECK_EQUAL( 0 , cluster.size( ));

  test_utils::terminateOpenGLContext();
}