/*
 * Copyright (c) 2015-2020 VG-Lab/URJC.
 *
 * Authors: Sergio E. Galindo <sergio.galindo@urjc.es>
 *
 * This file is part of ViSimpl <https://github.com/vg-lab/visimpl>
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

#ifndef __SIMIL_CORRELATIONCOMPUTER__
#define __SIMIL_CORRELATIONCOMPUTER__

#include "types.h"

#include <unordered_map>
#include <simil/simil.h>
#include <sumrice/api.h>

namespace visimpl
{
  class SUMRICE_API CorrelationComputer
  {
  public:

    CorrelationComputer( simil::SpikeData* simData );

    void configureEvents( const std::vector< std::string >& events,
                          double deltaTime );

    std::vector< Correlation >
    correlateSubset( const std::string& subset,
               const std::vector< std::string >& events,
               float deltaTime,
               float initTime,
               float endTime,
               float selectionThreshold = 0.0f );

    Correlation computeCorrelation( const std::string& subset,
                  const std::string& event,
                  float initTime,
                  float endTime,
                  float deltaTime = 0.125f,
                  float selectionThreshold = 0.0f);

    std::vector< std::string > correlationNames( void ) const;

    const Correlation* correlation( const std::string& subsetName ) const;

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
