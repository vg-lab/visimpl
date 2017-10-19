/*
 * @file  DataSource.cpp
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */

#include "InputMultiplexer.h"

#include "prefr/ValuedSource.h"

namespace visimpl
{
  static float invRGBInt = 1.0f / 255;

  InputMultiplexer::InputMultiplexer( prefr::ParticleSystem* particleSystem,
                                      const TGIDSet& gids)
  : _particleSystem( particleSystem )
  , _gids( gids )
  , _base( nullptr )
  , _off( nullptr )
  , _showGroups( false )
  {
    auto cluster =
        particleSystem->clusters( ).begin( );
    for( auto gid : _gids )
    {
      _neuronClusters.insert( std::make_pair( gid, *cluster ));
      _clusterNeurons.insert( std::make_pair( *cluster, gid ));

      ++cluster;
    }

  }

  InputMultiplexer::~InputMultiplexer( )
  {

  }

  VisualGroup* InputMultiplexer::addVisualGroup( const GIDUSet& gids,
                                                 const std::string& name,
                                                 bool overrideGIDS )
  {
    VisualGroup* group = new VisualGroup( name );

    prefr::ColorOperationModel* model =
        new prefr::ColorOperationModel( *_base );

    group->_active = true;
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
        scoop::ColorPalette::colorBrewerQualitative(
            ( scoop::ColorPalette::ColorBrewerQualitative::Set1 ), 9 );

    auto colors = palette.colors( );

//    float invSize = 1.0f / ( colors.size( ) - 1 );


    std::cout << "Created color: ";
    TTransferFunction colorVariation;

    QColor color = colors[ _groups.size( ) ];

    glm::vec4 baseColor( color.red( ) * invRGBInt,
                         color.green( ) * invRGBInt,
                         color.blue( ) * invRGBInt, 0.6f );

    color = QColor( baseColor.r * 255,
                    baseColor.g * 255,
                    baseColor.b * 255,
                    baseColor.a * 255 );

    glm::vec4 darkColor =
        ( baseColor * 0.4f ) + ( glm::vec4( 0.1f, 0.1f, 0.1f, 0.4f ) * 0.6f );

    QColor darkqColor = QColor( darkColor.r * 255,
                                darkColor.g * 255,
                                darkColor.b * 255,
                                darkColor.a * 255 );

//    colorVariation.push_back( std::make_pair( 0.0f, darkqColor));
    colorVariation.push_back( std::make_pair( 0.0f, color ));
//    colorVariation.push_back( std::make_pair( 0.2f, color ));

//    colorVariation.push_back( std::make_pair( 0.7f, color ));

    colorVariation.push_back( std::make_pair( 1.0f, darkqColor));

    group->colorMapping( colorVariation );

    _groups.push_back( group );

    return group;
  }

  void InputMultiplexer::showGroups( bool show )
  {
    _showGroups = show;

    update( );
  }

  void InputMultiplexer::update( void )
  {
    if( _showGroups )
      updateGroups( );
    else
      updateSelection( );
  }

  bool InputMultiplexer::showGroups( void )
  {
    return _showGroups;
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

        cluster->second->model( state ? group->model( ) : _off);
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
          ( group->second->active( ) ? group->second->model( ) : _off ) :
          _off );
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

  void InputMultiplexer::processInput( const simil::SpikesCRange& spikes_,
                                            float begin, float end, bool /*clear*/)
  {
//    if( clear )
//      resetParticles( );

    if( _showGroups )
      processFrameInputGroups( spikes_, begin, end );
    else
      processFrameInputSelection( spikes_, begin, end );
  }

  InputMultiplexer::TModifiedNeurons
  InputMultiplexer::parseInput( const simil::SpikesCRange& spikes_,
                                float /*begin*/, float end )
  {
    TModifiedNeurons result;
    std::unordered_map< uint32_t, unsigned int > inserted;

    auto current = result.begin( );
    for( simil::SpikesCIter spike = spikes_.first; spike != spikes_.second; ++spike )
    {
      float lifeValue = _decayValue - ( end - spike->first);

      auto it = inserted.find( spike->second );
      if( it == inserted.end( ))
      {
        inserted.insert( std::make_pair( spike->second, result.size( )));
//        *current = std::make_tuple( spike->second, lifeValue );
        result.emplace_back( spike->second, lifeValue );

        ++current;
      }
    }

    return result;
  }

  void InputMultiplexer::processFrameInputGroups( const simil::SpikesCRange& spikes_,
                                                  float begin, float end )
  {

    auto state = parseInput( spikes_, begin, end );

    for( const auto& neuron : state )
    {
      auto gid = std::get< 0 >( neuron );

      auto visualGroup = _neuronGroup.find( gid );

      auto cluster = _neuronClusters.find( gid );

      assert( cluster != _neuronClusters.end( ));

      auto particleRange = cluster->second->particles( );

      if( visualGroup != _neuronGroup.end( ) && visualGroup->second->active( ))
      {
        dynamic_cast< prefr::ValuedSource* >( cluster->second->source( ))->particlesLife( std::get< 1 >( neuron ));
      }


    }

//    for( simil::SpikesCIter spike = spikes_.first; spike != spikes_.second; ++spike )
//    {
//      auto visualGroup = _neuronGroup.find( spike->second );
//
//      auto cluster = _neuronClusters.find( spike->second );
//
//      assert( cluster != _neuronClusters.end( ));
//
//      if( visualGroup != _neuronGroup.end( ) && visualGroup->second->active( ))
//        cluster->second->killParticles( );
//    }
  }

  void InputMultiplexer::processFrameInputSelection( const simil::SpikesCRange& spikes_,
                                                     float begin, float end )
  {
    auto state = parseInput( spikes_, begin, end );

    for( const auto& neuron : state )
    {
      auto gid = std::get< 0 >( neuron );

      auto cluster = _neuronClusters.find( gid );

      assert( cluster != _neuronClusters.end( ));

      auto particleRange = cluster->second->particles( );

      if( _selection.empty( ) || _selection.find( gid ) != _selection.end( ))
      {
        dynamic_cast< prefr::ValuedSource* >( cluster->second->source( ))->particlesLife( std::get< 1 >( neuron ));
      }


    }

//    for( simil::SpikesCIter spike = spikes_.first; spike != spikes_.second; ++spike )
//    {
//      if( _selection.empty( ) || _selection.find( spike->second ) != _selection.end( ))
//      {
//        auto cluster = _neuronClusters.find( spike->second );
//
//        assert( cluster != _neuronClusters.end( ));
//
//        cluster->second->killParticles( );
//      }
//    }
  }

  void InputMultiplexer::selection( const GIDUSet& newSelection )
  {
    _selection = newSelection;

    if( !_showGroups )
      updateSelection( );
  }

  const GIDUSet& InputMultiplexer::selection( void )
  {
    return _selection;
  }

  void InputMultiplexer::models( prefr::ColorOperationModel* main,
                                 prefr::ColorOperationModel* off )
  {
    _base = main;
    _off = off;
  }

  std::vector< prefr::Cluster* > InputMultiplexer::activeClusters( void )
  {
    std::vector< prefr::Cluster* > result;

    if( _showGroups )
    {
      for( auto group : _groups )
      {
        if( !group->active( ))
          continue;

        for( auto gid : group->gids( ))
        {
          auto it = _neuronGroup.find( gid );
          if( it != _neuronGroup.end( ) && group == it->second )
          {
            auto cluster = _neuronClusters.find( gid );
            result.push_back( cluster->second );
          }

        }
      }
    }
    else
    {
      result.reserve( _selection.size( ));
      for( auto gid : _selection )
      {
        auto cluster = _neuronClusters.find( gid );
        result.push_back( cluster->second );
      }
    }

    return result;
  }


  void InputMultiplexer::decay( float decayValue )
  {
    _decayValue = decayValue;
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

//      cluster->source( )->restart( );
    }

    _particleSystem->run( true );

    _particleSystem->update( 0.0f );
  }

  const std::vector< VisualGroup* >& InputMultiplexer::groups( void ) const
  {
    return _groups;
  }

}
