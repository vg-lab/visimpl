/*
 * @file  CorrelationComputer.h
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */
#ifndef __SIMIL_CORRELATIONCOMPUTER__
#define __SIMIL_CORRELATIONCOMPUTER__

#include "types.h"

#include <unordered_map>
#include <simil/simil.h>

namespace visimpl
{

  class CorrelationComputer
  {
  public:

    SIMIL_API
    CorrelationComputer( simil::SpikeData* simData );

    SIMIL_API std::vector< Correlation >
    correlate( const std::string& subset,
               const std::vector< std::string >& events,
               float deltaTime = 0.125f,
               float selectionThreshold = 0.0f );

    SIMIL_API
    void compute( const std::string& subset,
                  const std::string& event,
                  float initTime,
                  float endTime,
                  float deltaTime = 0.125f,
                  float selectionThreshold = 0.0f);

    std::vector< std::string > correlationNames( void );

    Correlation* correlation( const std::string& subsetName );

  protected:

    std::vector< float > _eventTimePerBin( const std::string& event,
                                    float startTime,
                                    float endTime,
                                    float deltaTime);

    double _entropy( unsigned int active, unsigned int totalBins ) const;

    simil::SpikeData* _simData;

    simil::SubsetEventManager* _subsetEvents;

    std::unordered_map< std::string, std::vector< float >> _eventTimeBins;

    std::map< std::string, Correlation > _correlations;

  };



}



#endif /* __SIMIL_CORRELATIONCOMPUTER__ */
