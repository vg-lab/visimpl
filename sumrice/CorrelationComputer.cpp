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

#include "CorrelationComputer.h"

namespace visimpl
{
  CorrelationComputer::CorrelationComputer( simil::SpikeData* simData )
  : _simData( simData )
  , _subsetEvents( simData->subsetsEvents( ))
  , _startTime{0}
  , _endTime{-1}
  { }

  void CorrelationComputer::configureEvents( const std::vector< std::string >& eventsNames,
                                             double deltaTime )
  {
    _startTime = _simData->startTime( );
    _endTime   = std::max( _simData->subsetsEvents( )->totalTime( ), _simData->endTime( ));

    _eventNames = eventsNames;
    _eventTimeBins.clear();

    auto insertBin = [this, &deltaTime](const std::string &name)
    {
      _eventTimeBins[ name ] = _eventTimePerBin( name, _startTime, _endTime, deltaTime );
    };
    std::for_each(_eventNames.cbegin(), _eventNames.cend(), insertBin);
  }

  double CorrelationComputer::_entropy( unsigned int active, unsigned int totalBins ) const
  {
    double result = 0.0;
    const double probability = static_cast<double>(active) / static_cast<double>(totalBins);

    if( probability > 0.0 )
      result = - ( probability * std::log2( probability ) );

    if( probability < 1.0 )
      result -= (( 1.0 -  probability ) * ( std::log2( 1.0 - probability )));

    return result;
  }

  Correlation CorrelationComputer::computeCorrelation( const std::string& subset,
                                            const std::string& eventName,
                                            float initTime,
                                            float endTime,
                                            float deltaTime,
                                            float /*selectionThreshold*/ )
  {

    const GIDVec gids = _subsetEvents->getSubset( subset );
    Correlation correlation_;

    if( gids.empty( ))
    {
      std::cout << "Warning: subset " << subset << " NOT found." << std::endl;
      return correlation_;
    }

    auto eventTime = _eventTimeBins.find( eventName );
    if( eventTime == _eventTimeBins.end( ))
    {
      std::cout << "Event " << eventName << " not configured." << std::endl;
      return correlation_;
    }

    const TGIDUSet giduset( gids.begin( ), gids.end( ));

    const TSpikes& spikes = _simData->spikes( );

    enum tSRecord { tPatternFiring = 0, tNotPatternFiring, tPatternNotFiring, tNotPatternNotFiring, tTotalFiring };
    typedef std::tuple< unsigned int, unsigned int, unsigned int, unsigned int, unsigned int >  tSpikesRecord;
    std::unordered_map< uint32_t, tSpikesRecord > neuronSpikes;

    // Calculate delta time inverse to avoid further division operations.
    const double invDeltaTime = 1.0 / deltaTime;

    // Threshold for considering an event active during bin time.
    const float threshold = deltaTime * 0.5f;

    // Calculate bins number.
    const unsigned int totalBins = std::ceil( _endTime * invDeltaTime );
    const unsigned int analysisTotalBins =
        std::ceil( ( endTime - initTime ) * invDeltaTime );

    // Initialize vector storing delta time spaced event's activity.
    std::vector< unsigned int > eventBins( totalBins, 0 );

    unsigned int totalActiveBins = 0;
    unsigned int analysisActiveBins = 0;

    // Calculate the number of active bins for the current event.
    double currentTime = 0.0;
    for( unsigned int i = 0; i < totalBins; ++i, currentTime += deltaTime )
    {
      const auto time = eventTime->second[ i ];
      const bool value = ( time >= threshold );

      if( value )
      {
        eventBins[ i ] = value;
        totalActiveBins += value;

        if( currentTime >= initTime && currentTime < endTime )
          analysisActiveBins += value;
      }

    }

    const double entropyPattern = _entropy( analysisActiveBins, analysisTotalBins );

    std::vector< std::set< uint32_t >> binSpikes( totalBins );
    std::unordered_map< uint32_t, std::unordered_set< unsigned int >> neuronActiveBins;

    auto insertSpikesAndActiveBins = [&neuronSpikes, &neuronActiveBins](const uint32_t id)
    {
      neuronSpikes.insert( std::make_pair( id, std::make_tuple( 0, 0, 0, 0, 0 )));
      neuronActiveBins.insert(
          std::make_pair( id, std::unordered_set< unsigned int >( )));
    };
    std::for_each(gids.cbegin(), gids.cend(), insertSpikesAndActiveBins);


    auto processSpike = [&giduset,&binSpikes,&neuronActiveBins,invDeltaTime](const Spike &s)
    {
      const auto id = std::find(giduset.cbegin(), giduset.cend(), s.second);
      if( id == giduset.cend( ))
      {
        std::cerr << "ERROR: can't find id " << __FILE__ << " " << __LINE__ << std::endl;
        return;
      }

      unsigned int binIdx = std::floor( s.first * invDeltaTime );

      binSpikes[ binIdx ].insert( s.second );

      auto neuronBinFired = neuronActiveBins.find(s.second);
      if (neuronBinFired == neuronActiveBins.end())
      {
        std::cerr << "ERROR: can't find active bin " << __FILE__ << " " << __LINE__ << std::endl;
        return;
      }

      neuronBinFired->second.insert( binIdx );
    };
    std::for_each(spikes.cbegin(), spikes.cend(), processSpike);

    const unsigned int startBin = std::floor( initTime / deltaTime );
    const unsigned int endBin = std::ceil( endTime / deltaTime );

    auto computeStatistics = [&](const uint32_t id)
    {
      auto neuron = neuronSpikes.find(id);
      if(neuron == neuronSpikes.end()) return;

      auto& stats = neuron->second;
      auto firedBins = neuronActiveBins.find(id);
      if(firedBins == neuronActiveBins.end()) return;

      unsigned int totalFiringBins = 0;
      unsigned int firedPattern = 0;
      unsigned int firedNotPattern = 0;
      unsigned int notFiredPattern = 0;
      unsigned int notFiredNotPattern = 0;

      for( unsigned int i = startBin; i < endBin; ++i )
      {
        const auto hasFired = std::find(firedBins->second.cbegin(), firedBins->second.cend(), i);
        const bool fired = hasFired != firedBins->second.cend( );
        const bool pattern = eventBins[ i ];

        if( fired )
        {
          if( pattern )
            ++firedPattern;
          else
            ++firedNotPattern;

          ++totalFiringBins;
        }
        else
        {
          if( pattern )
            ++notFiredPattern;
          else
            ++notFiredNotPattern;
        }
      }

      std::get< tPatternFiring >( stats ) = firedPattern;
      std::get< tPatternNotFiring >( stats ) = notFiredPattern;
      std::get< tNotPatternFiring >( stats ) = firedNotPattern;
      std::get< tNotPatternNotFiring >( stats ) = notFiredNotPattern;
      std::get< tTotalFiring >( stats ) = totalFiringBins;
    };
    std::for_each(gids.cbegin(), gids.cend(), computeStatistics);

    correlation_.subsetName = subset;
    correlation_.eventName = eventName;
    correlation_.gids = giduset;

    // Calculate normalization factors by the inverse of active/inactive bins.
    const double normBins = 1.0 / analysisTotalBins;

    // Initialize maximum value probes.
    double maxHitValue = 0.0;
    double maxFHValue = 0.0;
    double maxResValue = 0.0;

    double avgHitValue = 0.0;
    double avgFHValue = 0.0;
    double avgResValue = 0.0;

    auto computeCorrelation = [&](const uint32_t id)
    {
      const auto neuronStats = neuronSpikes.find(id);
      if(neuronStats == neuronSpikes.end()) return;

      const unsigned int binsFiringPattern = std::get< tPatternFiring >( neuronStats->second );
      const unsigned int binsFiringNotPattern = std::get< tNotPatternFiring >( neuronStats->second );
      const unsigned int binsNotFiringPattern = std::get< tPatternNotFiring >( neuronStats->second );
      const unsigned int binsNotFiringNotPattern = std::get< tNotPatternNotFiring >( neuronStats->second );

      const unsigned int binsTotalFiring = std::get< tTotalFiring >( neuronStats->second );

      // Calculate corresponding values according to current event activity.
      CorrelationValues values;

      // Hit value relates to spiking neurons during active event.
      values.hit  = std::max( 0.0, std::min( 1.0, binsFiringPattern * normBins ));
      values.cr   = std::max( 0.0, std::min( 1.0, binsNotFiringNotPattern * normBins ));
      values.miss = std::max( 0.0, std::min( 1.0, binsNotFiringPattern * normBins ));
      // False hit is related to spiking neurons when event is not active.
      values.falseAlarm = std::max( 0.0, std::min( 1.0, binsFiringNotPattern * normBins ));

      values.entropy = _entropy( binsTotalFiring, analysisTotalBins );

      values.jointEntropy = 0.0;
      if( values.hit > 0 )
        values.jointEntropy -= ( values.hit * std::log2( values.hit ));

      if( values.cr > 0 )
        values.jointEntropy -= ( values.cr * std::log2( values.cr ));

      if( values.miss > 0 )
        values.jointEntropy -= ( values.miss * std::log2( values.miss ));

      if( values.falseAlarm > 0 )
        values.jointEntropy -= ( values.falseAlarm * std::log2( values.falseAlarm ));

      values.mutualInformation =
          entropyPattern + values.entropy - values.jointEntropy;

      // Result responds to Hit minus False Hit.
      values.result = values.mutualInformation;

      // Store maximum values.
      if( values.hit > maxHitValue )
        maxHitValue = values.hit;

      if( values.falseAlarm > maxFHValue )
        maxFHValue = values.falseAlarm;

      if( values.result > maxResValue )
        maxResValue = values.result;

      avgHitValue += values.hit;
      avgFHValue += values.falseAlarm;
      avgResValue += values.result;

        // Store neuron correlation value.
      correlation_.values.insert( std::make_pair( id, values ));
    };
    std::for_each(gids.cbegin(), gids.cend(), computeCorrelation);

    avgHitValue /= neuronSpikes.size( );
    avgFHValue /= neuronSpikes.size( );
    avgResValue /= neuronSpikes.size( );

    return correlation_;
  }

  std::vector< Correlation >
  CorrelationComputer::correlateSubset( const std::string& subsetName,
                                  const std::vector< std::string >& eventNames,
                                  float deltaTime,
                                  float initTime,
                                  float endTime,
                                  float selectionThreshold )
  {
    const std::vector< uint32_t > gids =
        _subsetEvents->getSubset( subsetName );

    std::vector< Correlation > result;

    if( gids.empty( ))
    {
      std::cerr << "Error: Destination GID subset " << subsetName
                << " is empty or does not exist!" << std::endl;
      return result;
    }

    auto computeAndInsertCorrelation = [&](const std::string &eventName)
    {
      const auto composedName = _composeName(subsetName, eventName);
      auto res = _correlations.find(composedName);
      if( res == _correlations.end())
      {
        auto correlation =
            computeCorrelation( subsetName, eventName, initTime, endTime,
                                deltaTime, selectionThreshold );

        correlation.fullName = composedName;

        if( !correlation.values.empty( ))
          _correlations.insert( std::make_pair( composedName, correlation ));
      }
    };
    std::for_each(eventNames.cbegin(), eventNames.cend(), computeAndInsertCorrelation);

    return result;
  }

  std::vector< float > CorrelationComputer::_eventTimePerBin( const std::string& eventName,
                                                       float startTime,
                                                       float endTime,
                                                       float deltaTime )
  {

    std::vector< float > eventTime;
    const EventVec events = _subsetEvents->getEvent( eventName );

    if( events.empty( ))
    {
      std::cout << "Warning: event " << eventName << " NOT found." << std::endl;
      return eventTime;
    }

    const double totalTime = ( endTime - startTime );
    const double invTotalTime = 1.0 / totalTime;
    const double invDeltaTime = 1.0 / deltaTime ;

    const unsigned int binsNumber = std::ceil( totalTime * invDeltaTime );

    eventTime.resize( binsNumber, 0.0f );

    for( auto eventRange : events )
    {
      const double lowerEvent = ( eventRange.first - startTime ) * invTotalTime;
      const double upperEvent = (eventRange.second - startTime ) * invTotalTime;

      unsigned int binStart = std::max(0, static_cast<int>(std::floor( lowerEvent * binsNumber ) - 1 ));
      const unsigned int binEnd =
          std::min( binsNumber, static_cast<unsigned int>(std::ceil( upperEvent * binsNumber )));

      if( binStart > binEnd )
      {
        std::cerr << "shit is wrong, dude " << binStart << " > " << binEnd << std::endl;
        continue;
      }

      double currentTime = binStart * deltaTime;
      while( currentTime > eventRange.first )
      {
        --binStart;
        currentTime = binStart * deltaTime;
      }

      double nextTime = currentTime + deltaTime;

      double eventAcc = 0.0;
      double currentAcc = 0.0;

      for( unsigned int i = binStart; i < binEnd;
          ++i, nextTime += deltaTime, currentTime += deltaTime )
      {

        const double lowerBound = std::max( currentTime, ( double ) eventRange.first );
        const double upperBound = std::min( nextTime, ( double ) eventRange.second );

        if( lowerBound > upperBound )
          continue;

        currentAcc = std::abs( upperBound - lowerBound );

        if( lowerBound < nextTime && upperBound > currentTime )
        {
          eventTime[ i ] += currentAcc;
          eventAcc += currentAcc;
        }
      }
    }

    return eventTime;
  }

  const Correlation*
  CorrelationComputer::correlation( const std::string& subsetName ) const
  {
    auto correlation = _correlations.find(subsetName);

    if( correlation == _correlations.end( ))
      return nullptr;

    return &correlation->second;
  }

  std::vector< std::string > CorrelationComputer::correlationNames( void ) const
  {
    std::vector< std::string > result;

    auto insertName = [&result](const std::pair<std::string, Correlation> &c)
    {
      result.emplace_back(c.first);
    };
    std::for_each(_correlations.cbegin(), _correlations.cend(), insertName);

    return result;
  }

  std::string CorrelationComputer::_composeName( const std::string& subsetName,
                                                 const std::string& eventName ) const
  {
    return subsetName + eventName;
  }

  GIDUSet CorrelationComputer::getCorrelatedNeurons( const std::string& correlationName ) const
  {
    GIDUSet result;

    auto correlIt = _correlations.find(correlationName);
    if( correlIt == _correlations.end( ))
    {
      std::cout << "No correlated neurons for " << correlationName << std::endl;
      return result;
    }

    const auto& correlation = correlIt->second;

    typedef std::pair< unsigned int, double > TvalueTuple;
    std::vector< TvalueTuple > sortedValues;
    sortedValues.reserve( correlation.values.size( ));

    auto insertCorrelationValues = [&sortedValues](const std::pair< uint32_t, CorrelationValues > &neuron)
    {
      sortedValues.emplace_back( std::make_pair( neuron.first, neuron.second.result ));
    };
    const auto &values = correlation.values;
    std::for_each(values.cbegin(), values.cend(), insertCorrelationValues);

    auto lessThanTuple = []( const TvalueTuple& a, const TvalueTuple& b ) { return a.second > b.second; };
    std::sort( sortedValues.begin( ), sortedValues.end( ), lessThanTuple );

    auto insertValues = [&result](const TvalueTuple &v)
    {
      result.insert(v.first);
    };
    std::for_each(sortedValues.cbegin(), sortedValues.cend(), insertValues);

    return result;
  }

} // namespace visimpl
