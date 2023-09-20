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
// Created by grial on 18/11/22.
//

#define BOOST_TEST_MODULE scoop_color_palette

#include <boost/test/unit_test.hpp>
#include <scoop/scoop.h>

#include <iostream>

BOOST_AUTO_TEST_CASE( scoop_color_palette_interpolate )
{
  scoop::ColorPalette palette = scoop::ColorPalette::colorBrewerDiverging(
    scoop::ColorPalette::ColorBrewerDiverging::BrBG ,
    3 ,
    true
  );

  BOOST_CHECK_EQUAL( palette.size( ) , 3 );
  BOOST_CHECK_EQUAL( palette.colors( )[ 0 ] ,
                     scoop::Color( 216 , 179 , 101 , 255 ));
}