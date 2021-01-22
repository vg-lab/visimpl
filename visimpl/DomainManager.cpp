/*
 * Copyright (c) 2015-2020 GMRV/URJC.
 *
 * Authors: Sergio E. Galindo <sergio.galindo@urjc.es>
 *
 * This file is part of ViSimpl <https://github.com/gmrvvis/visimpl>
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

// ViSimpl
#include "DomainManager.h"
#include "VisualGroup.h"

// Prefr
#include "prefr/UpdaterStaticPosition.h"
#include "prefr/SourceMultiPosition.h"

// C++
#include <exception>

namespace visimpl
{
  void expandBoundingBox( glm::vec3& minBounds,
                          glm::vec3& maxBounds,
                          const glm::vec3& position)
  {
    for( unsigned int i = 0; i < 3; ++i )
    {
      minBounds[ i ] = std::min( minBounds[ i ], position[ i ] );
      maxBounds[ i ] = std::max( maxBounds[ i ], position[ i ] );
    }
  }

  static float invRGBInt = 1.0f / 255;

  static std::unordered_map< std::string, std::string > _attributeNameLabels =
  {
    {"PYR", "Pyramidal"}, {"INT", "Interneuron"},
    {"EXC", "Excitatory"}, {"INH", "Inhibitory"}
  };


  DomainManager::DomainManager( prefr::ParticleSystem* particleSystem,
                                      const TGIDSet& gids)
  : _particleSystem( particleSystem )
  , _gids( gids )
  , _clusterSelected( nullptr )
  , _clusterUnselected( nullptr )
  , _clusterHighlighted( nullptr )
  , _sourceSelected( nullptr )
  , _currentAttrib( T_TYPE_UNDEFINED )
  , _modelBase( nullptr )
  , _modelOff( nullptr )
  , _modelHighlighted( nullptr )
  , _sampler( nullptr )
  , _updater( nullptr )
  , _mode( TMODE_SELECTION )
  , _decayValue( 0.0f )
  , _showInactive( true )
  , _groupByName( false )
  , _autoGroupByName( true )
  {
  }

  void DomainManager::init( const tGidPosMap& positions
#ifdef SIMIL_USE_BRION
                            , const brion::BlueConfig* blueConfig
#endif
  )
  {
    _gidPositions = positions;

#ifdef SIMIL_USE_BRION
    if( blueConfig )
      _gidTypes = _loadNeuronTypes( *blueConfig );
#endif


    _sourceSelected = new SourceMultiPosition( );

    _sourceSelected->setPositions( _gidPositions );

    _clusterSelected = new prefr::Cluster( );
    _clusterUnselected = new prefr::Cluster( );
    _clusterHighlighted = new prefr::Cluster( );

    _particleSystem->addCluster( _clusterSelected );
    _particleSystem->addCluster( _clusterUnselected );
    _particleSystem->addCluster( _clusterHighlighted );

    _loadPaletteColors( );
  }

  void DomainManager::initializeParticleSystem( void )
  {
    std::cout << "Initializing particle system..." << std::endl;

    _updater = new UpdaterStaticPosition( );

    prefr::Sorter* sorter = new prefr::Sorter( );
    prefr::GLRenderer* renderer = new prefr::GLPickRenderer( );

    _particleSystem->addUpdater( _updater );
    _particleSystem->sorter( sorter );
    _particleSystem->renderer( renderer );

    _modelOff = new prefr::ColorOperationModel( _decayValue, _decayValue );
    _modelOff->color.Insert( 0.0f, ( glm::vec4(0.1f, 0.1f, 0.1f, 0.2f)));

    _modelOff->velocity.Insert( 0.0f, 0.0f );

    _modelOff->size.Insert( 1.0f, 10.0f );

    _particleSystem->addModel( _modelOff );

    _modelHighlighted = new prefr::ColorOperationModel( _decayValue, _decayValue );
    _modelHighlighted->color.Insert( 0.0f, ( glm::vec4( 0.9f, 0.9f, 0.9f, 0.5f )));
    _modelHighlighted->color.Insert( 1.0f, ( glm::vec4( 0.75f, 0.75f, 0.75f, 0.2f )));
    _modelHighlighted->velocity.Insert( 0.0f, 0.0f );
    _modelHighlighted->size.Insert( 0.0f, 20.0f );
    _modelHighlighted->size.Insert( 1.0f, 10.0f );
    _particleSystem->addModel( _modelHighlighted );


    _modelBase = new prefr::ColorOperationModel( _decayValue, _decayValue );
    _modelBase->color.Insert( 0.0f, ( glm::vec4(0.f, 1.f, 0.f, 0.05)));
    _modelBase->color.Insert( 0.35f, ( glm::vec4(1, 0, 0, 0.2 )));
    _modelBase->color.Insert( 0.7f, ( glm::vec4(1.f, 1.f, 0, 0.2 )));
    _modelBase->color.Insert( 1.0f, ( glm::vec4(0, 0, 1.f, 0.2 )));

    _modelBase->velocity.Insert( 0.0f, 0.0f );

    _modelBase->size.Insert( 0.0f, 20.0f );
    _modelBase->size.Insert( 1.0f, 10.0f );

    _particleSystem->addModel( _modelBase );

    _sampler = new prefr::PointSampler( );

    _particleSystem->renderDeadParticles( true );
    _particleSystem->parallel( true );

    _particleSystem->start();
  }

  const tGidPosMap& DomainManager::positions( void ) const
  {
    return _gidPositions;
  }

  void DomainManager::reloadPositions( void )
  {
    _resetBoundingBox( );

    for( auto gidPartId : _gidToParticle )
    {
      auto pos = _gidPositions.find( gidPartId.first );

      auto particle = _particleSystem->particles( ).at( gidPartId.second );
      particle.set_position( pos->second );

      expandBoundingBox( _boundingBox.first,
                         _boundingBox.second,
                         pos->second );
    }
  }

  const TGIDSet& DomainManager::gids( void ) const
  {
    return _gids;
  }

  void DomainManager::mode( tVisualMode newMode )
  {
    clearView( );

    _mode = newMode;

    switch( _mode )
    {
      case TMODE_SELECTION:
        _generateSelectionIndices( );
        break;
      case TMODE_GROUPS:
        _generateGroupsIndices( );
        break;
      case TMODE_ATTRIBUTE:
        generateAttributesGroups( _currentAttrib == T_TYPE_UNDEFINED ?
                                  T_TYPE_MORPHO : _currentAttrib );

        _generateAttributesIndices( );
        break;
      default:
        {
          const auto message = std::string("Invalid tVisualMode value ") +
                               std::to_string(static_cast<int>(newMode)) + " " +
                               std::string(__FILE__) + ":" +
                               std::to_string(__LINE__);
          throw std::out_of_range(message.c_str());
        }
        break;
    }
  }

  tVisualMode DomainManager::mode( void )
  {
    return _mode;
  }

  void DomainManager::clearView( void )
  {
    switch( _mode )
    {
      case TMODE_SELECTION:
        _clearSelectionView( );
        break;
      case TMODE_GROUPS:
        _clearGroupsView( );
        break;
      case TMODE_ATTRIBUTE:
        _clearAttribView( );
        break;
      default:
        {
          const auto message = std::string("Invalid tVisualMode value ") +
                               std::to_string(static_cast<int>(_mode)) + " " +
                               std::string(__FILE__) + ":" +
                               std::to_string(__LINE__);
          throw std::out_of_range(message.c_str());
        }
        break;
    }
  }

  void DomainManager::_clearParticlesReference( void )
  {
    _gidToParticle.clear( );
    _particleToGID.clear( );

    _gidSource.clear( );
  }

  void DomainManager::_clearSelectionView( void )
  {
    if(_sourceSelected )
      _particleSystem->detachSource( _sourceSelected );

    _clearParticlesReference( );
  }

  void DomainManager::_clearGroupsView( void )
  {
    for( auto group : _groups )
    {
      _clearGroup( group, false );
    }

    _clearParticlesReference( );
  }

  void DomainManager::_clearAttribView( void )
  {
    for( auto group : _attributeGroups )
    {
      _clearGroup( group, false );
    }

    _clearParticlesReference( );
  }

  void DomainManager::_clearGroups( void )
  {
    for( auto group : _groups )
      delete group;

    _groups.clear( );
  }

  void DomainManager::_clearAttribs( bool clearCustom )
  {
    if( clearCustom )
    {
      for( auto group : _attributeGroups )
        delete group;

      _attributeGroups.clear( );
    }
    else
    {
      std::vector< VisualGroup* > aux;
      aux.reserve( _attributeGroups.size( ));
      for( auto group : _attributeGroups )
        if( group->custom( ))
          aux.push_back( group );
        else
          delete group;
      aux.shrink_to_fit( );
      _attributeGroups = aux;
    }
  }

  void DomainManager::_loadPaletteColors( void )
  {
    scoop::ColorPalette palette =
        scoop::ColorPalette::colorBrewerQualitative(
            ( scoop::ColorPalette::ColorBrewerQualitative::Set1 ), 9 );

    auto colors = palette.colors( );

    constexpr float brightFactor = 0.4f;
    constexpr float darkFactor = 1.0f - brightFactor;

    _paletteColors.clear( );

    for( auto color: colors )
    {
      glm::vec4 baseColor( color.red( ) * invRGBInt,
                           color.green( ) * invRGBInt,
                           color.blue( ) * invRGBInt, 0.6f );

      color = QColor( baseColor.r * 255,
                      baseColor.g * 255,
                      baseColor.b * 255,
                      baseColor.a * 255 );

      glm::vec4 darkColor =
          ( baseColor * brightFactor ) + ( glm::vec4( 0.1f, 0.1f, 0.1f, 0.4f ) * darkFactor );

      QColor darkqColor = QColor( darkColor.r * 255,
                                  darkColor.g * 255,
                                  darkColor.b * 255,
                                  darkColor.a * 255 );

      _paletteColors.emplace_back( std::make_pair( color, darkqColor ));
    }
  }

  void DomainManager::update( void )
  {
    switch( _mode )
    {
      case TMODE_SELECTION:
        _generateSelectionIndices( );
        break;
      case TMODE_GROUPS:
        _generateGroupsIndices( );
        break;
      case TMODE_ATTRIBUTE:
        _generateAttributesIndices( );
        break;
      default:
        {
          const auto message = std::string("Invalid tVisualMode value ") +
                               std::to_string(static_cast<int>(_mode)) + " " +
                               std::string(__FILE__) + ":" +
                               std::to_string(__LINE__);
          throw std::out_of_range(message.c_str());
        }
        break;
    }
  }

  void DomainManager::updateData(const TGIDSet& gids,
                                 const tGidPosMap& positions)
  {
    _gids = gids;
    _gidPositions = positions;

    _sourceSelected->setPositions( _gidPositions );
    clearView();
    reloadPositions();

    update();
  }

  void DomainManager::_resetBoundingBox( void )
  {
    _boundingBox.first = glm::vec3( std::numeric_limits< float >::max( ),
                                    std::numeric_limits< float >::max( ),
                                    std::numeric_limits< float >::max( ));

    _boundingBox.second = glm::vec3( std::numeric_limits< float >::min( ),
                                     std::numeric_limits< float >::min( ),
                                     std::numeric_limits< float >::min( ));
  }

  bool DomainManager::showGroups( void )
  {
    return _mode == TMODE_GROUPS;
  }

  void DomainManager::updateGroups( void )
  {
    _generateGroupsIndices( );
  }

  void DomainManager::updateAttributes( void )
  {
    _generateAttributesIndices( );
  }

  void DomainManager::showInactive( bool state )
  {
    _showInactive = state;
  }

  void DomainManager::setVisualGroupState( unsigned int i, bool state, bool attrib )
  {
    VisualGroup* group = nullptr;

    if( !attrib )
    {
      if( i < _groups.size( ))
        group = _groups[ i ];
    }
    else
    {
      if( i < _attributeGroups.size( ))
        group = _attributeGroups[ i ];
    }

    if(!group) return;

    group->active( state );
    group->cluster( )->setModel( state ? group->model( ) : _modelOff );

    if( !_showInactive )
      group->source( )->active( state );
  }


  void DomainManager::_updateGroupsModels( void )
  {
    for( auto group : _groups )
    {
      group->model( group->active( ) ? group->model( ) : _modelOff );
    }
  }

  void DomainManager::_updateSelectionIndices( void )
  {
    prefr::ParticleIndices selection;
    prefr::ParticleIndices other;
    for( auto gid : _gids )
    {
      auto particleId = _gidToParticle.find( gid )->second;

      if( _selection.empty( ) || _selection.find( gid ) != _selection.end( ))
        selection.push_back( particleId );
      else
        other.push_back( particleId );
    }
    _clusterSelected->particles( ).indices( selection );
    _clusterUnselected->particles( ).indices( other );

    _clusterSelected->setModel( _modelBase );
    _clusterUnselected->setModel( _modelOff );
  }

  void DomainManager::_generateSelectionIndices( void )
  {
    unsigned int numParticles = _gids.size( );

    prefr::ParticleIndices indices;
    prefr::ParticleIndices indicesSelected;
    prefr::ParticleIndices indicesUnselected;

    indices.reserve( numParticles );
    indicesSelected.reserve( numParticles );
    indicesUnselected.reserve( numParticles );

    auto availableParticles =  _particleSystem->retrieveUnused( numParticles );

    std::cout << "Retrieved " << availableParticles.size( ) << std::endl;

    _resetBoundingBox( );

    auto gidit = _gids.begin( );
    for( auto particle : availableParticles )
    {
      unsigned int id = particle.id( );

      // Create reference
      _gidToParticle.insert( std::make_pair( *gidit, id ));
      _particleToGID.insert( std::make_pair( id, *gidit ));

      _gidSource.insert( std::make_pair( *gidit, _sourceSelected ));

      // Check if part of selection
      if( _selection.empty( ) || _selection.find( *gidit ) != _selection.end( ))
      {
        indicesSelected.emplace_back( id );

        const auto pos = _gidPositions.find( *gidit )->second;
        expandBoundingBox( _boundingBox.first, _boundingBox.second, pos );
      }
      else
      {
        indicesUnselected.emplace_back( id );
      }

      indices.emplace_back( id );

      ++gidit;
    }

    indicesSelected.shrink_to_fit( );
    indicesUnselected.shrink_to_fit( );

    _clusterSelected->particles( ).indices( indicesSelected );
    _clusterUnselected->particles( ).indices( indicesUnselected );

    _sourceSelected->setIdxTranslation( _particleToGID );

    _particleSystem->addSource( _sourceSelected, indices );

    _clusterSelected->setUpdater( _updater );
    _clusterUnselected->setUpdater( _updater );

    _clusterUnselected->setModel( _modelOff );
    _clusterSelected->setModel( _modelBase );
  }

  VisualGroup* DomainManager::_generateGroup( const GIDUSet& gids,
                                              const std::string& name,
                                              unsigned int idx ) const
  {
    VisualGroup* group = new VisualGroup( name );

    // Create model from base
    prefr::ColorOperationModel* model =
        new prefr::ColorOperationModel( *_modelBase );
    group->model( model );

    TTransferFunction colorVariation;
    auto colors = _paletteColors[ idx % _paletteColors.size( )];
    colorVariation.push_back( std::make_pair( 0.0f, colors.first ));
    colorVariation.push_back( std::make_pair( 1.0f, colors.second ));
    group->colorMapping( colorVariation );

    prefr::Cluster* cluster = new prefr::Cluster( );

    SourceMultiPosition* source = new SourceMultiPosition( );
    source->setPositions( _gidPositions );
    source->setIdxTranslation( _particleToGID );

    group->cluster( cluster );
    group->source( source );

    group->gids( gids );

    group->active( true );
    group->cached( false );
    group->dirty( true );

    return group;
  }

  VisualGroup* DomainManager::addVisualGroupFromSelection( const std::string& name,
                                                           bool overrideGIDs )
  {
    return addVisualGroup( _selection, name, overrideGIDs );
  }

  VisualGroup* DomainManager::addVisualGroup( const GIDUSet& gids,
                                              const std::string& name,
                                              bool overrideGIDS )
  {
    VisualGroup* group = _generateGroup( gids, name, _groups.size( ));
    group->custom( true );

    if(overrideGIDS)
    {
      for( auto gid : gids )
      {
        auto reference = _neuronGroup.find( gid );

        if( reference != _neuronGroup.end( ))
        {
          auto& oldGroup = reference->second;

          auto oldGroupGIDs = oldGroup->gids( );
          oldGroupGIDs.erase( gid );
          reference->second->gids( oldGroupGIDs );

          oldGroup->cached( false );
        }
      }
    }
    _groups.push_back( group );

    return group;
  }

  void DomainManager::removeVisualGroup( unsigned int i )
  {
    auto group = _groups[ i ];

    _clearGroup( group, true );

    delete group;

    _groups.erase( _groups.begin( ) + i );
  }

  void DomainManager::_clearGroup( VisualGroup* group, bool clearState )
  {
    _particleSystem->detachSource( group->source( ));

    if( clearState )
    {
      for( auto gid : group->gids( ))
      {
        auto ref = _gidToParticle.find( gid );

        if( ref != _gidToParticle.end( ))
        {
        _particleToGID.erase( ref->second );
        _gidToParticle.erase( ref );
        }
        _neuronGroup.erase( gid );
      }
    }

    group->cached( false );
    group->dirty( true );
  }

  void DomainManager::_generateGroupsIndices( void )
  {
    for( auto group : _groups )
    {
      if( !group->dirty( ))
        continue;

      if( group->cached( ))
        _clearGroup( group, true );

      const auto& gids = group->gids( );

      auto availableParticles =  _particleSystem->retrieveUnused( gids.size( ));

      auto cluster = group->cluster( );

      _particleSystem->addCluster( cluster, availableParticles.indices( ));
      _particleSystem->addSource( group->source( ), availableParticles.indices( ));

      cluster->setUpdater( _updater );
      cluster->setModel( group->model( ));

      auto partId = availableParticles.begin( );
      for( auto gid : gids )
      {
        _neuronGroup.insert( std::make_pair( gid, group ));

        _gidToParticle.insert( std::make_pair( gid, partId.id( )));
        _particleToGID.insert( std::make_pair( partId.id( ), gid ));

        ++partId;
      }

      group->cached( true );
      group->dirty( false );
    }
  }

  void DomainManager::generateAttributesGroups( tNeuronAttributes attrib )
  {
    if( attrib == _currentAttrib || attrib == T_TYPE_UNDEFINED || _mode != TMODE_ATTRIBUTE  )
      return;

    _clearAttribView( );

    _clearAttribs( );

    _clearParticlesReference( );

    // Clustering
    // TODO auto-detect attribute type
    std::unordered_set< unsigned int > differentValues;
    std::unordered_multimap< unsigned int, unsigned int > valueGids;

    auto functor = ( [&]( const NeuronAttributes& att )
        { return ( attrib == T_TYPE_MORPHO ) ? ( std::get< T_TYPE_MORPHO >( att )) :
                                                 std::get< T_TYPE_FUNCTION >( att ); });

    const auto& typeIndices =
        ( attrib == T_TYPE_MORPHO ) ? _typeToIdxMorpho : _typeToIdxFunction;

    const auto& nameIndices =
        ( attrib == T_TYPE_MORPHO ) ? _namesIdxMorpho : _namesIdxFunction;

    for( auto attribs : _gidTypes )
    {
      unsigned int gid = attribs.first;
      unsigned int value = typeIndices.find( functor( attribs.second ))->second;

      differentValues.insert( value );
      valueGids.insert( std::make_pair( value, gid ));
    }

    _attributeGroups.resize( nameIndices.size( ));

    // Generate attrib groups
    for( auto typeIndex : nameIndices )
    {
      GIDUSet gids;

      auto elements = valueGids.equal_range( typeIndex.second );
      for( auto it = elements.first; it != elements.second; ++it )
        gids.insert( it->second );

      auto group = _generateGroup( gids, typeIndex.first, typeIndex.second );
      group->custom( false );

      _attributeGroups[ typeIndex.second ] = group;
    }

    _generateAttributesIndices( );

    _currentAttrib = attrib;
  }

  void DomainManager::_generateAttributesIndices( void )
  {
    for( auto group : _attributeGroups )
    {

      if( !group->dirty( ))
        continue;

      if( group->cached( ))
        _clearGroup( group, true );

      const auto& gids = group->gids( );

      auto availableParticles =  _particleSystem->retrieveUnused( gids.size( ));

      auto cluster = group->cluster( );

      _particleSystem->addCluster( cluster, availableParticles.indices( ));
      _particleSystem->addSource( group->source( ), availableParticles.indices( ));

      cluster->setUpdater( _updater );
      cluster->setModel( group->model( ));

      auto partId = availableParticles.begin( );
      for( auto gid : gids )
      {
        _neuronGroup.insert( std::make_pair( gid, group ));

        _gidToParticle.insert( std::make_pair( gid, partId.id( )));
        _particleToGID.insert( std::make_pair( partId.id( ), gid ));

        ++partId;
      }

      group->cached( true );
      group->dirty( false );
    }
  }

  void DomainManager::processInput( const simil::SpikesCRange& spikes_,
                                       float begin, float end, bool clear )
  {
    if( clear )
      resetParticles( );

    switch( _mode )
    {
      case TMODE_SELECTION:
        _processFrameInputSelection( spikes_, begin, end );
        break;
      case TMODE_GROUPS:
        _processFrameInputGroups( spikes_, begin, end );
        break;
      case TMODE_ATTRIBUTE:
        _processFrameInputAttributes( spikes_, begin, end );
        break;
      default:
        {
          const auto message = std::string("Invalid tVisualMode value ") +
                               std::to_string(static_cast<int>(_mode)) + " " +
                               std::string(__FILE__) + ":" +
                               std::to_string(__LINE__);
          throw std::out_of_range(message.c_str());
        }
        break;
    }
  }

  DomainManager::TModifiedNeurons
  DomainManager::_parseInput( const simil::SpikesCRange& spikes_,
                                float /*begin*/, float end )
  {
    TModifiedNeurons result;
    std::unordered_map< uint32_t, float > inserted;

    for( simil::SpikesCIter spike = spikes_.first; spike != spikes_.second; ++spike )
    {
      float lifeValue = _decayValue - ( end - spike->first);

      auto it = inserted.find( spike->second );
      if( it == inserted.end( ))
      {
        inserted[ spike->second ] = lifeValue;
      }
    }

    result.reserve( inserted.size( ));
    for( auto spike : inserted )
    {
      result.emplace_back( spike.first, spike.second );
    }

    return result;
  }

  void DomainManager::_processFrameInputSelection( const simil::SpikesCRange& spikes_,
                                                     float begin, float end )
  {
    if( !_particleSystem || !_particleSystem->run( ) || _mode != TMODE_SELECTION )
      return;

    auto state = _parseInput( spikes_, begin, end );

    for( const auto& neuron : state )
    {
      auto gid = std::get< 0 >( neuron );

      if( _selection.empty( ) || _selection.find( gid ) != _selection.end( ))
      {
        const auto source = _gidSource.find( gid );

        if( source == _gidSource.end( ))
        {
          std::cout << "GID " << gid << " source not found." << std::endl;
          return;
        }

        const unsigned int partIdx = _gidToParticle.find( gid )->second;
        auto particle = _particleSystem->particles( ).at( partIdx );
        particle.set_life( std::get< 1 >( neuron ));
      }
    }
  }

  void DomainManager::_processFrameInputGroups( const simil::SpikesCRange& spikes_,
                                                  float begin, float end )
  {

    if( !_particleSystem || !_particleSystem->run( ) || _mode != TMODE_GROUPS )
      return;

    auto state = _parseInput( spikes_, begin, end );

    for( const auto& neuron : state )
    {
      auto gid = std::get< 0 >( neuron );

      auto visualGroup = _neuronGroup.find( gid );

      if( visualGroup != _neuronGroup.end( ) && visualGroup->second->active( ))
      {
        auto partIt = _gidToParticle.find( gid );
        if( partIt != _gidToParticle.end( ))
        {
          unsigned int particleIndex = partIt->second;

          auto particle = _particleSystem->particles( ).at( particleIndex );
          particle.set_life( std::get< 1 >( neuron ) );
        }
      }
    }
  }

  void DomainManager::_processFrameInputAttributes( const simil::SpikesCRange& spikes_,
                                                  float begin, float end )
  {

    if( !_particleSystem || !_particleSystem->run( ) || _mode != TMODE_ATTRIBUTE )
      return;

    auto state = _parseInput( spikes_, begin, end );

    for( const auto& neuron : state )
    {
      auto gid = std::get< 0 >( neuron );

      auto visualGroup = _neuronGroup.find( gid );

      if( visualGroup != _neuronGroup.end( ) && visualGroup->second->active( ))
      {
        auto partIt = _gidToParticle.find( gid );
        if( partIt != _gidToParticle.end( ))
        {
          unsigned int particleIndex = partIt->second;
          auto particle = _particleSystem->particles( ).at( particleIndex );
          particle.set_life( std::get< 1 >( neuron ) );
        }
      }
    }
  }

  void DomainManager::selection( const GIDUSet& newSelection )
  {
    _selection = newSelection;

    if( _mode == TMODE_SELECTION )
    {
      _updateSelectionIndices( );
    }
  }

  const GIDUSet& DomainManager::selection( void )
  {
    return _selection;
  }

  tBoundingBox DomainManager::boundingBox( void ) const
  {
    tBoundingBox result = _boundingBox;

    if( _boundingBox.first == _boundingBox.second )
      result.second = _boundingBox.second + vec3( 0.1f, 0.1f, 0.1f );

    return result;
  }

  prefr::ColorOperationModel* DomainManager::modelSelectionBase( void )
  {
    return _modelBase;
  }

  void DomainManager::decay( float decayValue )
  {
    _decayValue = decayValue;

    _modelBase->setLife( decayValue, decayValue );
    _modelHighlighted->setLife( decayValue, decayValue );
  }

  float DomainManager::decay( void ) const
  {
    return _decayValue;
  }

  void DomainManager::clearSelection( void )
  {
    _selection.clear( );

    if( _mode == TMODE_SELECTION )
    {
      _updateSelectionIndices( );
    }
  }

  void DomainManager::resetParticles( void )
  {
    _particleSystem->run( false );

    auto particles = _particleSystem->retrieveActive( );
    for( int i = 0; i < ( int ) particles.size( ); ++i )
    {
      particles.at( i ).set_life( 0 );
    }

    _particleSystem->run( true );

    _particleSystem->update( 0.0f );
  }

  const std::vector< VisualGroup* >& DomainManager::groups( void ) const
  {
    return _groups;
  }

  const std::vector< VisualGroup* >& DomainManager::attributeGroups( void ) const
  {
    return _attributeGroups;
  }

  const std::vector< std::pair< QColor, QColor >>&
    DomainManager::paletteColors( void ) const
  {
    return _paletteColors;
  }

  const std::vector< std::string >& DomainManager::namesMorpho( void ) const
  {
    return _namesTypesMorpho;
  }
  const std::vector< std::string >& DomainManager::namesFunction( void ) const
  {
    return _namesTypesFunction;
  }

#ifdef SIMIL_USE_BRION
  tNeuronAttribs DomainManager::_loadNeuronTypes( const brion::BlueConfig& blueConfig )
  {
    tNeuronAttribs result;

    const auto& gids = _gids;

    try
    {
      brion::Circuit circuit( blueConfig.getCircuitSource( ));

      uint32_t attributes = brion::NEURON_COLUMN_GID |
                            brion::NEURON_MTYPE |
                            brion::NEURON_ETYPE;


      const brion::NeuronMatrix& attribsData = circuit.get( gids, attributes );

      _namesTypesMorpho = circuit.getTypes( brion::NEURONCLASS_MORPHOLOGY_CLASS );
      _namesTypesFunction = circuit.getTypes( brion::NEURONCLASS_FUNCTION_CLASS );

      _typesMorpho.reserve( gids.size( ));
      _typesFunction.reserve( gids.size( ));

      for( unsigned int i = 0; i < gids.size( ); ++i )
      {
        const unsigned int morphoType =
            boost::lexical_cast< unsigned int >( attribsData[ i ][ 1 ]);

        const unsigned int functionType =
            boost::lexical_cast< unsigned int >( attribsData[ i ][ 2 ]);

        _typesMorpho.push_back( morphoType );
        _typesFunction.push_back( functionType );
      }

    }
    catch( ... )
    {
      brain::Circuit circuit( blueConfig );
      _namesTypesMorpho = circuit.getMorphologyTypeNames( );
      _namesTypesFunction = circuit.getElectrophysiologyTypeNames( );

      auto transform = [](std::vector<unsigned long long> vec)
      {
          std::vector<unsigned long> vec32;
          vec32.reserve(vec.size());

          std::for_each(vec.cbegin(), vec.cend(), [&vec32](unsigned long long num) { vec32.emplace_back(static_cast<unsigned long>(num)); });

          return vec32;
      };

      _typesMorpho = transform(circuit.getMorphologyTypes( gids ));
      _typesFunction = transform(circuit.getElectrophysiologyTypes( gids ));

    }

    unsigned int counter = 0;

    for( auto gid : gids )
    {
      NeuronAttributes attribs;

      for( unsigned int i = 0; i < ( unsigned int ) T_TYPE_UNDEFINED; ++i )
      {
        unsigned int typeValue =
            (( i == 0 ) ? _typesMorpho : _typesFunction )[ counter ];

        auto& stats = (( i == 0 ) ? _statsMorpho : _statsFunction );

        auto& indexBack =
            ( i == 0 ) ? _idxToTypeMorpho : _idxToTypeFunction;

        auto attribFunctor =
            ( i == 0 ) ?  &std::get< T_TYPE_MORPHO >( attribs ) :
                           &std::get< T_TYPE_FUNCTION >( attribs );

        auto statsIt = stats.find( typeValue );
        if( statsIt == stats.end( ))
          statsIt = stats.insert( std::make_pair( typeValue, 0 )).first;

        ++statsIt->second;

        indexBack.insert( std::make_pair( i, typeValue ));

        *attribFunctor = typeValue;
      }

      ++counter;

      result.insert( std::make_pair( gid, attribs ));
    }


    if( _autoGroupByName )
    {
      for( unsigned int i = 0; i < ( unsigned int ) T_TYPE_UNDEFINED; ++i )
      {
        const auto& names = ( i == 0 ) ? _namesTypesMorpho : _namesTypesFunction;
        auto& groupedNameStorage =
            ( i == 0 ) ? _namesTypesMorphoGrouped : _namesTypesFunctionGrouped;

        auto& groupedIndices =
            ( i == 0 ) ? _typeToIdxMorpho : _typeToIdxFunction;

        auto& nameIndexer = ( i == 0 ) ? _namesIdxMorpho : _namesIdxFunction;

        const auto& stats = (( i == 0 ) ? _statsMorpho : _statsFunction );

        counter = 0;
        for( auto name : names )
        {

          if( stats.find( counter ) != stats.end( ))
          {

            auto nameIndex = nameIndexer.find( name );
            if( nameIndex == nameIndexer.end( ))
            {
              unsigned int index = nameIndexer.size( );
              nameIndex = nameIndexer.insert( std::make_pair( name, index )).first;
            }

            groupedIndices.insert( std::make_pair( counter, nameIndex->second ));
          }

          ++counter;

        }
        std::cout << std::endl;

        groupedNameStorage.resize( nameIndexer.size( ));

        for( auto name : nameIndexer )
        {
          groupedNameStorage[ name.second ] = name.first;
        }

        if( groupedNameStorage.size( ) != names.size( ))
          _groupByName = true;
      }
    }

//    std::cout << "Loaded attributes." << std::endl;
//    std::cout << "- Morphological: " << std::endl;
//    for( auto type : _statsMorpho )
//      std::cout << _namesTypesMorpho[ type.first ]
//                << " -> " << type.first
//                << " # " << type.second
//                << std::endl;
//
//    std::cout << " Grouped in: " << std::endl;
//    for( auto type : _namesIdxMorpho )
//      std::cout << type.second << ": " << type.first << std::endl;
//
//    std::cout << "- Functional: " << std::endl;
//    for( auto type : _statsFunction )
//      std::cout << _namesTypesFunction[ type.first ]
//                << " -> " << type.first
//                << " # " << type.second
//                << std::endl;
//
//    std::cout << " Grouped in: " << std::endl;
//    for( auto type : _namesIdxFunction )
//      std::cout << type.second << ": " << type.first << std::endl;

    return result;
  }
#endif // SIMIL_USE_BRION

  const std::vector< long unsigned int >& DomainManager::attributeValues( int attribNumber ) const
   {
     if(( tNeuronAttributes )attribNumber == T_TYPE_MORPHO )
       return _typesMorpho;
     else
       return _typesFunction;
   }

   Strings DomainManager::attributeNames( int attribNumber, bool  ) const
   {
     Strings result;

     if( _groupByName )
     {
       result = ( attribNumber == 0 ) ? _namesTypesMorphoGrouped :
                                       _namesTypesFunctionGrouped;
     }
     else
     {
       const auto& namesIdx = ( attribNumber == 0 ) ? _namesIdxMorpho : _namesIdxFunction;

       result.resize( namesIdx.size( ));

       for( auto name : namesIdx )
       {
         result[ name.second ] = name.first;
       }

     }
     result.shrink_to_fit( );

     return result;
   }

   tAppStats DomainManager::attributeStatistics( void ) const
   {
     tAppStats result;

     const auto& stats =
         ( _currentAttrib == T_TYPE_MORPHO ) ? _statsMorpho : _statsFunction;

     const auto& typeIdx =
         ( _currentAttrib == T_TYPE_MORPHO ) ? &_typeToIdxMorpho : &_typeToIdxFunction;

     const auto& nameIndices =
         ( _currentAttrib == T_TYPE_MORPHO ) ? _namesIdxMorpho : _namesIdxFunction;

     unsigned int numberOfValues = nameIndices.size( );

     result.resize( numberOfValues );

     std::vector< unsigned int > statsTotal( numberOfValues, 0 );

     for( auto valueStats : stats )
     {
       unsigned int index = typeIdx->find( valueStats.first )->second;

       statsTotal[ index ] += valueStats.second;
     }

     for( auto attrib : nameIndices )
     {
       auto& name = attrib.first;

       auto idxIt = nameIndices.find( name );
       unsigned int idx = idxIt->second;

       unsigned int value = statsTotal[ idx ];

       std::string label = "";
       auto labelIt = _attributeNameLabels.find( name );
       if( labelIt != _attributeNameLabels.end( ))
         label = labelIt->second;

       tStatsGroup attribs = std::make_tuple( 0, name, label, value );

       result[ idx ] = attribs;
     }

     return result;
   }

   tParticleInfo DomainManager::pickingInfoSimple( unsigned int particleId ) const
   {
     tParticleInfo result;

     unsigned int gid = 0;
     bool valid = false;
     vec3 position( 0, 0, 0 );
     QPoint screenPos( 0, 0 );

     auto gidIt = _particleToGID.find( particleId );
     if( gidIt != _particleToGID.end( ))
     {
       gid = gidIt->second;

       auto particle = _particleSystem->particles( ).at( particleId );

       position = particle.position( );

       valid = true;
     }

     std::get< T_PART_GID >( result ) = gid;
     std::get< T_PART_INTERNAL_GID >( result ) = particleId;
     std::get< T_PART_POSITION >( result ) = position;
     std::get< T_PART_SCREEN_POS >( result ) = screenPos;
     std::get< T_PART_VALID >( result ) = valid;

     return result;
   }

   void DomainManager::highlightElements( const std::unordered_set< unsigned int >& highlighted )
   {
     clearHighlighting( );

     prefr::ParticleIndices indices;
     indices.reserve( highlighted.size( ));

     for( auto gid : highlighted )
       indices.push_back( gid );

     _clusterHighlighted->particles( ).indices( indices );

     _clusterHighlighted->setModel( _modelHighlighted );

   }

   void DomainManager::clearHighlighting( void )
   {
     if( _mode == TMODE_SELECTION )
     {
       _clusterSelected->setModel( _modelBase );
       _clusterUnselected->setModel( _modelOff );
     }
     else
     {
       auto groupArray = ( _mode == TMODE_GROUPS ) ? _groups : _attributeGroups;

       for( auto group : groupArray )
       {
         group->cluster( )->setModel( group->model( ));
       }
     }
   }
}
