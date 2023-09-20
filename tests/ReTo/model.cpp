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

//
// Created by gaeqs on 9/11/22.
//

#define BOOST_TEST_MODULE reto_model

#include <reto/reto.h>
#include <boost/test/unit_test.hpp>
#include <reto_test_data.h>

using namespace reto;

BOOST_AUTO_TEST_CASE( reto_model_parse )
{
  ObjParser obj;
  Model m = obj.loadObj( OBJ_MODEL_TEST_DATA , true );
  BOOST_CHECK_EQUAL( m.vertices.size( ) , 8 * 3 * 3 );
  BOOST_CHECK_EQUAL( m.normals.size( ) , 8 * 3 * 3 );
  BOOST_CHECK_EQUAL( m.texCoords.size( ) , 4 * 6 * 2 );
  BOOST_CHECK_EQUAL( m.indices.size( ) , 36 );
  BOOST_CHECK_EQUAL( m.tangents.size( ) , 8 * 3 * 3 );
  BOOST_CHECK_EQUAL( m.bitangents.size( ) , 8 * 3 * 3 );
}