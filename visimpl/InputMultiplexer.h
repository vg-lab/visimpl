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

  class InputMultiplexer
  {
  public:

    InputMultiplexer( prefr::ParticleSystem* particleSystem,
                      const std::unordered_map< uint32_t, prefr::Cluster* >& reference );
    ~InputMultiplexer( );

    void addVisualGroup( const GIDUSet& group_, bool overrideGIDs = false );

    void setVisualGroupState( unsigned int i, bool state );


    void processFrameInput( const simil::SpikesCRange& spikes_ );

    void showGroups( bool show );

    void selection( const GIDUSet& newSelection );
    const GIDUSet& selection( void );

    void clearSelection( void );
    void resetParticles( void );

  protected:

    void updateGroups( void );
    void updateSelection( void );

    void processFrameInputGroups( const simil::SpikesCRange& spikes_ );
    void processFrameInputSelection( const simil::SpikesCRange& spikes_ );

    prefr::ParticleSystem* _particleSystem;

    std::vector< VisualGroup* > _groups;

    std::unordered_map< uint32_t, VisualGroup* > _neuronGroup;

    std::unordered_map< uint32_t, prefr::Cluster* > _neuronClusters;
    std::unordered_map< prefr::Cluster*, uint32_t > _clusterNeurons;

    prefr::ColorOperationModel* _base;
    prefr::ColorOperationModel* _off;

    bool _showGroups;

    GIDUSet _selection;

  };


}

#endif /* __VISIMPL_VISUALGROUPMANAGER__ */
