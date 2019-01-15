/*
 * @file  CorrelationComputer.cpp
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */

#include "CorrelationComputer.h"

namespace visimpl
{

  CorrelationComputer::CorrelationComputer( simil::SpikeData* simData )
  : _simData( simData )
  , _subsetEvents( simData->subsetsEvents( ))
  { }

  void CorrelationComputer::configureEvents( const std::vector< std::string >& eventsNames,
                                             double deltaTime )
  {
    _startTime = _simData->startTime( );
    _endTime =
        std::max( _simData->subsetsEvents( )->totalTime( ), _simData->endTime( ));

    std::cout << "Simulation time: [" << _startTime
              << ", " << _endTime << "]"
              << std::endl;

    _eventNames = eventsNames;

    for( auto eventName : eventsNames )
    {
      _eventTimeBins[ eventName ] =
          _eventTimePerBin( eventName, _startTime, _endTime, deltaTime );
    }

  }


  double CorrelationComputer::_entropy( unsigned int active, unsigned int totalBins ) const
  {
    double result = 0.0;
//    double invTotal = 1.0 ;
    double probability = ( double ) active / ( double )totalBins;

//    std::cout << "probability " << probability << std::endl;

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

    GIDVec gids = _subsetEvents->getSubset( subset );
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

    TGIDUSet giduset( gids.begin( ), gids.end( ));

    const TSpikes& spikes = _simData->spikes( );

    enum tSRecord { tPatternFiring = 0, tNotPatternFiring, tPatternNotFiring, tNotPatternNotFiring, tTotalFiring };
    typedef std::tuple< unsigned int, unsigned int, unsigned int, unsigned int, unsigned int >  tSpikesRecord;
    std::unordered_map< uint32_t, tSpikesRecord > neuronSpikes;
//    std::map< uint32_t, unsigned int > totalNeuronSpikes;

    // Calculate delta time inverse to avoid further division operations.
    double invDeltaTime = 1.0 / deltaTime;

    // Threshold for considering an event active during bin time.
    float threshold = deltaTime * 0.5f;

    // Calculate bins number.
    unsigned int totalBins = std::ceil( _endTime * invDeltaTime );
    unsigned int analysisTotalBins =
        std::ceil( ( endTime - initTime ) * invDeltaTime );

    // Initialize vector storing delta time spaced event's activity.
    std::vector< unsigned int > eventBins( totalBins, 0 );

    unsigned int totalActiveBins = 0;
    unsigned int analysisActiveBins = 0;

    // Calculate the number of active bins for the current event.
    double currentTime = 0.0;
    for( unsigned int i = 0; i < totalBins; ++i, currentTime += deltaTime )
    {
      auto time = eventTime->second[ i ];
      bool value = ( time >= threshold );

      if( value )
      {
        eventBins[ i ] = value;
        totalActiveBins += value;

        if( currentTime >= initTime && currentTime < endTime )
          analysisActiveBins += value;
      }

    }


    std::cout << "Pattern " << eventName
              << " time [" << initTime << ", " << endTime << "]"
              << " " << totalActiveBins << "/" << totalBins
              << " analysis " << analysisActiveBins << "/" << analysisTotalBins
              << std::endl;

//    double invTotalBins = 1.0 / binsNumber;

    double entropyPattern = _entropy( analysisActiveBins, analysisTotalBins );

    std::cout << "Pattern " << eventName << " entropy " << entropyPattern << std::endl;

    std::vector< std::set< uint32_t >> binSpikes( totalBins );
    std::unordered_map< uint32_t, std::unordered_set< unsigned int >> neuronActiveBins;

    for( auto gid : gids )
    {
      neuronSpikes.insert( std::make_pair( gid, std::make_tuple( 0, 0, 0, 0, 0 )));
      neuronActiveBins.insert(
          std::make_pair( gid, std::unordered_set< unsigned int >( )));
    }

    for( const auto& spike : spikes )
    {
      if( giduset.find( spike.second ) == giduset.end( ))
        continue;

      unsigned int binIdx = std::floor( spike.first * invDeltaTime );

      binSpikes[ binIdx ].insert( spike.second );

      auto neuronBinFired = neuronActiveBins.find( spike.second );
      neuronBinFired->second.insert( binIdx );
    }

    unsigned int startBin = std::floor( initTime / deltaTime );
    unsigned int endBin = std::ceil( endTime / deltaTime );

    std::cout << "Bin range [" << startBin << ", " << endBin << "] "
              << initTime << " " << endTime << std::endl;

    for( auto gid : gids )
    {
      auto neuron = neuronSpikes.find( gid );

//      auto gid = neuron->first;
      auto& stats = neuron->second;

      auto firedBins = neuronActiveBins.find( gid );

      unsigned int totalFiringBins = 0;
      unsigned int firedPattern = 0;
      unsigned int firedNotPattern = 0;
      unsigned int notFiredPattern = 0;
      unsigned int notFiredNotPattern = 0;

      for( unsigned int i = startBin; i < endBin; ++i )
      {
        bool fired = firedBins->second.find( i ) != firedBins->second.end( );
        bool pattern = eventBins[ i ];

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
    }

//    for( unsigned int i = 0; i < binSpikes.size( ); ++i )
//    {
//      auto neuronsPerBin = binSpikes[ i ];
//
//      for( auto neuron : neuronsPerBin )
//      {
//        auto it = neuronSpikes.find( neuron );
//
//        if( eventBins[ i ])
//        {
//            std::get< 0 >( it->second )++;
//        }
//        else
//          std::get< 1 >( it->second )++;
//
//        std::get< 2 >( it->second )++;
//      }
//    }


    correlation_.subsetName = subset;
    correlation_.eventName = eventName;
    correlation_.gids = giduset;

    // Calculate normalization factors by the inverse of active/inactive bins.
    double normBins = 1.0 / analysisTotalBins;
//    double normHit = 1.0 / analysisActiveBins;
//    double normFH = 1.0 / ( totalBins - totalActiveBins );

    // Initialize maximum value probes.
    double maxHitValue = 0.0;
    double maxFHValue = 0.0;
    double maxResValue = 0.0;

    double avgHitValue = 0.0;
    double avgFHValue = 0.0;
    double avgResValue = 0.0;


    unsigned int binsFiringPattern = 0;
    unsigned int binsFiringNotPattern = 0;
    unsigned int binsNotFiringPattern = 0;
    unsigned int binsNotFiringNotPattern = 0;
    unsigned int binsTotalFiring = 0;

    // For each neuron...
    for( auto gid : gids )
    {

      auto neuronStats = neuronSpikes.find( gid );
//      auto neuronTotalSpikes = totalNeuronSpikes.find( gid );

      binsFiringPattern = std::get< tPatternFiring >( neuronStats->second );
      binsFiringNotPattern = std::get< tNotPatternFiring >( neuronStats->second );
      binsNotFiringPattern = std::get< tPatternNotFiring >( neuronStats->second );
      binsNotFiringNotPattern = std::get< tNotPatternNotFiring >( neuronStats->second );

      binsTotalFiring = std::get< tTotalFiring >( neuronStats->second );

//      unsigned int binsFiringTotal = std::get< tTotalFiring >( neuronStats->second );
//      unsigned int binsRest =

      // Calculate corresponding values according to current event activity.
      CorrelationValues values;

      // Hit value relates to spiking neurons during active event.
      values.hit = std::max( 0.0, std::min( 1.0, binsFiringPattern * normBins ));
      values.cr = std::max( 0.0, std::min( 1.0, binsNotFiringNotPattern * normBins ));
      values.miss = std::max( 0.0, std::min( 1.0, binsNotFiringPattern * normBins ));
      // False hit is related to spiking neurons when event is not active.
      values.falseAlarm = std::max( 0.0, std::min( 1.0, binsFiringNotPattern * normBins ));

      values.entropy = _entropy( binsTotalFiring, analysisTotalBins );

      values.jointEntropy = - ( values.hit * std::log2( values.hit ));
      values.jointEntropy -= ( values.cr * std::log2( values.cr ));
      values.jointEntropy -= ( values.miss * std::log2( values.miss ));
      values.jointEntropy -= ( values.falseAlarm * std::log2( values.falseAlarm ));

      values.mutualInformation =
          entropyPattern + values.entropy - values.jointEntropy;

      // Result responds to Hit minus False Hit.
      values.result = values.mutualInformation;

//      std::cout << "Neuron " << gid
////                << " " << binsFiringPattern
////                << " " << binsNotFiringNotPattern
////                << " " << binsNotFiringPattern
////                << " " << binsFiringNotPattern
//                << " Hit " << values.hit
//                << " CR " << values.cr
//                << " Miss " << values.miss
//                << " FA " << values.falseAlarm
//                << " ent " << values.entropy
//                << " joint " << values.jointEntropy
//                << " MI " << values.mutualInformation
//                << std::endl;


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

      // If result is above the given threshold...
//      if( values.result >= selectionThreshold )

        // Store neuron correlation value.
      correlation_.values.insert( std::make_pair( gid, values ));

    }

    avgHitValue /= neuronSpikes.size( );
    avgFHValue /= neuronSpikes.size( );
    avgResValue /= neuronSpikes.size( );

    std::cout << "Hit: Max value: " << maxHitValue << std::endl;
    std::cout << "False hit: Max value " << maxFHValue << std::endl;
    std::cout << "Result: Max value " << maxResValue << std::endl;

    std::cout << "Hit: Avg " << avgHitValue << std::endl;
    std::cout << "False hit: Avg " << avgFHValue << std::endl;
    std::cout << "Result: Avg " << avgResValue << std::endl;


    // Store the full subset correlation for filtered neurons.
//    _correlations.insert( std::make_pair( , correlation_ ));

    std::cout << "Computed correlation for event " << subset
              << " with "<< correlation_.values.size( ) << " elements."
              << std::endl;

    std::cout << "-------------------------" << std::endl;

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
    std::vector< uint32_t > gids =
        _subsetEvents->getSubset( subsetName );

    std::vector< Correlation > result;

    if( gids.empty( ))
    {
      std::cerr << "Error: Destination GID subset " << subsetName
                << " is empty or does not exist!" << std::endl;
      return result;
    }

    std::vector< std::string > composedNames( eventNames.size( ));

    for( unsigned int i = 0; i < eventNames.size( ); ++i )
    {
      auto eventName = eventNames[ i ];
      auto composedName = _composeName( subsetName, eventName );

      composedNames[ i ] = composedName;

      auto res = _correlations.find( composedName );

      if( res == _correlations.end( ))
      {
        auto correlation =
            std::move( computeCorrelation( subsetName, eventName, initTime, endTime, deltaTime, selectionThreshold ));

        correlation.fullName = composedNames[ i ];

        if( !correlation.values.empty( ))
          res = _correlations.insert( std::make_pair( composedName, correlation )).first;
      }
    }

//    unsigned int counter = 0;

//    result.reserve( eventNames.size( ));
//    for( auto& c : result )
//    {
//      c.subsetName = subsetName;
//      c.eventName = eventNames[ counter ];
//
//      ++counter;
//    }

//    for( auto gid : gids )
//    {
//      CorrelationValues max;
//      max.falseAlarm = max.hit = max.result = std::numeric_limits< float >::min( );
//
//      int maxNumber = -1;
//
//      counter = 0;
//      for( const auto corr : impliedCorrelations )
//      {
//        auto res = corr->values.find( gid );
//        if( res != corr->values.end( ) && res->second > max )
//        {
//          max = res->second;
//          maxNumber = counter;
//        }
//
//        ++counter;
//      }
//
//      if( maxNumber > -1 )
//        result[ maxNumber ].values.insert( std::make_pair( gid, max ));
//    }

    return result;
  }

  std::vector< float > CorrelationComputer::_eventTimePerBin( const std::string& eventName,
                                                       float startTime,
                                                       float endTime,
                                                       float deltaTime )
  {

    std::vector< float > eventTime;
    EventVec events = _subsetEvents->getEvent( eventName );

    if( events.empty( ))
    {
      std::cout << "Warning: event " << eventName << " NOT found." << std::endl;
      return eventTime;
    }


    double totalTime = ( endTime - startTime );
    double invTotalTime = 1.0 / totalTime;

    double invDeltaTime = 1.0 / deltaTime ;


    unsigned int binsNumber = std::ceil( totalTime * invDeltaTime );

    eventTime.resize( binsNumber, 0.0f );

    for( auto eventRange : events )
    {
      double currentTime = 0.0f;
      double nextTime = 0.0f;
      unsigned int counter = 0;

      double lowerBound;
      double upperBound;

      double lowerEvent = ( eventRange.first - startTime ) * invTotalTime;
      double upperEvent = (eventRange.second - startTime ) * invTotalTime;

      unsigned int binStart =
          std::max( ( int ) 0,
                    ( int ) std::floor( lowerEvent * binsNumber ) - 1 );
      unsigned int binEnd =
          std::min( binsNumber, ( unsigned int ) std::ceil( upperEvent * binsNumber ));

//      if( binEnd > binsNumber )
//        binEnd = binsNumber;

//      std::cout << "Calculated bin range: [" << binStart
//                << ", " << binEnd << ")"<< std::endl;

      if( binStart > binEnd )
      {
        std::cerr << "shit is wrong, dude " << binStart << " > " << binEnd << std::endl;
        continue;
      }

      currentTime = binStart * deltaTime;
      while( currentTime > eventRange.first )
      {
        --binStart;
        currentTime = binStart * deltaTime;
      }

      nextTime = currentTime + deltaTime;

      double eventAcc = 0.0;
      double currentAcc = 0.0;

      for( unsigned int i = binStart; i < binEnd;
          ++i, nextTime += deltaTime, currentTime += deltaTime )
//      for( auto binIt = eventTime.begin( ) + binStart;
//           binIt != eventTime.begin( ) + binEnd; ++binIt )
      {

//        if( nextTime < eventRange.first || currentTime > eventRange.second )
//          continue;

        lowerBound = std::max( currentTime, ( double ) eventRange.first );
        upperBound = std::min( nextTime, ( double ) eventRange.second );

        if( lowerBound > upperBound )
          continue;

        currentAcc = std::abs( upperBound - lowerBound );
//        *binIt += ( upperBound - lowerBound );

        if( lowerBound < nextTime && upperBound > currentTime )
        {
          eventTime[ i ] += currentAcc;
          eventAcc += currentAcc;


//          std::cout << i << " e [" << eventRange.first << ", " << eventRange.second
//                    << "] t [" << currentTime << ", " << nextTime
//                    << "] -> [" << lowerBound << ", " << upperBound << ")"
//                    << " -> " << currentAcc << " " << eventAcc << "/" << eventRange.second - eventRange.first
//                    << " - " << ( int ) ( lowerBound == eventRange.first )
//                    << " " << ( int ) ( upperBound == ( eventRange.second ))
//                    << std::endl;
        }

        counter++;
      }

    }

    return eventTime;
  }

  Correlation*
  CorrelationComputer::correlation( const std::string& subsetName )
  {
    auto it = _correlations.find( subsetName );

    if( it == _correlations.end( ))
      return nullptr;

    return &it->second;
  }

  std::vector< std::string > CorrelationComputer::correlationNames( void )
  {
    std::vector< std::string > result;

    for( auto c : _correlations )
      result.push_back( c.first );

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

    auto correl = _correlations.find( correlationName );

    if( correl == _correlations.end( ))
    {
      std::cout << "No correlated neurons for " << correlationName << std::endl;
      return result;
    }

    const auto& correlation = correl->second;

    unsigned int eventsNumber = _eventNames.size( );
    if( eventsNumber == 1 )
      ++eventsNumber;

    unsigned int selectionNumber = correlation.gids.size( ) / eventsNumber;

    typedef std::pair< unsigned int, double > TvalueTuple;
     std::vector< TvalueTuple > sortedValues;
     sortedValues.reserve( correlation.values.size( ));

     for( auto neuron : correlation.values )
     {
       sortedValues.emplace_back( std::make_pair( neuron.first, neuron.second.result ));
     }

     std::sort( sortedValues.begin( ), sortedValues.end( ),
                [ ]( const TvalueTuple& a, const TvalueTuple& b )
                { return a.second > b.second; });

     std::cout << "Correlated neurons: (" << selectionNumber
               << "/" << sortedValues.size( ) << ")" << std::endl;;

     for( unsigned int i = 0; i < selectionNumber; ++i )
     {
       auto value = sortedValues[ i ];
       std::cout << " " << value.first; //<< ":" << value.second;
       result.insert( value.first );
     }

     std::cout << std::endl;

     return result;
  }

} // namespace visimpl
