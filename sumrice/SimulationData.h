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

namespace visimpl
{
  typedef std::set< uint32_t > TGIDSet;
  typedef std::vector< vmml::Vector3f > TPosVect;
  typedef std::multimap< float, uint32_t > TSpikes;

  typedef enum
  {
    TUndefined = 0,
    TSpikes,
    TVoltages
  } TSimulationType;

  typedef enum
  {
    TBlueConfig = 0,
    THDF5
  } TDataType;

  class SimulationData
  {
  public:

    SimulationData( std::string filePath, TDataType dataType );

    const TGIDSet& gids( void ) const;

    const TPosVect& positions( void ) const;

    TSimulationType simulationType( void ) const;

    SimulationData* get( void ) = 0;

  protected:

    std::string filePath;

    TGIDSet _gids;

    TPosVect _positions;

    TSimulationType _simulationType;


    brion::BlueConfig* _blueConfig;

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
