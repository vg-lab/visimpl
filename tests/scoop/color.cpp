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

#define BOOST_TEST_MODULE scoop_color

#include <boost/test/unit_test.hpp>
#include <visimpl_test_utils.h>
#include <scoop/scoop.h>

BOOST_AUTO_TEST_CASE( scoop_color_rgb )
{
  scoop::Color color1(100, 0, 0);
  scoop::Color color2(0, 100, 0);

  // RGBA returns ARGB because Qt hates us.
  BOOST_CHECK_EQUAL(color1.rgba(), 0xFF640000);
  BOOST_CHECK_EQUAL(color2.rgba(), 0xFF006400);
}