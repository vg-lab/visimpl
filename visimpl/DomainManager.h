//
// Created by gaeqs on 21/06/22.
//

#ifndef VISIMPL_DOMAINMANAGER_H
#define VISIMPL_DOMAINMANAGER_H

#include <map>

// ParticleLab
#include <plab/plab.h>

#include "visimpl/particlelab/NeuronParticle.h"
#include "VisualGroup.h"

#include "types.h"

namespace visimpl
{
  enum class VisualMode
  {
    Selection = 0 ,
    Groups ,
    Attribute ,
    Undefined
  };


  class DomainManager
  {

    VisualMode _mode;
    std::shared_ptr< plab::ICamera > _camera;

    std::vector< uint32_t > _selectionGids;
    std::shared_ptr< plab::Cluster< NeuronParticle > > _selectionCluster;

    std::map< std::string , std::shared_ptr< VisualGroup > > _groupClusters;

    std::map< std::string , std::shared_ptr< VisualGroup > > _attributeClusters;
    std::map< tNeuronAttributes , std::vector< std::string>> _attributeNames;
    std::map< tNeuronAttributes , std::vector< std::vector< uint32_t>> > _attributeTypeGids;

    // Models
    std::shared_ptr< StaticGradientModel > _selectionModel;

    // Renders
    reto::ShaderProgram _defaultProgram;
    reto::ShaderProgram _solidProgram;

    std::shared_ptr< plab::CoverageRenderer > _currentRenderer;
    std::shared_ptr< plab::CoverageRenderer > _defaultRenderer;
    std::shared_ptr< plab::CoverageRenderer > _solidRenderer;

    // Others
    tBoundingBox _boundingBox;
    float _decay;

  public:

    DomainManager( );

    void initRenderers(
      const std::shared_ptr< reto::ClippingPlane >& leftPlane ,
      const std::shared_ptr< reto::ClippingPlane >& rightPlane ,
      const std::shared_ptr< plab::ICamera >& camera );

#ifdef SIMIL_USE_BRION

    void initAttributeData( const TGIDSet& gids ,
                            const brion::BlueConfig* blueConfig );

#endif

    std::shared_ptr< plab::Cluster< NeuronParticle > >
    getSelectionCluster( ) const;

    const std::shared_ptr< StaticGradientModel >& getSelectionModel( ) const;

    int getGroupAmount( ) const;

    const std::map< std::string , std::shared_ptr< VisualGroup>>&
    getGroups( ) const;

    std::shared_ptr< VisualGroup > getGroup( const std::string& name ) const;

    const std::map< std::string , std::shared_ptr< VisualGroup>>&
    getAttributeClusters( ) const;

    VisualMode getMode( ) const;

    void setMode( VisualMode mode );

    float getDecay( ) const;

    void setDecay( float decay );

    void setTime( float time );

    void addTime( float time, float endTime );

    const tBoundingBox& getBoundingBox( ) const;

    void setSelection( const TGIDSet& gids ,
                       const tGidPosMap& positions );

    void setSelection( const GIDUSet& gids ,
                       const tGidPosMap& positions );

    std::shared_ptr< VisualGroup > createGroup( const GIDUSet& gids ,
                                                const tGidPosMap& positions ,
                                                const std::string& name );

    std::shared_ptr< VisualGroup > createGroupFromSelection(
      const tGidPosMap& positions , const std::string& name );

    void removeGroup( const std::string& name );

    void selectAttribute(
      const std::vector< QColor >& colors ,
      const tGidPosMap& positions ,
      tNeuronAttributes attribute );

    void applyDefaultShader( );

    void applySolidShader( );

    void enableAccumulativeMode( bool enabled );

    void enableClipping( bool enabled );

    void draw( ) const;

    void processInput(
      const simil::SpikesCRange& spikes , bool killParticles );

  protected:

    std::unordered_map< uint32_t , float >
    parseInput( const simil::SpikesCRange& spikes );

    void processSelectionSpikes(
      const simil::SpikesCRange& spikes , bool killParticles );

    void processGroupSpikes(
      const simil::SpikesCRange& spikes , bool killParticles );

    void processAttributeSpikes(
      const simil::SpikesCRange& spikes , bool killParticles );

  };

}

#endif //VISIMPL_DOMAINMANAGER_H
