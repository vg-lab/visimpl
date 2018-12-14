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
    TMODE_ATTRIBUTE
  };

  class DomainManager
  {
  public:

    DomainManager( prefr::ParticleSystem* particleSystem,
                      const TGIDSet& gids );
    ~DomainManager( );

    void init( const tGidPosMap& positions );

    void initializeParticleSystem( void );

    VisualGroup* addVisualGroup( const GIDUSet& group_,
                                 const std::string& name,
                                 bool overrideGIDs = false );

    void setVisualGroupState( unsigned int i, bool state );

    void removeVisualGroup( unsigned int i );

    void processInput( const simil::SpikesCRange& spikes_,
                       float begin, float end, bool clear );

    void update( void );

    void mode( tVisualMode newMode );
    tVisualMode mode( void );

//    void showGroups( bool show );
    bool showGroups( void );

    void selection( const GIDUSet& newSelection );
    const GIDUSet& selection( void );

    void decay( float decayValue );
    float decay( void ) const;

//    std::vector< prefr::Cluster* > activeClusters( void );

    void clearSelection( void );
    void resetParticles( void );

    const std::vector< VisualGroup* >& groups( void ) const;

    tBoundingBox boundingBox( void ) const;

    prefr::ColorOperationModel* modelSelectionBase( void );

  protected:

    typedef std::vector< std::tuple< uint32_t, float >> TModifiedNeurons;

    TModifiedNeurons _parseInput( const simil::SpikesCRange& spikes_,
                                 float begin, float end );

    void _updateGroupsModels( void );
    void _updateSelectionModels( void );

    void _updateSelectionIndices( void );

    void _processFrameInputGroups( const simil::SpikesCRange& spikes_,
                                  float begin, float end );
    void _processFrameInputSelection( const simil::SpikesCRange& spikes_,
                                     float begin, float end );

    prefr::ParticleSystem* _particleSystem;

    tGidPosMap _gidPositions;

    TGIDSet _gids;

    prefr::Cluster* _clusterSelected;
    prefr::Cluster* _clusterUnselected;

    SourceMultiPosition* _sourceSelected;
    SourceMultiPosition* _sourceUnselected;

    std::vector< VisualGroup* > _groups;

    std::unordered_map< uint32_t, VisualGroup* > _neuronGroup;

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
  };


}

#endif /* __VISIMPL_VISUALGROUPMANAGER__ */
