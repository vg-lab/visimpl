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

#include "VisualGroup.h"
#include "prefr/ColorOperationModel.h"

namespace visimpl
{

  typedef std::pair< uint32_t, prefr::Cluster* > TGIDCluster;

  class InputMultiplexer
  {
  public:

    InputMultiplexer( prefr::ParticleSystem* particleSystem,
                      const TGIDSet& gids );
    ~InputMultiplexer( );

    VisualGroup* addVisualGroup( const GIDUSet& group_,
                                 const std::string& name,
                                 bool overrideGIDs = false );

    void setVisualGroupState( unsigned int i, bool state );

    void processInput( const simil::SpikesCRange& spikes_,
                       float begin, float end, bool clear );

    void update( void );

    void showGroups( bool show );
    bool showGroups( void );

    void selection( const GIDUSet& newSelection );
    const GIDUSet& selection( void );

    void models( prefr::ColorOperationModel* main,
                 prefr::ColorOperationModel* off );

    void decay( float decayValue );

    std::vector< prefr::Cluster* > activeClusters( void );

    void clearSelection( void );
    void resetParticles( void );

    const std::vector< VisualGroup* >& groups( void ) const;

  protected:

    typedef std::vector< std::tuple< uint32_t, float >> TModifiedNeurons;

    TModifiedNeurons parseInput( const simil::SpikesCRange& spikes_,
                                 float begin, float end );

    void updateGroups( void );
    void updateSelection( void );

    void processFrameInputGroups( const simil::SpikesCRange& spikes_,
                                  float begin, float end );
    void processFrameInputSelection( const simil::SpikesCRange& spikes_,
                                     float begin, float end );

    prefr::ParticleSystem* _particleSystem;

    TGIDSet _gids;

    std::vector< VisualGroup* > _groups;

    std::unordered_map< uint32_t, VisualGroup* > _neuronGroup;

    std::unordered_map< uint32_t, prefr::Cluster* > _neuronClusters;
    std::unordered_map< prefr::Cluster*, uint32_t > _clusterNeurons;

    prefr::ColorOperationModel* _base;
    prefr::ColorOperationModel* _off;

    bool _showGroups;

    GIDUSet _selection;

    float _decayValue;


  };


}

#endif /* __VISIMPL_VISUALGROUPMANAGER__ */
