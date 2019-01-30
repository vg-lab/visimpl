/*
 * @file  DataSource.cpp
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */

#include "DomainManager.h"

#include "prefr/UpdaterStaticPosition.h"
#include "prefr/SourceMultiPosition.h"

namespace visimpl
{
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
  , _sourceSelected( nullptr )
//  , _sourceUnselected( nullptr )
  , _currentAttrib( T_TYPE_UNDEFINED )
  , _modelBase( nullptr )
  , _modelOff( nullptr )
  , _sampler( nullptr )
  , _updater( nullptr )
  , _mode( TMODE_SELECTION )
  , _decayValue( 0.0f )
  , _showInactive( true )
  , _groupByName( false )
  , _autoGroupByName( true )
//  , _showGroups( false )
  {
//TODO
//    auto cluster =
//        particleSystem->clusters( ).begin( );
//    for( auto gid : _gids )
//    {
//      _neuronClusters.insert( std::make_pair( gid, *cluster ));
//      _clusterNeurons.insert( std::make_pair( *cluster, gid ));
//
//      ++cluster;
//    }

  }

  DomainManager::~DomainManager( )
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
//    _sourceUnselected = new SourceMultiPosition( );

    _sourceSelected->setPositions( _gidPositions );
//    _sourceUnselected->setPositions( _gidPositions );

//    _particleSystem->addSource( _sourceSelected );
//    _particleSystem->addSource( _sourceUnselected );

    _clusterSelected = new prefr::Cluster( );
    _clusterUnselected = new prefr::Cluster( );

    _particleSystem->addCluster( _clusterSelected );
    _particleSystem->addCluster( _clusterUnselected );

    _loadPaletteColors( );
  }

  void DomainManager::initializeParticleSystem( void )
  {
    std::cout << "Initializing particle system..." << std::endl;

    _updater = new UpdaterStaticPosition( );

    prefr::Sorter* sorter = new prefr::Sorter( );
    prefr::GLRenderer* renderer = new prefr::GLRenderer( );

    _particleSystem->addUpdater( _updater );
    _particleSystem->sorter( sorter );
    _particleSystem->renderer( renderer );

    _modelOff = new prefr::ColorOperationModel( _decayValue, _decayValue );
    _modelOff->color.Insert( 0.0f, ( glm::vec4(0.1f, 0.1f, 0.1f, 0.2f)));

    _modelOff->velocity.Insert( 0.0f, 0.0f );

    _modelOff->size.Insert( 1.0f, 10.0f );

    _particleSystem->addModel( _modelOff );

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
    _particleSystem->detachSource( _sourceSelected );
//    _particleSystem->detachSource( _sourceUnselected );

    _clearParticlesReference( );
//    _particleSystem->releaseParticles( _clusterSelected->particles( ).indices( ));
//    _particleSystem->releaseParticles( _clusterUnselected->particles( ).indices( ));
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


    //TODO
//    _currentAttrib = T_TYPE_UNDEFINED;
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

//    float invSize = 1.0f / ( colors.size( ) - 1 );

    float brightFactor = 0.4f;
    float darkFactor = 1.0f - brightFactor;

    _paletteColors.clear( );

    for( auto color: colors )
    {
//      QColor color = colors[ _groups.size( ) ];

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



//  void DomainManager::showGroups( bool show )
//  {
//    _showGroups = show;
//
//    update( );
//  }

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

        break;
    }

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

  //TODO
  bool DomainManager::showGroups( void )
  {
//    return _showGroups;
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
      assert( i < _groups.size( ));
      group = _groups[ i ];
    }
    else
    {
      assert( i < _attributeGroups.size( ));
      group = _attributeGroups[ i ];
    }

    group->active( state );
    group->cluster( )->setModel( state ? group->model( ) : _modelOff );

    //TODO
    if( !_showInactive )
      group->source( )->active( state );

  }

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


  void DomainManager::_updateGroupsModels( void )
  {

//    for( int i = 0; i < ( int ) )
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

//    _sourceSelected->particles( ).transferIndicesTo( _sourceUnselected->particles( ), other );
//    _sourceUnselected->particles( ).transferIndicesTo( _sourceSelected->particles( ), selection );

    _clusterSelected->particles( ).indices( selection );
    _clusterUnselected->particles( ).indices( other );

//    _clusterSelected->setSource( _sourceSelected, false );
//    _clusterUnselected->setSource( _sourceUnselected, false );

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

//    std::cout << "Particle ids: ";
    auto gidit = _gids.begin( );
    for( auto particle : availableParticles )
    {
      unsigned int id = particle.id( );

//      std::cout << " " << id;
      // Create reference
      _gidToParticle.insert( std::make_pair( *gidit, id ));
      _particleToGID.insert( std::make_pair( id, *gidit ));

      _gidSource.insert( std::make_pair( *gidit, _sourceSelected ));

      // Check if part of selection
      if( _selection.empty( ) || _selection.find( *gidit ) != _selection.end( ))
      {
        indicesSelected.emplace_back( id );
//        _gidSource.insert( std::make_pair( *gidit, _sourceSelected ));

        auto pos = _gidPositions.find( *gidit )->second;
        expandBoundingBox( _boundingBox.first, _boundingBox.second, pos );
      }
      else
      {
        indicesUnselected.emplace_back( id );
//        _gidSource.insert( std::make_pair( *gidit, _sourceUnselected ));
      }

      indices.emplace_back( id );

      ++gidit;
    }

//    std::cout << std::endl;

    indicesSelected.shrink_to_fit( );
    indicesUnselected.shrink_to_fit( );


    _clusterSelected->particles( ).indices( indicesSelected );
    _clusterUnselected->particles( ).indices( indicesUnselected );

//    _sourceSelected->particles( indicesSelected );
//    _sourceUnselected->particles( indicesUnselected );

//    _clusterSelected->setSource( _sourceSelected );
//    _clusterUnselected->setSource( _sourceUnselected );
//
    _sourceSelected->setIdxTranslation( _particleToGID );
//    _sourceUnselected->setIdxTranslation( _particleToGID );
//
//    _sourceSelected->restart( );
//    _sourceUnselected->restart( );

    _particleSystem->addSource( _sourceSelected, indices );
//    _particleSystem->addSource( _sourceSelected, indicesSelected );
//    _particleSystem->addSource( _sourceUnselected, indicesUnselected );

    _clusterSelected->setUpdater( _updater );
    _clusterUnselected->setUpdater( _updater );

    _clusterUnselected->setModel( _modelOff );
    _clusterSelected->setModel( _modelBase );

    std::cout << "Created indices for selection mode" << std::endl;

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


  VisualGroup* DomainManager::addVisualGroup( const GIDUSet& gids,
                                              const std::string& name,
                                              bool overrideGIDS )
  {
    VisualGroup* group = _generateGroup( gids, name, _groups.size( ));
    group->custom( true );

    for( auto gid : gids )
    {
      auto reference = _neuronGroup.find( gid );

      if( overrideGIDS && reference != _neuronGroup.end( ))
      {
        auto& oldGroup = reference->second;

        auto oldGroupGIDs = oldGroup->gids( );
        oldGroupGIDs.erase( gid );
        reference->second->gids( oldGroupGIDs );

        oldGroup->cached( false );
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
//    std::cout << "Clearing group " << group->name( )
//              << " size " << group->gids( ).size( )
//              << std::endl;

    _particleSystem->detachSource( group->source( ));

    if( clearState )
    {
      for( auto gid : group->gids( ))
      {
        auto ref = _gidToParticle.find( gid );

        _particleToGID.erase( ref->second );
        _gidToParticle.erase( ref );

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


//      unsigned int counter = 0;
      auto partId = availableParticles.begin( );
      for( auto gid : gids )
      {
//        auto reference = _neuronGroup.find( gid );

        _neuronGroup.insert( std::make_pair( gid, group ));

        _gidToParticle.insert( std::make_pair( gid, partId.id( )));
        _particleToGID.insert( std::make_pair( partId.id( ), gid ));

        ++partId;
      }

      group->cached( true );
      group->dirty( false );

//      std::cout << "Generated particles for group with size " << group->gids( ).size( ) << std::endl;
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

//    const auto& names =
//        ( attrib == T_TYPE_MORPHO ) ? _namesTypesMorpho : _namesTypesFunction;

//    const auto& stats =
//        ( attrib == T_TYPE_MORPHO ) ? _statsMorpho : _statsFunction;

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

//      auto number = stats.find()

//      auto name = names[ typeIndex ];

//      unsigned int index = typeIndices.find( typeIndex.first )->second;

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

      //      unsigned int counter = 0;
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

//      std::cout << "Generated particles for attrib group with size " << group->gids( ).size( ) << std::endl;
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
//        *current = std::make_tuple( spike->second, lifeValue );
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
        auto source = _gidSource.find( gid );
        assert( source != _gidSource.end( ));

        if( source == _gidSource.end( ))
        {
          std::cout << "GID " << gid << " source not found." << std::endl;
        }

        unsigned int partIdx = _gidToParticle.find( gid )->second;
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

//      assert( visualGroup != _neuronGroup.end( ));

      if( visualGroup != _neuronGroup.end( ) && visualGroup->second->active( ))
      {
//        auto particles = _gidToParticle.equal_range( gid );
//
//        for( auto partIt = particles.first; partIt != particles.second; ++partIt )
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
//      auto groups = _neuronGroup.equal_range( gid );
//      assert( visualGroup != _neuronGroup.end( ));

      if( visualGroup != _neuronGroup.end( ) && visualGroup->second->active( ))
//      for( auto groupIt = groups.first; groupIt != groups.second; ++groupIt )
      {
//        auto particles = _gidToParticle.equal_range( gid );
//        for( auto partIt = particles.first; partIt != particles.second; ++partIt )
        auto partIt = _gidToParticle.find( gid );
        if( partIt != _gidToParticle.end( ))
        {
          unsigned int particleIndex = partIt->second;
//          auto group = groupIt->second;
//          if( group && group->source( )->particles( ).hasElement( particleIndex ))
//          {
          auto particle = _particleSystem->particles( ).at( particleIndex );
          particle.set_life( std::get< 1 >( neuron ) );
//          }
        }
      }
    }

  }


  void DomainManager::selection( const GIDUSet& newSelection )
  {
    _selection = newSelection;

    if( _mode == TMODE_SELECTION )
    {
//      _clearSelectionView( );
//      _generateSelectionIndices( );
      _updateSelectionIndices( );
    }
  }

  const GIDUSet& DomainManager::selection( void )
  {
    return _selection;
  }

  tBoundingBox DomainManager::boundingBox( void ) const
  {
    return _boundingBox;
  }

  prefr::ColorOperationModel* DomainManager::modelSelectionBase( void )
  {
    return _modelBase;
  }

//  std::vector< prefr::Cluster* > DomainManager::activeClusters( void )
//  {
//    std::vector< prefr::Cluster* > result;
//
//    if( _showGroups )
//    {
//      for( auto group : _groups )
//      {
//        if( !group->active( ))
//          continue;
//
//        for( auto gid : group->gids( ))
//        {
//          auto it = _neuronGroup.find( gid );
//          if( it != _neuronGroup.end( ) && group == it->second )
//          {
//            auto cluster = _neuronClusters.find( gid );
//            result.push_back( cluster->second );
//          }
//
//        }
//      }
//    }
//    else
//    {
//      result.reserve( _selection.size( ));
//      for( auto gid : _selection )
//      {
//        auto cluster = _neuronClusters.find( gid );
//        result.push_back( cluster->second );
//      }
//    }
//
//    return result;
//  }


  void DomainManager::decay( float decayValue )
  {
    _decayValue = decayValue;

    _modelBase->setLife( decayValue, decayValue );
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

//#ifdef VISIMPL_USE_OPENMP
//    #pragma omp parallel for
//    for( int i = 0; i < ( int )_particleSystem->clusters( ).size( ); i++ )
//    {
//      prefr::Cluster* cluster = _particleSystem->clusters( )[ i ];
//#else
//    for( auto cluster : _particleSystem->clusters( ))
//    {
//#endif
//
//      cluster->killParticles( false );
//
////      cluster->source( )->restart( );
//    }

    auto particles = _particleSystem->retrieveActive( );
    for( int i = 0; i < ( int ) particles.size( ); ++i )
    {
//    for( auto particle : particles )
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
        unsigned int morphoType =
            boost::lexical_cast< unsigned int >( attribsData[ i ][ 1 ]);

        unsigned int functionType =
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

      _typesMorpho = circuit.getMorphologyTypes( gids );
      _typesFunction = circuit.getElectrophysiologyTypes( gids );

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
//        auto& typeIdx = ( i == 0 ) ? _typeToIdxMorpho : _typeToIdxFunction;

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
//        std::unordered_map< std::string, unsigned int > uniqueNames;

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

//        unsigned int counter = 0;
        for( auto name : nameIndexer )
        {
          groupedNameStorage[ name.second ] = name.first;

//          ++counter;
        }

        if( groupedNameStorage.size( ) != names.size( ))
          _groupByName = true;

      }
    }


    std::cout << "Loaded attributes." << std::endl;
    std::cout << "- Morphological: " << std::endl;
    for( auto type : _statsMorpho )
      std::cout << _namesTypesMorpho[ type.first ]
                << " -> " << type.first
                << " # " << type.second
                << std::endl;

    std::cout << " Grouped in: " << std::endl;
    for( auto type : _namesIdxMorpho )
      std::cout << type.second << ": " << type.first << std::endl;

    std::cout << "- Functional: " << std::endl;
    for( auto type : _statsFunction )
      std::cout << _namesTypesFunction[ type.first ]
                << " -> " << type.first
                << " # " << type.second
                << std::endl;

    std::cout << " Grouped in: " << std::endl;
    for( auto type : _namesIdxFunction )
      std::cout << type.second << ": " << type.first << std::endl;

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

// //    auto statistics = _domainManager->attributeStatistics( );
//     auto statistics =
//         ( _currentAttrib == T_TYPE_MORPHO ) ? _statsMorpho : _statsFunction;
//
//     result.resize( statistics.size( ));
//
//     const auto& names = (( tNeuronAttributes ) attribNumber ) == T_TYPE_MORPHO ?
//                         &_namesTypesMorpho : &_namesTypesFunction;
//
//     const auto& indices = (( tNeuronAttributes ) attribNumber ) == T_TYPE_MORPHO ?
//                         &_typeToIdxMorpho : &_typeToIdxFunction;
//
// //    auto typeIds = ( _currentAttrib == T_TYPE_MORPHO ) ? &_typesMorpho : &_typesFunction;
//
//     for( auto attrib : statistics )
//     {
//       auto attribIdx = indices->find( attrib.first );
//
//       if( attribIdx == indices->end( ))
//       {
//         std::cout << "Attrib index not found " << attrib.first
//                   << " in map " << indices->size( )
//                   << std::endl;
//         continue;
//       }
// //      unsigned int attribId = ( *typeIds )[ attrib.first ];
//
//       auto name = ( *names )[ attrib.first ];
//
//       if( labels )
//       {
//         auto labelIt = _attributeNameLabels.find( name );
//         if( labelIt != _attributeNameLabels.end( ))
//           name = labelIt->second;
//       }
//
//       result[ attribIdx->second ] = ( name );
//     }


     if( _groupByName )
     {
       result = ( attribNumber == 0 ) ? _namesTypesMorphoGrouped :
                                       _namesTypesFunctionGrouped;
     }
     else
     {
//       const auto& names = ( attribNumber == 0 ) ? _namesTypesMorpho : _namesTypesFunction;
       const auto& namesIdx = ( attribNumber == 0 ) ? _namesIdxMorpho : _namesIdxFunction;

       result.resize( namesIdx.size( ));

       for( auto name : namesIdx )
       {
         result[ name.second ] = name.first;
       }

     }
     result.shrink_to_fit( );

 //    std::cout << "Attribute names: ";
 //    for( auto name : result )
 //      std::cout << " " << name;
 //    std::cout << std::endl;

     return result;
   }

   tAppStats DomainManager::attributeStatistics( void ) const
   {
     tAppStats result;

     const auto& stats =
         ( _currentAttrib == T_TYPE_MORPHO ) ? _statsMorpho : _statsFunction;

     const auto& typeIdx =
         ( _currentAttrib == T_TYPE_MORPHO ) ? &_typeToIdxMorpho : &_typeToIdxFunction;

//     const auto& names =
//         ( _currentAttrib == T_TYPE_MORPHO ) ? _namesTypesMorpho : _namesTypesFunction;

     const auto& nameIndices =
         ( _currentAttrib == T_TYPE_MORPHO ) ? _namesIdxMorpho : _namesIdxFunction;

//     const auto& backIndex =
//         ( _currentAttrib == T_TYPE_MORPHO ) ? _idxToTypeMorpho : _idxToTypeFunction;

     unsigned int numberOfValues = nameIndices.size( );

     result.resize( numberOfValues );

     std::vector< unsigned int > statsTotal( numberOfValues, 0 );

     for( auto valueStats : stats )
     {
       unsigned int index = typeIdx->find( valueStats.first )->second;

       statsTotal[ index ] += valueStats.second;
     }

 //    std::cout << "Stats" << std::endl;
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

 //      std::cout << " " << idx
 //                << " " << (( _currentAttrib == T_TYPE_MORPHO ) ?
 //                            _namesTypesMorpho : _namesTypesFunction)[ attrib.first ]
 //                << ": " << attrib.second << std::endl;
     }

     return result;



 //    return _domainManager->attributeStatistics( );
   }


}
