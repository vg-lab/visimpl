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

  DomainManager::DomainManager( prefr::ParticleSystem* particleSystem,
                                      const TGIDSet& gids)
  : _particleSystem( particleSystem )
  , _gids( gids )
  , _clusterSelected( nullptr )
  , _clusterUnselected( nullptr )
  , _sourceSelected( nullptr )
  , _sourceUnselected( nullptr )
  , _modelBase( nullptr )
  , _modelOff( nullptr )
  , _sampler( nullptr )
  , _updater( nullptr )
  , _mode( TMODE_SELECTION )
  , _decayValue( 0.0f )
  , _showInactive( true )
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

  void DomainManager::init( const tGidPosMap& positions )
  {
    _gidPositions = positions;

    _sourceSelected = new SourceMultiPosition( );
    _sourceUnselected = new SourceMultiPosition( );

    _sourceSelected->setPositions( _gidPositions );
    _sourceUnselected->setPositions( _gidPositions );

    _particleSystem->addSource( _sourceSelected );
    _particleSystem->addSource( _sourceUnselected );

    _clusterSelected = new prefr::Cluster( );
    _clusterUnselected = new prefr::Cluster( );

    _particleSystem->addCluster( _clusterSelected );
    _particleSystem->addCluster( _clusterUnselected );
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
    _particleSystem->parallel( false );

    _particleSystem->start();
  }

  void DomainManager::mode( tVisualMode newMode )
  {
    _mode = newMode;

    switch( _mode )
    {
      case TMODE_SELECTION:

        _updateSelectionIndices( );
        _updateSelectionModels( );

        break;
      case TMODE_GROUPS:

        break;

      case TMODE_ATTRIBUTE:
        //TODO
        break;

      default:
        break;
    }
  }

  tVisualMode DomainManager::mode( void )
  {
    return _mode;
  }


  VisualGroup* DomainManager::addVisualGroup( const GIDUSet& gids,
                                                 const std::string& name,
                                                 bool overrideGIDS )
  {
    VisualGroup* group = new VisualGroup( name );

    prefr::ColorOperationModel* model =
        new prefr::ColorOperationModel( *_modelBase );

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

  void DomainManager::removeVisualGroup( unsigned int  )
  {

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
        _updateSelectionModels( );
        break;
      case TMODE_GROUPS:
        _updateGroupsModels( );
        break;
      case TMODE_ATTRIBUTE:
        //TODO
        break;
    }

  }

  //TODO
  bool DomainManager::showGroups( void )
  {
//    return _showGroups;
    return _mode == TMODE_GROUPS;
  }

  void DomainManager::setVisualGroupState( unsigned int i, bool state )
  {
    assert( i < _groups.size( ));

    VisualGroup* group = _groups[ i ];


    group->active( state );
    group->cluster( )->setModel( state ? group->model( ) : _modelOff );

    //TODO
    if( !_showInactive )
      group->source( )->active( state );

//    if( _showGroups )
//    {
//      for( auto gid : group->gids( ))
//      {
//        auto cluster = _neuronClusters.find( gid );
//
//        cluster->second->setModel( state ? group->model( ) : _off);
//      }
//    }
//    else
//    {
//      for( auto gid : group->gids( ))
//      {
//        auto cluster = _neuronClusters.find( gid );
//
//        auto res = _selection.find( gid );
//
//        cluster->second->setModel( res != _selection.end( ) ? _base : _off );
//
//      }
//    }

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
      group->model( group->active( ) ? _modelBase : _modelOff );
    }
  }

  void DomainManager::_updateSelectionIndices( void )
  {
    unsigned int numParticles = _gids.size( );

    prefr::ParticleIndices indicesSelected;
    prefr::ParticleIndices indicesUnselected;
    indicesSelected.reserve( numParticles );
    indicesUnselected.reserve( numParticles );

    _particleSystem->releaseParticles( _clusterSelected->particles( ).indices( ));
    _particleSystem->releaseParticles( _clusterUnselected->particles( ).indices( ));

    auto availableParticles =  _particleSystem->retrieveUnused( numParticles );

    _boundingBox.first = glm::vec3( std::numeric_limits< float >::max( ),
                                    std::numeric_limits< float >::max( ),
                                    std::numeric_limits< float >::max( ));

    _boundingBox.second = glm::vec3( std::numeric_limits< float >::min( ),
                                     std::numeric_limits< float >::min( ),
                                     std::numeric_limits< float >::min( ));

//    std::cout << "Particle ids: ";
    auto gidit = _gids.begin( );
    for( auto particle : availableParticles )
    {
      unsigned int id = particle.id( );

//      std::cout << " " << id;
      // Create reference
      _gidToParticle.insert( std::make_pair( *gidit, id ));
      _particleToGID.insert( std::make_pair( id, *gidit ));

      // Check if part of selection
      if( _selection.empty( ) || _selection.find( *gidit ) != _selection.end( ))
      {
        indicesSelected.emplace_back( id );
        _gidSource.insert( std::make_pair( *gidit, _sourceSelected ));
      }
      else
      {
        indicesUnselected.emplace_back( id );
        _gidSource.insert( std::make_pair( *gidit, _sourceUnselected ));
      }

      auto pos = _gidPositions.find( *gidit )->second;
      expandBoundingBox( _boundingBox.first, _boundingBox.second, pos );

      ++gidit;
    }

//    std::cout << std::endl;

    indicesSelected.shrink_to_fit( );
    indicesUnselected.shrink_to_fit( );


    _clusterSelected->particles( ).indices( indicesSelected );
    _clusterUnselected->particles( ).indices( indicesUnselected );

//    _sourceSelected->particles( indicesSelected );
//    _sourceUnselected->particles( indicesUnselected );

    _clusterSelected->setSource( _sourceSelected );
    _clusterUnselected->setSource( _sourceUnselected );

    _sourceSelected->setIdxTranslation( _particleToGID );
    _sourceUnselected->setIdxTranslation( _particleToGID );

    _sourceSelected->restart( );
    _sourceUnselected->restart( );


    _clusterSelected->setUpdater( _updater );
    _clusterUnselected->setUpdater( _updater );

    _clusterUnselected->setModel( _modelOff );
    _clusterSelected->setModel( _modelBase );

    std::cout << "Created indices for selection mode" << std::endl;

  }

  void DomainManager::_updateSelectionModels( void )
  {


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
        //TODO
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

  void DomainManager::_processFrameInputGroups( const simil::SpikesCRange& spikes_,
                                                  float begin, float end )
  {

    auto state = _parseInput( spikes_, begin, end );

    for( const auto& neuron : state )
    {
      auto gid = std::get< 0 >( neuron );

      auto visualGroup = _neuronGroup.find( gid );

      assert( visualGroup != _neuronGroup.end( ));

      if( visualGroup->second->active( ))
      {
        unsigned int particleIndex = _gidToParticle.find( gid )->second;

        auto particle = _particleSystem->particles( ).at( particleIndex );
//        auto particle = visualGroup->second->source( )->particles( ).at( particleIndex );
        particle.set_life( std::get< 1 >( neuron ) );
      }
//      auto cluster = _neuronClusters.find( gid );
//
//      assert( cluster != _neuronClusters.end( ));
//
//      auto particleRange = cluster->second->particles( );
//
//      if( visualGroup != _neuronGroup.end( ) && visualGroup->second->active( ))
//      {
//        dynamic_cast< prefr::ValuedSource* >( cluster->second->source( ))->particlesLife( std::get< 1 >( neuron ));
//      }


    }

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
//        auto particle = source->second->particles( ).at( partIdx );
        particle.set_life( std::get< 1 >( neuron ));
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

  void DomainManager::selection( const GIDUSet& newSelection )
  {
    _selection = newSelection;

    if( _mode == TMODE_SELECTION )
    {
      _updateSelectionIndices( );
      _updateSelectionModels( );
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
      _updateSelectionModels( );
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



}
