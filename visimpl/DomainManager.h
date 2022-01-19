/*
 * Copyright (c) 2015-2020 VG-Lab/URJC.
 *
 * Authors: Sergio E. Galindo <sergio.galindo@urjc.es>
 *
 * This file is part of ViSimpl <https://github.com/vg-lab/visimpl>
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

    DomainManager( prefr::ParticleSystem* particleSystem, const TGIDSet& gids );

#ifdef SIMIL_USE_BRION
    void init( const tGidPosMap& positions, const brion::BlueConfig* blueConfig );
#else
    void init( const tGidPosMap& positions );
#endif


    void initializeParticleSystem( void );

    VisualGroup* addVisualGroupFromSelection( const std::string& name,
                                              bool overrideGIDs = false );
    VisualGroup* addVisualGroup( const GIDUSet& group_, const std::string& name,
                                 bool overrideGIDs = false );

    void setVisualGroupState( unsigned int i, bool state, bool attrib = false );
    void removeVisualGroup( unsigned int i );

    void showInactive( bool state );


    void generateAttributesGroups( tNeuronAttributes attrib );

    void processInput( const simil::SpikesCRange& spikes_,
                       float begin, float end, bool clear );

    void update( void );

    void updateData(const TGIDSet& gids,const tGidPosMap& positions);

    void mode(const tVisualMode newMode );
    tVisualMode mode( void ) const;

    void clearView( void );

    bool showGroups( void );
    void updateGroups( void );
    void updateAttributes( void );

    void selection( const GIDUSet& newSelection );
    const GIDUSet& selection( void );

    void decay( float decayValue );
    float decay( void ) const;

    void clearSelection( void );
    void resetParticles( void );

    const std::vector< VisualGroup* >& groups( void ) const;
    const std::vector< VisualGroup* >& attributeGroups( void ) const;

    const tGidPosMap& positions( void ) const;

    void reloadPositions( void );


    const TGIDSet& gids( void ) const;

    tBoundingBox boundingBox( void ) const;

    prefr::ColorOperationModel* modelSelectionBase( void );

    const std::vector< std::pair< QColor, QColor >>& paletteColors( void ) const;

    // Statistics
    const std::vector< std::string >& namesMorpho( void ) const;
    const std::vector< std::string >& namesFunction( void ) const;

    const std::vector< long unsigned int >& attributeValues( int attribNumber ) const;
    Strings attributeNames( int attribNumber, bool labels = false ) const;

    tAppStats attributeStatistics( void ) const;

    tParticleInfo pickingInfoSimple( unsigned int particleId ) const;

    void highlightElements( const std::unordered_set< unsigned int >& highlighted );
    void clearHighlighting( void );

    /** \brief Helper method to generate que QColor pair from a given color.
     * \param[inout] c scoop::Color object reference.
     *
     */
    static std::pair<QColor, QColor> generateColorPair(scoop::Color &c);

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

    SourceMultiPosition* _getSource( unsigned int numParticles );

#ifdef SIMIL_USE_BRION
    tNeuronAttribs _loadNeuronTypes( const brion::BlueConfig& blueConfig );
#endif

    prefr::ParticleSystem* _particleSystem;

    tGidPosMap _gidPositions;

    TGIDSet _gids;

    prefr::Cluster* _clusterSelected;
    prefr::Cluster* _clusterUnselected;
    prefr::Cluster* _clusterHighlighted;

    SourceMultiPosition* _sourceSelected;

    std::vector< VisualGroup* > _groups;
    std::vector< VisualGroup* > _attributeGroups;
    tNeuronAttributes _currentAttrib;

    std::unordered_map< uint32_t, VisualGroup* > _neuronGroup;

    std::unordered_map< unsigned int, SourceMultiPosition* > _gidSource;

    tUintUMap _gidToParticle;
    tUintUMap _particleToGID;

    prefr::ColorOperationModel* _modelBase;
    prefr::ColorOperationModel* _modelOff;
    prefr::ColorOperationModel* _modelHighlighted;

    prefr::PointSampler* _sampler;
    prefr::Updater* _updater;

    tVisualMode _mode;

    GIDUSet _selection;

    float _decayValue;

    bool _showInactive;

    tBoundingBox _boundingBox;

    std::vector< std::pair< QColor, QColor >> _paletteColors;

    // Statistics
    bool _groupByName;
    bool _autoGroupByName;

    tNeuronAttribs _gidTypes;

    std::vector< std::string > _namesTypesMorpho;
    std::vector< std::string > _namesTypesFunction;

    std::vector< std::string > _namesTypesMorphoGrouped;
    std::vector< std::string > _namesTypesFunctionGrouped;

    std::vector< long unsigned int > _typesMorpho;
    std::vector< long unsigned int > _typesFunction;

    tUintUMap _typeToIdxMorpho;
    tUintUMap _typeToIdxFunction;

    tUintUMultimap _idxToTypeMorpho;
    tUintUMultimap _idxToTypeFunction;

    tUintUMap _statsMorpho;
    tUintUMap _statsFunction;

    std::unordered_map< std::string, unsigned int > _namesIdxMorpho;
    std::unordered_map< std::string, unsigned int > _namesIdxFunction;
  };
}

#endif /* __VISIMPL_VISUALGROUPMANAGER__ */
