/*
 * @file  DataSource.cpp
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */

#include "InputMultiplexer.h"

namespace visimpl
{
  InputMultiplexer::InputMultiplexer( prefr::ParticleSystem* particleSystem,
                                      const std::unordered_map< uint32_t, prefr::Cluster* >& reference)
  : _particleSystem( particleSystem )
  , _base( nullptr )
  , _off( nullptr )
  , _showGroups( false )
  {
    _neuronClusters = reference;
    for( auto ref : reference )
      _clusterNeurons.insert( std::make_pair( ref.second, ref.first ));
  }

  InputMultiplexer::~InputMultiplexer( )
  {

  }

  void InputMultiplexer::addVisualGroup( const GIDUSet& gids, bool overrideGIDS )
  {
    VisualGroup* group = new VisualGroup( );

    prefr::ColorOperationModel* model =
        new prefr::ColorOperationModel( *_base );

    group->model( model );
    group->gids( gids );

    unsigned int counter = 0;
    for( auto gid : gids )
    {
      auto reference = _neuronGroup.find( gid );

      if( overrideGIDS || reference == _neuronGroup.end( ))
      {
        _neuronGroup[ gid ] = group;
        ++counter;
      }
    }

    if( counter == 0 )
    {
      std::cout << "Warning: This group has no exclusive GIDs so nothing will be shown" << std::endl;
    }

    scoop::ColorPalette palette =
        scoop::ColorPalette::colorBrewerSequential(
            ( scoop::ColorPalette::ColorBrewerSequential )( _groups.size( ) % 9 ), 4 );

    auto colors = palette.colors( );

    float invSize = 1.0f / colors.size( );


    TTransferFunction colorVariation;
    unsigned int i = 0;
    for( auto c : colors )
    {
      colorVariation.push_back( std::make_pair( i * invSize, c ));
      ++i;
    }

    group->colorMapping( colorVariation );

    _groups.push_back( group );


  }

  void InputMultiplexer::showGroups( bool show )
  {
    _showGroups = show;

    if( _showGroups )
      updateGroups( );
    else
      updateSelection( );
  }

  void InputMultiplexer::setVisualGroupState( unsigned int i, bool state )
  {
    assert( i < _groups.size( ));

    VisualGroup* group = _groups[ i ];

    group->_active = state;

    if( _showGroups )
    {
      for( auto gid : group->gids( ))
      {
        auto cluster = _neuronClusters.find( gid );

        cluster->second->model( state ? group->model( ) : _base );
      }
    }
    else
    {
      for( auto gid : group->gids( ))
      {
        auto cluster = _neuronClusters.find( gid );

        auto res = _selection.find( gid );

        cluster->second->model( res != _selection.end( ) ? _base : _off );

      }
    }

  }

  void InputMultiplexer::updateGroups( void )
  {

#ifdef VISIMPL_USE_OPENMP
    #pragma omp parallel for
    for( int i = 0; i < ( int )_particleSystem->clusters( ).size( ); i++ )
    {
      prefr::Cluster* cluster = _particleSystem->clusters( )[ i ];
#else
    for( auto cluster : _particleSystem->clusters( ))
    {
#endif
      auto gid = _clusterNeurons.find( cluster );

      auto group = _neuronGroup.find( gid->second );

      cluster->model( group != _neuronGroup.end( ) ?
          ( group->second->active( ) ? group->second->model( ) : _base ) :
          _base );
    }
  }

  void InputMultiplexer::updateSelection( void )
  {
    if( _selection.size( ) > 0 )
    {

#ifdef VISIMPL_USE_OPENMP
    #pragma omp parallel for
    for( int i = 0; i < ( int )_particleSystem->clusters( ).size( ); i++ )
    {
      prefr::Cluster* cluster = _particleSystem->clusters( )[ i ];
#else
    for( auto cluster : _particleSystem->clusters( ))
    {
#endif
        auto gid = _clusterNeurons.find( cluster );

        auto selected = _selection.find( gid->second );

        cluster->model( selected != _selection.end( ) ? _base : _off );
      }
    }
    else
    {
      for( auto& cluster : _particleSystem->clusters( ))
        cluster->model( _base );
    }
  }

  void InputMultiplexer::processFrameInput( const simil::SpikesCRange& spikes_ )
  {
    if( _showGroups )
      processFrameInputGroups( spikes_ );
    else
      processFrameInputSelection( spikes_ );
  }

  void InputMultiplexer::processFrameInputGroups( const simil::SpikesCRange& spikes_ )
  {
    for( simil::SpikesCIter spike = spikes_.first; spike != spikes_.second; ++spike )
    {
      auto visualGroup = _neuronGroup.find( spike->second );

      assert( visualGroup != _neuronGroup.end( ));

      auto cluster = _neuronClusters.find( spike->second );

      assert( cluster != _neuronClusters.end( ));

      if( visualGroup->second->active( ))
        cluster->second->killParticles( );
    }
  }

  void InputMultiplexer::processFrameInputSelection( const simil::SpikesCRange& spikes_ )
  {
    for( simil::SpikesCIter spike = spikes_.first; spike != spikes_.second; ++spike )
    {
      if( _selection.empty( ) || _selection.find( spike->second ) != _selection.end( ))
      {
        auto cluster = _neuronClusters.find( spike->second );

        assert( cluster != _neuronClusters.end( ));

        cluster->second->killParticles( );
      }
    }
  }

  void InputMultiplexer::selection( const GIDUSet& newSelection )
  {
    _selection = newSelection;

    if( !_showGroups )
      updateSelection( );
  }

  void InputMultiplexer::clearSelection( void )
  {
    _selection.clear( );

    if( !_showGroups )
      updateSelection( );
  }

  void InputMultiplexer::resetParticles( void )
  {
    _particleSystem->run( false );

#ifdef VISIMPL_USE_OPENMP
    #pragma omp parallel for
    for( int i = 0; i < ( int )_particleSystem->clusters( ).size( ); i++ )
    {
      prefr::Cluster* cluster = _particleSystem->clusters( )[ i ];
#else
    for( auto cluster : _particleSystem->clusters( ))
    {
#endif


      cluster->killParticles( false );

      cluster->source( )->restart( );
    }

    _particleSystem->run( true );

    _particleSystem->update( 0.0f );
  }

}
