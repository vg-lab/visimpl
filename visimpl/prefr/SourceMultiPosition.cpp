/*
 * @file  SourceMultiPosition.cpp
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */
#include "SourceMultiPosition.h"

namespace visimpl
{
  SourceMultiPosition::SourceMultiPosition( void )
  : Source( -1, glm::vec3( 0, 0, 0))
  , _positions( nullptr )
  { }

  SourceMultiPosition::~SourceMultiPosition( void )
  { }

  void SourceMultiPosition::setIdxTranslation( const tIdxTransMap& idxTranslation )
  {
    assert( !idxTranslation.empty( ));
    _idxTranslate = &idxTranslation;
  }

  void SourceMultiPosition::setPositions( const tGidPosMap& positions_ )
  {
    assert( !positions_.empty( ) );
    _positions = &positions_;
  }

  void SourceMultiPosition::removeElements( const prefr::ParticleSet& indices )
  {
    _particles.removeIndices( indices );
    restart( );
  }

  vec3 SourceMultiPosition::position( unsigned int idx )
  {
    auto partId = _idxTranslate->find( idx );
    assert( partId != _idxTranslate->end( ));

    auto pos = _positions->find( partId->second );
    assert( pos != _positions->end( ));

    return pos->second;
  }
}
