/*
 * @file  DataSource.h
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
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

    bool _active;
    bool _cached;
    bool _dirty;

    bool _custom;
  };


}

#endif /* __VISIMPL_VISUALGROUP__ */
