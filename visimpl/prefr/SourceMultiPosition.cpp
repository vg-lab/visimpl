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

#include "SourceMultiPosition.h"

namespace visimpl
{
  SourceMultiPosition::SourceMultiPosition( void )
  : Source( -1, glm::vec3( 0, 0, 0))
  , _positions( nullptr )
  , _idxTranslate( nullptr )
  { }

  void SourceMultiPosition::setIdxTranslation( const tUintUMap& idxTranslation )
  {
    _idxTranslate = &idxTranslation;
  }

  void SourceMultiPosition::setPositions( const tGidPosMap& positions_ )
  {
    _positions = &positions_;
  }

  void SourceMultiPosition::removeElements( const prefr::ParticleSet& indices )
  {
    _particles.removeIndices( indices );
    restart( );
  }

  vec3 SourceMultiPosition::position( unsigned int idx )
  {
    assert( !_idxTranslate->empty( ));
    assert( !_positions->empty( ) );

    auto partId = _idxTranslate->find( idx );
    assert( partId != _idxTranslate->end( ));

    auto pos = _positions->find( partId->second );
    assert( pos != _positions->end( ));

    return pos->second;
  }
}
