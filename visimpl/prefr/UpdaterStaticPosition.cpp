/*
 * Copyright (c) 2015-2020 GMRV/URJC.
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

#include "UpdaterStaticPosition.h"

#include "SourceMultiPosition.h"

namespace visimpl
{
  using namespace prefr;

  UpdaterStaticPosition::UpdaterStaticPosition( void )
  : prefr::Updater( )
  { }

  UpdaterStaticPosition::~UpdaterStaticPosition( void )
  {

  }

  void UpdaterStaticPosition::updateParticle( prefr::tparticle current,
                                              float deltaTime )
    {

      unsigned int id = current.id( );
      SourceMultiPosition* source =
          dynamic_cast< SourceMultiPosition* >( _updateConfig->source( id ));

      Model* model = _updateConfig->model( id );

      assert( model );
      assert( source );


      if( _updateConfig->emitted( id ) && !current.alive( ))
      {
        current.set_life( 0 );

        current.set_alive( true );

        current.set_position( source->position( id ));
        current.set_velocity( glm::vec3( 0, 1, 0 ) );

        current.set_velocityModule( 0 );
        current.set_acceleration( glm::vec3( 0, 0, 0 ));

        _updateConfig->setEmitted( id, false );
      }

      float life = std::max( 0.0f, current.life( ) - deltaTime );

      current.set_life( life );

      float refLife = 1.0f - glm::clamp( life * model->inverseMaxLife( ),
                                    0.0f, 1.0f );

      current.set_color( model->color.GetValue( refLife ));
      current.set_size( model->size.GetValue( refLife ));
//      if( current.alive( ))
//      {
//        if( current.life( ) <= 0.0f )
//        {
//          current.set_life( 0.0f );
//          current.set_alive( false );
//
//          _updateConfig->setDead( id, true );
//        }
//      }

    }

}


