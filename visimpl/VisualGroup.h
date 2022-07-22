/*
 * Copyright (c) 2014-2020 VG-Lab/URJC.
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

#ifndef __VISIMPL_VISUALGROUP__
#define __VISIMPL_VISUALGROUP__

// Sumrice
#include <sumrice/sumrice.h>

// Plab
#include <plab/core/Cluster.h>
#include <plab/reto/CameraModel.h>

// Visimpl
#include "visimpl/particlelab/NeuronParticle.h"
#include "visimpl/particlelab/StaticGradientModel.h"

namespace visimpl
{
  class VisualGroup
  {
    friend class OldDomainManager;

  public:
    VisualGroup(
      const std::shared_ptr< plab::ICamera >& camera ,
      const std::shared_ptr< reto::ClippingPlane >& leftPlane ,
      const std::shared_ptr< reto::ClippingPlane >& rightPlane ,
      const std::shared_ptr< plab::Renderer >& renderer ,
      bool enableClipping );

    VisualGroup(
      const std::string& name ,
      const std::shared_ptr< plab::ICamera >& camera ,
      const std::shared_ptr< reto::ClippingPlane >& leftPlane ,
      const std::shared_ptr< reto::ClippingPlane >& rightPlane ,
      const std::shared_ptr< plab::Renderer >& renderer ,
      bool enableClipping );

    ~VisualGroup( );

    unsigned int id( );

    void name( const std::string& name_ );

    const std::string& name( ) const;

    const std::vector< uint32_t >& getGids( ) const;

    void colorMapping( const TTransferFunction& colors );

    TTransferFunction colorMapping( ) const;

    void sizeFunction( const TSizeFunction& sizes );

    TSizeFunction sizeFunction( ) const;

    const std::shared_ptr< plab::Cluster< NeuronParticle >> getCluster( ) const;

    const std::shared_ptr< StaticGradientModel > getModel( ) const;

    void active( bool state );

    bool active( ) const;

    void setParticles( const std::vector< uint32_t >& gids ,
                       const std::vector< NeuronParticle >& particles );

    void setRenderer( const std::shared_ptr< plab::Renderer >& renderer );

  protected:
    unsigned int _idx;
    static unsigned int _counter;

    std::string _name;

    std::shared_ptr< plab::Cluster< NeuronParticle >> _cluster;
    std::shared_ptr< StaticGradientModel > _model;

    std::vector< uint32_t > _gids;

    bool _active;
  };
}

#endif /* __VISIMPL_VISUALGROUP__ */
