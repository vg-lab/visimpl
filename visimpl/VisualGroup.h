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

namespace visimpl
{
  class VisualGroup
  {
    friend class InputMultiplexer;

  public:

    VisualGroup( );
    ~VisualGroup( );

    void gids( const GIDUSet& gids_ );
    const GIDUSet& gids( void ) const;

    void colorMapping( const TTransferFunction& colors );
    TTransferFunction colorMapping( void );

    void sizeFunction( const TSizeFunction& sizes );
    TSizeFunction sizeFunction( void );

    void model( prefr::Model* model_ );
    prefr::Model* model( void );

    bool active( void );

  protected:

    GIDUSet _gids;

    prefr::Model* _model;

    bool _active;
  };


}

#endif /* __VISIMPL_VISUALGROUP__ */
