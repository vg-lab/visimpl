//
// Created by gaeqs on 21/06/22.
//

#include "DomainManager.h"
#include "visimpl/particlelab/ParticleLabShaders.h"

#include <utility>


namespace visimpl
{

  DomainManager::DomainManager( )
    : _mode( VisualMode::Selection )
    , _camera( nullptr )
    , _selectionGids( )
    , _selectionCluster( nullptr )
    , _groupClusters( )
    , _attributeClusters( )
    , _attributeNames( )
    , _attributeTypeGids( )
    , _selectionModel( nullptr )
    , _currentRenderer( nullptr )
    , _defaultRenderer( nullptr )
    , _solidRenderer( nullptr )
    , _solidMode( false )
    , _accumulativeMode( false )
    , _decay( 1.5f )
  {
  }

  void DomainManager::initRenderers(
    const std::shared_ptr <reto::ClippingPlane>& leftPlane ,
    const std::shared_ptr <reto::ClippingPlane>& rightPlane ,
    const std::shared_ptr <plab::ICamera>& camera )
  {
    _camera = camera;

    _selectionCluster =
      std::make_shared < plab::Cluster < NeuronParticle >> ( );

    TColorVec colors;
    colors.emplace_back( 0.0f , glm::vec4( 0.0f , 1.0f , 0.0f , 0.2f ));
    colors.emplace_back( 0.35f , glm::vec4( 1.0f , 0.0f , 0.0f , 0.5f ));
    colors.emplace_back( 0.7f , glm::vec4( 1.0f , 1.0f , 0.0f , 0.5f ));
    colors.emplace_back( 1.0f , glm::vec4( 0.0f , 0.0f , 1.0f , 0.5f ));

    TSizeFunction sizes;
    sizes.emplace_back( 0.0f , 6.0f );
    sizes.emplace_back( 1.0f , 18.0f );

    _selectionModel = std::make_shared< StaticGradientModel >(
      camera , leftPlane , rightPlane , sizes , colors ,
      true , false , 0.0f , _decay );

    _defaultProgram.loadFromText(
      visimpl::PARTICLE_VERTEX_SHADER ,
      visimpl::PARTICLE_DEFAULT_FRAGMENT_SHADER );
    _defaultProgram.compileAndLink( );

    _solidProgram.loadFromText(
      visimpl::PARTICLE_VERTEX_SHADER ,
      visimpl::PARTICLE_SOLID_FRAGMENT_SHADER );
    _solidProgram.compileAndLink( );

    _defaultAccProgram.loadFromText(
      visimpl::PARTICLE_VERTEX_SHADER ,
      visimpl::PARTICLE_ACC_DEFAULT_FRAGMENT_SHADER );
    _defaultAccProgram.compileAndLink( );

    _solidAccProgram.loadFromText(
      visimpl::PARTICLE_VERTEX_SHADER ,
      visimpl::PARTICLE_ACC_SOLID_FRAGMENT_SHADER );
    _solidAccProgram.compileAndLink( );

    _defaultRenderer = std::make_shared< plab::SimpleRenderer >(
      _defaultProgram.program( ));
    _solidRenderer = std::make_shared< plab::SimpleRenderer >(
      _solidProgram.program( ));

    _defaultAccRenderer = std::make_shared< plab::SimpleRenderer >(
      _defaultAccProgram.program( ));
    _solidAccRenderer = std::make_shared< plab::SimpleRenderer >(
      _solidAccProgram.program( ));

    _selectionCluster->setModel( _selectionModel );

    refreshRenderer( );
  }

  void DomainManager::refreshRenderer( )
  {
    if ( _accumulativeMode )
    {
      _currentRenderer = _solidMode ? _solidAccRenderer : _defaultAccRenderer;
    }
    else
    {
      _currentRenderer = _solidMode ? _solidRenderer : _defaultRenderer;
    }

    if ( _selectionCluster != nullptr )
      _selectionCluster->setRenderer( _currentRenderer );
    for ( const auto& item: _groupClusters )
      item.second->setRenderer( _currentRenderer );
    for ( const auto& item: _attributeClusters )
      item.second->setRenderer( _currentRenderer );
  }

#ifdef SIMIL_USE_BRION

  void DomainManager::initAttributeData(
    const TGIDSet& gids ,
    const brion::BlueConfig* blueConfig )
  {
    if ( blueConfig == nullptr ) return;
    brion::Circuit circuit( blueConfig->getCircuitSource( ));
    uint32_t attributes = brion::NEURON_COLUMN_GID |
                          brion::NEURON_MTYPE |
                          brion::NEURON_ETYPE;
    const auto& attributeData = circuit.get( gids , attributes );

    auto rawMorphoNames = circuit.getTypes(
      brion::NEURONCLASS_MORPHOLOGY_CLASS );
    auto rawFuncNames = circuit.getTypes(
      brion::NEURONCLASS_FUNCTION_CLASS );

    _attributeNames[ T_TYPE_MORPHO ] = rawMorphoNames;
    _attributeNames[ T_TYPE_FUNCTION ] = rawFuncNames;
    auto& morphoNames = _attributeNames[ T_TYPE_MORPHO ];
    auto& funcNames = _attributeNames[ T_TYPE_FUNCTION ];
    std::sort( morphoNames.begin( ) , morphoNames.end( ));
    std::sort( funcNames.begin( ) , funcNames.end( ));

    morphoNames.erase( std::unique( morphoNames.begin( ) , morphoNames.end( )) ,
                       morphoNames.end( ));
    funcNames.erase( std::unique( funcNames.begin( ) , funcNames.end( )) ,
                     funcNames.end( ));

    _attributeTypeGids[ T_TYPE_MORPHO ] =
      std::vector< std::vector< uint32_t > >( morphoNames.size( ));
    _attributeTypeGids[ T_TYPE_FUNCTION ] =
      std::vector< std::vector< uint32_t > >( funcNames.size( ));
    auto& morphoSets = _attributeTypeGids[ T_TYPE_MORPHO ];
    auto& funcSets = _attributeTypeGids[ T_TYPE_FUNCTION ];


    uint32_t i = 0;
    for ( const auto& gid: gids )
    {
      auto morphoType = boost::lexical_cast< unsigned int >(
        attributeData[ i ][ 1 ] );
      auto functionType = boost::lexical_cast< unsigned int >(
        attributeData[ i ][ 2 ] );

      auto& morphoName = rawMorphoNames[ morphoType ];
      auto& funcName = rawFuncNames[ functionType ];

      int morphoIndex = std::distance(
        morphoNames.begin( ) ,
        std::find( morphoNames.begin( ) , morphoNames.end( ) , morphoName ));

      int funcIndex = std::distance(
        funcNames.begin( ) ,
        std::find( funcNames.begin( ) , funcNames.end( ) , funcName ));


      morphoSets[ morphoIndex ].push_back( gid );
      funcSets[ funcIndex ].push_back( gid );

      i++;
    }
  }

#endif

  std::shared_ptr <plab::Cluster< NeuronParticle >>
  DomainManager::getSelectionCluster( ) const
  {
    return _selectionCluster;
  }

  const std::shared_ptr <StaticGradientModel>&
  DomainManager::getSelectionModel( ) const
  {
    return _selectionModel;
  }

  int DomainManager::getGroupAmount( ) const
  {
    return _groupClusters.size( );
  }

  const std::map <std::string , std::shared_ptr< VisualGroup>>&
  DomainManager::getGroups( ) const
  {
    return _groupClusters;
  }

  std::shared_ptr <VisualGroup>
  DomainManager::getGroup( const std::string& name ) const
  {
    return _groupClusters.at( name );
  }

  const std::map <std::string , std::shared_ptr< VisualGroup>>&
  DomainManager::getAttributeClusters( ) const
  {
    return _attributeClusters;
  }

  VisualMode DomainManager::getMode( ) const
  {
    return _mode;
  }

  void DomainManager::setMode( VisualMode mode )
  {
    _mode = mode;
  }

  float DomainManager::getDecay( ) const
  {
    return _decay;
  }

  void DomainManager::setDecay( float decay )
  {
    _decay = decay;
    _selectionModel->setDecay( decay );
    for ( const auto& item: _groupClusters )
    {
      item.second->getModel( )->setDecay( decay );
    }
    for ( const auto& item: _attributeClusters )
    {
      item.second->getModel( )->setDecay( decay );
    }
  }

  void DomainManager::setTime( float time )
  {
    if ( _selectionModel != nullptr )
    {
      _selectionModel->setTime( time );
    }
    for ( const auto& item: _groupClusters )
      item.second->getModel( )->setTime( time );
  }

  void DomainManager::addTime( float time , float endTime )
  {
    if ( _selectionModel != nullptr )
    {
      _selectionModel->addTime( time , endTime );
    }
    for ( const auto& item: _groupClusters )
      item.second->getModel( )->addTime( time , endTime );
  }

  const tBoundingBox& DomainManager::getBoundingBox( ) const
  {
    return _boundingBox;
  }

  void DomainManager::setSelection( const TGIDSet& gids ,
                                    const tGidPosMap& positions )
  {
    _selectionGids.clear( );
    std::copy(
      gids.cbegin( ) , gids.cend( ) ,
      std::back_inserter( _selectionGids )
    );

    float minLimit = std::numeric_limits< float >::min( );
    float maxLimit = std::numeric_limits< float >::max( );
    glm::vec3 min( maxLimit , maxLimit , maxLimit );
    glm::vec3 max( minLimit , minLimit , minLimit );

    std::vector <NeuronParticle> particles;
    for ( const auto& gid: _selectionGids )
    {
      NeuronParticle p;
      p.position = positions.at( gid );
      particles.push_back( p );

      min = glm::min( min , p.position );
      max = glm::max( max , p.position );
    }

    _boundingBox = std::make_pair( min , max );

    _selectionCluster->setParticles( particles );
  }

  void DomainManager::setSelection( const GIDUSet& gids ,
                                    const tGidPosMap& positions )
  {
    _selectionGids.clear( );
    std::copy(
      gids.cbegin( ) , gids.cend( ) ,
      std::back_inserter( _selectionGids )
    );

    float minLimit = std::numeric_limits< float >::min( );
    float maxLimit = std::numeric_limits< float >::max( );
    glm::vec3 min( maxLimit , maxLimit , maxLimit );
    glm::vec3 max( minLimit , minLimit , minLimit );

    std::vector <NeuronParticle> particles;
    for ( const auto& gid: _selectionGids )
    {
      NeuronParticle p;
      p.position = positions.at( gid );
      particles.push_back( p );

      min = glm::min( min , p.position );
      max = glm::max( max , p.position );
    }

    _boundingBox = std::make_pair( min , max );

    _selectionCluster->setParticles( particles );
  }

  std::shared_ptr <VisualGroup> DomainManager::createGroup(
    const GIDUSet& gids , const tGidPosMap& positions ,
    const std::string& name )
  {
    auto group = std::make_shared< VisualGroup >(
      name , _camera ,
      _selectionModel->getLeftPlane( ) ,
      _selectionModel->getRightPlane( ) ,
      _currentRenderer ,
      _selectionModel->isClippingEnabled( ));

    const TSizeFunction defaultSizes{{ 0.0f , 50.0f } ,
                                     { 1.0f , 15.0f }};

    group->getModel( )->setParticleSize( defaultSizes );
    group->getModel( )->setAccumulativeMode( _accumulativeMode );

    std::vector <uint32_t> ids;
    std::vector <NeuronParticle> particles;

    std::copy(
      gids.cbegin( ) , gids.cend( ) ,
      std::back_inserter( ids )
    );

    for ( const auto& gid: ids )
    {
      NeuronParticle p;
      p.position = positions.at( gid );
      particles.push_back( p );
    }

    group->setParticles( ids , particles );
    _groupClusters[ name ] = group;

    return group;
  }

  std::shared_ptr <VisualGroup>
  DomainManager::createGroupFromSelection(
    const tGidPosMap& positions , const std::string& name )
  {
    auto group = std::make_shared< VisualGroup >(
      name , _camera ,
      _selectionModel->getLeftPlane( ) ,
      _selectionModel->getRightPlane( ) ,
      _currentRenderer ,
      _selectionModel->isClippingEnabled( ));

    group->getModel( )->setAccumulativeMode( _accumulativeMode );

    std::vector <NeuronParticle> particles;
    for ( const auto& gid: _selectionGids )
    {
      NeuronParticle p;
      p.position = positions.at( gid );
      particles.push_back( p );
    }

    group->setParticles( _selectionGids , particles );
    _groupClusters[ name ] = group;

    return group;
  }

  void DomainManager::removeGroup( const std::string& name )
  {
    if ( 1 != _groupClusters.erase( name ))
    {
      std::cerr << "DomainManager: Error removing group '" << name
                << "' - " << __FILE__ << ":" << __LINE__ << std::endl;
    }
  }

  void DomainManager::selectAttribute(
    const std::vector <QColor>& colors ,
    const tGidPosMap& positions ,
    tNeuronAttributes attribute )
  {
    _attributeClusters.clear( );

    auto& names = _attributeNames[ attribute ];
    auto& typeGids = _attributeTypeGids[ attribute ];

    uint32_t i = 0;
    for ( const auto& name: names )
    {
      auto& gids = typeGids[ i ];

      auto group = std::make_shared< VisualGroup >(
        name , _camera ,
        _selectionModel->getLeftPlane( ) ,
        _selectionModel->getRightPlane( ) ,
        _currentRenderer ,
        _selectionModel->isClippingEnabled( ));
      group->getModel( )->setAccumulativeMode( _accumulativeMode );

      const auto currentIndex = i % colors.size( );
      const auto color = colors[ currentIndex ].toRgb( );
      const auto variations = generateColorPair( color );

      TTransferFunction colorVariation;
      colorVariation.push_back( std::make_pair( 0.0f , variations.first ));
      colorVariation.push_back( std::make_pair( 1.0f , variations.second ));
      group->colorMapping( colorVariation );

      std::vector <NeuronParticle> particles;
      for ( const auto& gid: gids )
      {
        NeuronParticle p;
        p.position = positions.at( gid );
        particles.push_back( p );
      }

      group->setParticles( gids , particles );

      _attributeClusters[ name ] = group;

      i++;
    }

  }

  bool DomainManager::isAccumulativeModeEnabled( )
  {
    return _accumulativeMode;
  }

  void DomainManager::enableAccumulativeMode( bool enabled )
  {
    _accumulativeMode = enabled;
    if ( _selectionModel != nullptr )
    {
      _selectionModel->setAccumulativeMode( enabled );
    }
    for ( const auto& item: _groupClusters )
      item.second->getModel( )->setAccumulativeMode( enabled );
    refreshRenderer( );
  }

  void DomainManager::enableClipping( bool enabled )
  {
    if ( _selectionModel != nullptr )
    {
      _selectionModel->enableClipping( enabled );
    }
    for ( const auto& item: _groupClusters )
      item.second->getModel( )->enableClipping( enabled );
  }

  void DomainManager::draw( ) const
  {

    switch ( _mode )
    {
      case VisualMode::Selection:
        _selectionCluster->render( );
        break;
      case VisualMode::Groups:
        for ( const auto& item: _groupClusters )
        {
          if ( item.second->active( ))
          {
            item.second->getCluster( )->render( );
          }
        }
        break;
      case VisualMode::Attribute:
        for ( const auto& item: _attributeClusters )
        {
          if ( item.second->active( ))
          {
            item.second->getCluster( )->render( );
          }
        }
        break;
      default:
        break;
    }

  }

  void DomainManager::applyDefaultShader( )
  {
    _solidMode = false;
    refreshRenderer( );
  }

  void DomainManager::applySolidShader( )
  {
    _solidMode = true;
    refreshRenderer( );
  }

  void DomainManager::processInput(
    const simil::SpikesCRange& spikes , bool killParticles )
  {
    switch ( _mode )
    {
      case VisualMode::Selection:
        processSelectionSpikes( spikes , killParticles );
        break;
      case VisualMode::Groups:
        processGroupSpikes( spikes , killParticles );
        break;
      case VisualMode::Attribute:
        processAttributeSpikes( spikes , killParticles );
        break;
      default:
        break;
    }
  }

  std::unordered_map< uint32_t , float >
  DomainManager::parseInput( const simil::SpikesCRange& spikes )
  {
    std::unordered_map< uint32_t , float > result;

    for ( simil::SpikesCIter spike = spikes.first;
          spike != spikes.second; ++spike )
    {
      if ( result.count( spike->second ) == 0 )
      {
        result[ spike->second ] = spike->first;
      }
    }
    return result;
  }

  void DomainManager::processSelectionSpikes(
    const simil::SpikesCRange& spikes , bool killParticles )
  {
    auto input = parseInput( spikes );

    auto map = _selectionCluster->mapData( );
    const uint32_t size = _selectionCluster->size( );
    for ( uint32_t i = 0; i < size; i++ )
    {
      auto particle = map + i;
      if ( killParticles )
      {
        particle->timestamp = -std::numeric_limits< float >::infinity( );
      }

      auto gid = _selectionGids.at( i );
      auto value = input.find( gid );
      if ( value != input.cend( ))
      {
        particle->timestamp = value->second;
      }
    }

    _selectionCluster->unmapData( );
  }

  void DomainManager::processGroupSpikes( const simil::SpikesCRange& spikes ,
                                          bool killParticles )
  {
    auto input = parseInput( spikes );
    for ( const auto& item: _groupClusters )
    {
      auto cluster = item.second->getCluster( );
      auto map = cluster->mapData( );
      uint32_t size = cluster->size( );
      for ( uint32_t i = 0; i < size; i++ )
      {
        auto particle = map + i;
        if ( killParticles )
        {
          particle->timestamp = -std::numeric_limits< float >::infinity( );
        }
        auto gid = item.second->getGids( ).at( i );
        auto value = input.find( gid );
        if ( value != input.cend( ))
        {
          particle->timestamp = value->second;
        }
      }

      cluster->unmapData( );
    }
  }

  void
  DomainManager::processAttributeSpikes( const simil::SpikesCRange& spikes ,
                                         bool killParticles )
  {
    auto input = parseInput( spikes );
    for ( const auto& item: _attributeClusters )
    {
      auto cluster = item.second->getCluster( );
      auto map = cluster->mapData( );
      const uint32_t size = cluster->size( );
      for ( uint32_t i = 0; i < size; i++ )
      {
        auto particle = map + i;
        if ( killParticles )
        {
          particle->timestamp = -std::numeric_limits< float >::infinity( );
        }
        auto gid = item.second->getGids( ).at( i );
        auto value = input.find( gid );
        if ( value != input.cend( ))
        {
          particle->timestamp = value->second;
        }
      }

      cluster->unmapData( );
    }
  }
}
