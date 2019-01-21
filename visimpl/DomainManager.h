/*
 * @file  DataSourceManager.h
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */
#ifndef __VISIMPL_VISUALGROUPMANAGER__
#define __VISIMPL_VISUALGROUPMANAGER__

#include <unordered_map>

#include <prefr/prefr.h>
#include <simil/simil.h>
#include <scoop/scoop.h>

#include <sumrice/sumrice.h>

#include "types.h"
#include "VisualGroup.h"
#include "prefr/ColorOperationModel.h"
#include "prefr/SourceMultiPosition.h"

namespace visimpl
{

  enum tVisualMode
  {
    TMODE_SELECTION = 0,
    TMODE_GROUPS,
    TMODE_ATTRIBUTE,
    TMODE_UNDEFINED
  };

  class DomainManager
  {
  public:

    DomainManager( prefr::ParticleSystem* particleSystem,
                      const TGIDSet& gids );
    ~DomainManager( );

    void init( const tGidPosMap& positions, const tNeuronAttribs& types );

    void initializeParticleSystem( void );

    VisualGroup* addVisualGroup( const GIDUSet& group_, const std::string& name,
                                 bool overrideGIDs = false );
    void setVisualGroupState( unsigned int i, bool state, bool attrib = false );
    void removeVisualGroup( unsigned int i );

    void showInactive( bool state );


    void generateAttributesGroups( tNeuronAttributes attrib );

    void processInput( const simil::SpikesCRange& spikes_,
                       float begin, float end, bool clear );

    void update( void );

    void mode( tVisualMode newMode );
    tVisualMode mode( void );

    void clearView( void );

//    void showGroups( bool show );
    bool showGroups( void );
    void updateGroups( void );
    void updateAttributes( void );

    void selection( const GIDUSet& newSelection );
    const GIDUSet& selection( void );

    void decay( float decayValue );
    float decay( void ) const;

//    std::vector< prefr::Cluster* > activeClusters( void );

    void clearSelection( void );
    void resetParticles( void );

    const std::vector< VisualGroup* >& groups( void ) const;
    const std::vector< VisualGroup* >& attributeGroups( void ) const;


    tBoundingBox boundingBox( void ) const;

    prefr::ColorOperationModel* modelSelectionBase( void );

    const tUintUMap& attributeStatistics( void ) const;

    const std::vector< std::pair< QColor, QColor >>& paletteColors( void ) const;

  protected:

    typedef std::vector< std::tuple< uint32_t, float >> TModifiedNeurons;

    TModifiedNeurons _parseInput( const simil::SpikesCRange& spikes_,
                                 float begin, float end );


    VisualGroup* _generateGroup( const GIDUSet& gids, const std::string& name,
                                 unsigned int idx ) const;

    void _updateGroupsModels( void );
    void _generateGroupsIndices( void );

    void _updateSelectionIndices( void );
    void _generateSelectionIndices( void );

    void _updateAttributesIndices( void );
    void _generateAttributesIndices( void );

    void _processFrameInputSelection( const simil::SpikesCRange& spikes_,
                                      float begin, float end );
    void _processFrameInputGroups( const simil::SpikesCRange& spikes_,
                                   float begin, float end );
    void _processFrameInputAttributes( const simil::SpikesCRange& spikes_,
                                       float begin, float end );

    void _loadPaletteColors( void );

    void _clearSelectionView( void );
    void _clearGroupsView( void );
    void _clearAttribView( void );

    void _clearGroups( void );
    void _clearAttribs( bool clearCustom = true );

    void _clearGroup( VisualGroup* group, bool clearState = true );
    void _clearParticlesReference( void );

    void _resetBoundingBox( void );

    SourceMultiPosition* getSource( unsigned int numParticles );

    prefr::ParticleSystem* _particleSystem;

    tGidPosMap _gidPositions;
    tNeuronAttribs _gidTypes;

    TGIDSet _gids;

    prefr::Cluster* _clusterSelected;
    prefr::Cluster* _clusterUnselected;

    SourceMultiPosition* _sourceSelected;
//    SourceMultiPosition* _sourceUnselected;

//    std::queue< Cluster* > _availableClusters;
//    std::queue< SourceMultiPosition* > _availableSources;

    std::vector< VisualGroup* > _groups;
    std::vector< VisualGroup* > _attributeGroups;
    tNeuronAttributes _currentAttrib;

    tUintUMap _attribStatistics;

    std::unordered_map< uint32_t, VisualGroup* > _neuronGroup;
//    std::unordered_multimap< uint32_t, VisualGroup* > _neuronGroup;

    std::unordered_map< unsigned int, SourceMultiPosition* > _gidSource;
//    std::unordered_map< uint32_t, prefr::Cluster* > _neuronClusters;
//    std::unordered_map< prefr::Cluster*, uint32_t > _clusterNeurons;

    std::unordered_map< unsigned int, unsigned int > _gidToParticle;
    std::unordered_map< unsigned int, unsigned int > _particleToGID;

    prefr::ColorOperationModel* _modelBase;
    prefr::ColorOperationModel* _modelOff;

    prefr::PointSampler* _sampler;
    prefr::Updater* _updater;

//    bool _showGroups;
    tVisualMode _mode;

    GIDUSet _selection;

    float _decayValue;

    bool _showInactive;

    tBoundingBox _boundingBox;

    std::vector< std::pair< QColor, QColor >> _paletteColors;


  };


}

#endif /* __VISIMPL_VISUALGROUPMANAGER__ */
