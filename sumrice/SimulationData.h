/*
 * SimulationData.h
 *
 *  Created on: 5 de abr. de 2016
 *      Author: sgalindo
 */

#ifndef __SIMULATIONDATA_H__
#define __SIMULATIONDATA_H__

#include <brion/brion.h>
#include <brain/brain.h>

#include <vmmlib/vmmlib.h>

#include <H5Cpp.h>

#include "types.h"

#include "H5Network.h"

namespace visimpl
{
  class SimulationData
  {
  public:

    SimulationData( std::string filePath, TDataType dataType );
    virtual ~SimulationData( void );

    const TGIDSet& gids( void ) const;

    const TPosVect& positions( void ) const;

    TSimulationType simulationType( void ) const;

    virtual SimulationData* get( void );

    virtual float startTime( void );
    virtual float endTime( void );

  protected:

    std::string filePath;

    TGIDSet _gids;

    TPosVect _positions;

    TSimulationType _simulationType;


    brion::BlueConfig* _blueConfig;
    H5Network* _h5Network;

    float _startTime;
    float _endTime;

  };

  class SpikeData : public SimulationData
  {
  public:

    SpikeData( const std::string& filePath, TDataType dataType,
               const std::string& report = "" );

    const TSpikes& spikes( void ) const;

    SpikeData* get( void );

  protected:

    TSpikes _spikes;
  };

  class VoltageData : public SimulationData
  {

    VoltageData( const std::string& filePath, TDataType dataType,
                 const std::string& report = ""  );

  };

}



#endif /* __SIMULATIONDATA_H__ */
