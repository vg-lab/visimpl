/*
 * Copyright (c) 2014-2020 GMRV/URJC.
 *
 * Authors: Sergio E. Galindo <sergio.galindo@urjc.es>
 *
 * This file is part of ViSimpl <https://github.com/gmrvvis/visimpl>
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

#include <sumrice/sumrice.h>
#include <prefr/prefr.h>

#include "prefr/SourceMultiPosition.h"

namespace visimpl
{
  class VisualGroup
  {
    friend class DomainManager;

  public:

    VisualGroup( );
    VisualGroup( const std::string& name );
    ~VisualGroup( );

    unsigned int id( void );

    void name( const std::string& name_ );
    const std::string& name( void ) const;

    void gids( const GIDUSet& gids_ );
    const GIDUSet& gids( void ) const;

    QColor color( void ) const;

    void colorMapping( const TTransferFunction& colors );
    TTransferFunction colorMapping( void ) const;

    void sizeFunction( const TSizeFunction& sizes );
    TSizeFunction sizeFunction( void ) const;

    void cluster( prefr::Cluster* cluster_ );
    prefr::Cluster* cluster( void ) const;

    void model( prefr::Model* model_ );
    prefr::Model* model( void ) const;

    void source( SourceMultiPosition* source_ );
    SourceMultiPosition* source( void ) const;

    void active( bool state, bool updateSourceState = false );
    bool active( void ) const;

    void cached( bool state );
    bool cached( void ) const;

    void dirty( bool state );
    bool dirty( void ) const;

    void custom( bool state );
    bool custom( void ) const;

  protected:

    unsigned int _idx;
    static unsigned int _counter;

    std::string _name;

    GIDUSet _gids;

    prefr::Cluster* _cluster;
    prefr::Model* _model;
    SourceMultiPosition* _source;

    QColor _color;

    bool _active;
    bool _cached;
    bool _dirty;

    bool _custom;
  };


}

#endif /* __VISIMPL_VISUALGROUP__ */
