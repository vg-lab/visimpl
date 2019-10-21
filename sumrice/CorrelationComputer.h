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

    void configureEvents( const std::vector< std::string >& events,
                          double deltaTime );

    SIMIL_API std::vector< Correlation >
    correlateSubset( const std::string& subset,
               const std::vector< std::string >& events,
               float deltaTime,
               float initTime,
               float endTime,
               float selectionThreshold = 0.0f );

    SIMIL_API
    Correlation computeCorrelation( const std::string& subset,
                  const std::string& event,
                  float initTime,
                  float endTime,
                  float deltaTime = 0.125f,
                  float selectionThreshold = 0.0f);

    std::vector< std::string > correlationNames( void );

    Correlation* correlation( const std::string& subsetName );

    GIDUSet getCorrelatedNeurons( const std::string& correlationName ) const;

  protected:

    std::vector< float > _eventTimePerBin( const std::string& event,
                                    float startTime,
                                    float endTime,
                                    float deltaTime);

    double _entropy( unsigned int active, unsigned int totalBins ) const;

    std::string _composeName( const std::string& subsetName, const std::string& eventName ) const;

    simil::SpikeData* _simData;

    simil::SubsetEventManager* _subsetEvents;

    double _startTime;
    double _endTime;
    std::vector< std::string > _eventNames;
    std::unordered_map< std::string, std::vector< float >> _eventTimeBins;

    std::map< std::string, Correlation > _correlations;



  };



}



#endif /* __SIMIL_CORRELATIONCOMPUTER__ */
