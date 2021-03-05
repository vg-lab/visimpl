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

#include "ColorSource.h"

namespace prefr
{

  ColorSource::ColorSource( float emissionRate,
                            const glm::vec3& position_,
                            const glm::vec4& color_,
                            bool still_ )
  : Source( emissionRate, position_ )
  , _color( color_ )
  , _size ( 0.0f )
  , _still( still_ )
  {}

  void ColorSource::color( const glm::vec4& color_ )
  {
    _color = color_;
  }

  const glm::vec4& ColorSource::color( void )
  {
    return _color;
  }

  void ColorSource::still( bool still_ )
  {
    _still = still_;
  }

  bool ColorSource::still( void )
  {
    return _still;
  }

  void ColorSource::size( float size_ )
  {
    _size = size_;
  }

  float ColorSource::size( void )
  {
    return _size;
  }

  bool ColorSource::emits( void ) const
  {
    return true;
  }
}
