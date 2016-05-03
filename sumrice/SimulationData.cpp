/*
 * SimulationData.cpp
 *
 *  Created on: 5 de abr. de 2016
 *      Author: sgalindo
 */

#include "SimulationData.h"

namespace visimpl
{

  SimulationData::SimulationData( std::string filePath, TDataType dataType )
  : _simulationType( TUndefined )
  , _blueConfig( nullptr )
  , _h5Network( nullptr )
  {
    switch( dataType )
    {
      case TBlueConfig:
      {
        _blueConfig = new brion::BlueConfig( filePath );
        brain::Circuit* circuit = new brain::Circuit( _blueConfig );

        _gids = circuit->getGIDs( );

        _positions = circuit->getPositions( _gids );

        delete circuit;

        break;
      }
      case THDF5:
      {
        _h5Network = new H5Network( filePath );
        _h5Network->Load( );

        _gids = _h5Network->GetGIDs( );

        _positions = _h5Network->GetComposedPositions( );

        break;
      }
      default:
        break;
    }
  }

  SimulationData::~SimulationData( void )
  {

  }

  const TGIDSet& SimulationData::gids( void ) const
  {
    return _gids;
  }

  const TPosVect& SimulationData::positions( void ) const
  {
    return _positions;
  }

  TSimulationType SimulationData::simulationType( void ) const
  {
    return _simulationType;
  }

  SimulationData* SimulationData::get( void )
  {
    return this;
  }



  SpikeData::SpikeData( const std::string& filePath, TDataType dataType,
                        const std::string& report  )
  : SimulationData( filePath, dataType )
  {

    switch( dataType )
    {
      case TBlueConfig:
      {
        if( _blueConfig )
        {
          brion::SpikeReport spikeReport(  _blueConfig->getSpikeSource( ),
                                           brion::MODE_READ );
          _spikes = spikeReport.getSpikes( );
        }

        break;
      }
      case THDF5:
      {

        break;
      }
      default:
        break;
    }

  }

  const TSpikesMap& SpikeData::spikes( void ) const
  {
    return _spikes;
  }

  SpikeData* SpikeData::get( void )
  {
    return this;
  }


}


