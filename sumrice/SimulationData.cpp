/*
 * SimulationData.cpp
 *
 *  Created on: 5 de abr. de 2016
 *      Author: sgalindo
 */

#include "SimulationData.h"

#include "H5Activity.h"

namespace visimpl
{

  SimulationData::SimulationData( std::string filePath_,
                                  TDataType dataType )
  : _simulationType( TSimNetwork )
  , _blueConfig( nullptr )
  , _h5Network( nullptr )
  {
    switch( dataType )
    {
      case TBlueConfig:
      {
        _blueConfig = new brion::BlueConfig( filePath_ );
        brain::Circuit* circuit = new brain::Circuit( *_blueConfig );

        _gids = circuit->getGIDs( );

        _positions = circuit->getPositions( _gids );

        delete circuit;

        break;
      }
      case THDF5:
      {
        _h5Network = new H5Network( filePath_ );
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

  float SimulationData::startTime( void )
  {
    return _startTime;
  }

  float SimulationData::endTime( void )
  {
    return _endTime;
  }



  SpikeData::SpikeData( const std::string& filePath_, TDataType dataType,
                        const std::string& report  )
  : SimulationData( filePath_, dataType )
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

          _startTime = spikeReport.getStartTime( );
          _endTime = spikeReport.getEndTime( );
        }

        break;
      }
      case THDF5:
      {
        H5Spikes spikeReport( *_h5Network, report );
        spikeReport.Load( );

        _spikes = spikeReport.spikes( );

        _startTime = spikeReport.startTime( );
        _endTime = spikeReport.endTime( );

        break;
      }
      default:
        break;
    }

  }

  const TSpikes& SpikeData::spikes( void ) const
  {
    return _spikes;
  }

  SpikeData* SpikeData::get( void )
  {
    return this;
  }


}


