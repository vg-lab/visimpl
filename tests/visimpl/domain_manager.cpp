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

#define BOOST_TEST_MODULE visimpl_domain_manager

#include <boost/test/unit_test.hpp>
#include <visimpl_test_utils.h>
#include <visimpl/DomainManager.h>
#include <visimpl/types.h>

BOOST_AUTO_TEST_CASE( visimpl_domain_manager_groups_init )
{
  float minLimit = std::numeric_limits< float >::min( );
  float maxLimit = std::numeric_limits< float >::max( );
  visimpl::GIDUSet testSet{ 0 };
  visimpl::tGidPosMap testSetPositions{{ 0 , { 1.0f , 0.0f , 0.0f }}};
  auto camera = std::make_shared< visimpl::Camera >( );

  test_utils::initOpenGLContext( );
  visimpl::DomainManager dManager;

  // Check initial conditions
  BOOST_CHECK_EQUAL( dManager.getGroupAmount( ) , 0 );
  BOOST_CHECK_EQUAL( dManager.getAttributeClusters( ).size( ) , 0 );
  BOOST_CHECK_EQUAL(
    static_cast<int>(dManager.getMode( )) ,
    static_cast<int>(visimpl::VisualMode::Selection)
  );

  dManager.initRenderers( nullptr , nullptr , camera );

  // Check if bounding box is empty
  BOOST_CHECK_EQUAL( dManager.getBoundingBox( ).first.x , maxLimit );
  BOOST_CHECK_EQUAL( dManager.getBoundingBox( ).second.x , minLimit );

  // Check selection
  dManager.setSelection( testSet , testSetPositions );

  BOOST_CHECK_EQUAL( dManager.getBoundingBox( ).first.x , 1.0f );
  BOOST_CHECK_EQUAL( dManager.getBoundingBox( ).second.x , 1.0f );

  // DomainManager has two setSelection methods!
  dManager.setSelection( visimpl::TGIDSet{ 0 } , testSetPositions );

  BOOST_CHECK_EQUAL( dManager.getBoundingBox( ).first.x , 1.0f );
  BOOST_CHECK_EQUAL( dManager.getBoundingBox( ).second.x , 1.0f );

  // Add group from selection
  dManager.createGroupFromSelection( testSetPositions , "selection" );
  BOOST_CHECK_EQUAL( dManager.getGroupAmount( ) , 1 );
  BOOST_CHECK_NE( dManager.getGroup( "selection" ) , nullptr );
  BOOST_CHECK_EQUAL( dManager.getGroup( "selection" )->getGids( ).size( ) , 1 );

  dManager.removeGroup( "selection" );
  BOOST_CHECK_EQUAL( dManager.getGroupAmount( ) , 0 );


  dManager.setSelection( visimpl::TGIDSet{ } , visimpl::tGidPosMap{ } );
  BOOST_CHECK_EQUAL( dManager.getBoundingBox( ).first.x , maxLimit );
  BOOST_CHECK_EQUAL( dManager.getBoundingBox( ).second.x , minLimit );

  // Check group addition
  dManager.createGroup( { } , { } , "test_group" );
  BOOST_CHECK_EQUAL( dManager.getGroupAmount( ) , 1 );

  dManager.removeGroup( "invalid_group" );
  BOOST_CHECK_EQUAL( dManager.getGroupAmount( ) , 1 );

  dManager.removeGroup( "test_group" );
  BOOST_CHECK_EQUAL( dManager.getGroupAmount( ) , 0 );

  dManager.setMode( visimpl::VisualMode::Groups );
  BOOST_CHECK_EQUAL(
    static_cast<int>(dManager.getMode( )) ,
    static_cast<int>(visimpl::VisualMode::Groups)
  );

  // Check time
  // We have to add a group to test the time.
  dManager.createGroup( { } , { } , "test_group" );
  {
    auto group = dManager.getGroup( "test_group" );
    BOOST_CHECK_EQUAL( group->getModel( )->getTime( ) , 0.0f );
    dManager.addTime( 20.0f , 40.0f );
    BOOST_CHECK_EQUAL( group->getModel( )->getTime( ) , 20.0f );
    dManager.addTime( 20.0f , 40.0f ); // Time is cyclic!
    BOOST_CHECK_EQUAL( group->getModel( )->getTime( ) , 0.0f );
    dManager.addTime( 20.0f , 40.0f );
    BOOST_CHECK_EQUAL( group->getModel( )->getTime( ) , 20.0f );
    dManager.setTime( 25.0f );
    BOOST_CHECK_EQUAL( group->getModel( )->getTime( ) , 25.0f );
  }
  dManager.removeGroup( "test_group" );

  // Test drawing
  dManager.setMode( visimpl::VisualMode::Groups );
  dManager.createGroup( testSet , testSetPositions , "test_group" );
  dManager.applySolidShader( );
  dManager.draw( );
  dManager.applyDefaultShader( );
  dManager.draw( );

  test_utils::terminateOpenGLContext();
}

BOOST_AUTO_TEST_CASE( visimpl_domain_manager_spikes )
{
  test_utils::initOpenGLContext( );
  visimpl::DomainManager dManager;

  auto camera = std::make_shared< visimpl::Camera >( );
  visimpl::GIDUSet testSet{ 0 };
  visimpl::tGidPosMap testSetPositions{{ 0 , { 1.0f , 0.0f , 0.0f }}};

  dManager.initRenderers( nullptr , nullptr , camera );
  dManager.setSelection( testSet , testSetPositions );
  BOOST_CHECK_EQUAL( dManager.getSelectionCluster( )->size( ) , 1 );

  simil::Spikes spikes;
  spikes.emplace_back( 1.0f , 0 );
  simil::SpikesCRange range( spikes.cbegin( ) , spikes.cend( ));

  dManager.processInput( range , false );
  auto map = dManager.getSelectionCluster( )->mapData( );
  BOOST_CHECK_EQUAL( map->timestamp , 1.0f );
  dManager.getSelectionCluster( )->unmapData( );

  dManager.setMode( visimpl::VisualMode::Groups );
  dManager.createGroup( testSet , testSetPositions , "test_group" );
  dManager.processInput( range , false );
  {
    auto groupCluster = dManager.getGroup( "test_group" )->getCluster( );
    map = groupCluster->mapData( );
    BOOST_CHECK_EQUAL( map->timestamp , 1.0f );
    groupCluster->unmapData( );
  }

  test_utils::terminateOpenGLContext();
}