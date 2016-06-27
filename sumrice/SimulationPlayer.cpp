/*
 * SimulationPlayer.cpp
 *
 *  Created on: 3 de dic. de 2015
 *      Author: sgalindo
 */
#include "SimulationPlayer.h"
#include "log.h"

namespace visimpl
{

  SimulationPlayer::SimulationPlayer( void )
  : _currentTime( 0.0f )
  , _previousTime( 0.0f )
  , _deltaTime( 0.0f )
  , _startTime( 0.0f )
  , _endTime( 0.0f )
  , _playing( false )
  , _loop( false )
  , _finished( false )
  , _simulationType( TSimNetwork )
  , _simData( nullptr )
  {

  }

  SimulationPlayer::SimulationPlayer( const std::string& blueConfigFilePath,
                                      bool loadData )
  : _currentTime( 0.0f )
  , _previousTime( 0.0f )
  , _deltaTime( 0.0f )
  , _startTime( 0.0f )
  , _endTime( 0.0f )
  , _playing( false )
  , _loop( false )
  , _finished( false )
  , _simulationType( TSimNetwork )
  , _blueConfigPath( blueConfigFilePath )
//  , _blueConfig( nullptr )
//  , _circuit( nullptr )
  {

    if( loadData )
      LoadData( );
  }

  SimulationPlayer::~SimulationPlayer( )
  {
    Clear( );
  }

  void SimulationPlayer::LoadData( void )
  {
//    Clear( );
//
//    std::cout << " Loading BlueConfig: " << _blueConfigPath.c_str( ) << std::endl;
//
//    _blueConfig = new brion::BlueConfig( _blueConfigPath );
//
////    brion::URI circuitSource = _blueConfig->getCircuitSource( );
////    std::cout << " Loading Circuit: " << _blueConfig->getCircuitSource( )
////              << " -> " << circuitSource.getPath( )
////              << std::endl;
//
//    _circuit = new brain::Circuit( *_blueConfig );
//
//    _gids = _circuit->getGIDs( );
//
//    std::cout << "GID Set size: " << _gids.size( ) << std::endl;
  }


  void SimulationPlayer::LoadData( TDataType dataType,
                                   const std::string& networkPath_ )
  {
    Clear( );

    switch( dataType )
    {
      case TDataType::TBlueConfig:
      case TDataType::THDF5:
      {
        _simData = new SimulationData( networkPath_, dataType );
        _gids = _simData->gids( );

        std::cout << "GID Set size: " << _gids.size( ) << std::endl;

      }
      break;

      default:
        break;
    }
  }

  void SimulationPlayer::Clear( void )
  {
//    if( _blueConfig )
//    {
//      delete _blueConfig;
//
//      delete _circuit;
//
//      _gids.clear( );
//    }

    if( _simData )
      delete _simData;

    _gids.clear( );
  }

  void SimulationPlayer::Frame( void )
  {
    if( _playing )
    {
      _previousTime = _currentTime;
      _currentTime += _deltaTime;

      FrameProcess( );
    }
  }


  void SimulationPlayer::Play( void )
  {
    _playing = true;
    _finished = false;
//    _deltaTime = deltaTime_;
  }

  void SimulationPlayer::Pause( void )
  {
    _playing = false;
  }

  void SimulationPlayer::Stop( void )
  {
    _playing = false;
    _currentTime = _startTime;
    _previousTime = _currentTime;
  }

  void SimulationPlayer::GoTo( float timeStamp )
  {
    int aux = timeStamp / _deltaTime;

    _currentTime = aux * _deltaTime;
    _previousTime = std::max( _currentTime - _deltaTime, _startTime );

  }

  void SimulationPlayer::PlayAt( float percentage )
  {
    assert( percentage >= 0.0f && percentage <= 1.0f );

    float timeStamp = percentage * ( endTime( ) - startTime( )) + startTime( );

    int aux = timeStamp / _deltaTime;

    _currentTime = aux * _deltaTime;
    _previousTime = std::max( _currentTime - _deltaTime, _startTime );

    Play( );

  }

  float SimulationPlayer::GetRelativeTime( void )
  {
    return (( _currentTime - startTime( )) / (endTime( ) - startTime( )));
  }

  bool SimulationPlayer::isPlaying( void )
  {
    return _playing;
  }

  void SimulationPlayer::deltaTime( float deltaTime_ )
  {
    _deltaTime = deltaTime_;
  }

  float SimulationPlayer::deltaTime( void )
  {
    return _deltaTime;
  }

  //void SimulationPlayer::startTime( float startTime )
  //{
  //  _startTime = startTime;
  //}

  float SimulationPlayer::startTime( void )
  {
    return _startTime;
  }

  //void SimulationPlayer::endTime( float endTime )
  //{
  //  _endTime = endTime;
  //}

  float SimulationPlayer::endTime( void )
  {
    return _endTime;
  }

  float SimulationPlayer::currentTime( void )
  {
    return _currentTime;
  }

  void SimulationPlayer::loop( bool loop_ )
  {
    _loop = loop_;
  }
  bool SimulationPlayer::loop( void )
  {
    return _loop;
  }

//  void SimulationPlayer::autonomous( bool autonomous_ )
//  {
//    _autonomous = autonomous_;
//  }
//
//  bool SimulationPlayer::autonomous( void )
//  {
//    return _autonomous;
//  }

//  brion::BlueConfig* SimulationPlayer::blueConfig( void )
//  {
//    return _blueConfig;
//  }
//
//  brain::Circuit* SimulationPlayer::circuit( void )
//  {
//    return _circuit;
//  }

  const TGIDSet& SimulationPlayer::gids( void )
  {
    return _gids;
  }

  TPosVect SimulationPlayer::positions( void )
  {
//    return _circuit->getPositions( _gids );
    return _simData->positions( );
  }

  TSimulationType SimulationPlayer::simulationType( void )
  {
    return _simulationType;
  }

  void SimulationPlayer::Finished( void )
  {
    Stop( );
    std::cout << "Finished simulation." << std::endl;
    if( _loop )
    {
      Play( );
    }
  }

#ifdef VISIMPL_USE_ZEROEQ

#ifdef VISIMPL_USE_GMRVLEX

  ZeqEventsManager* SimulationPlayer::zeqEvents( void )
  {
    return _zeqEvents;
  }
#endif

  void SimulationPlayer::connectZeq( const std::string& zeqUri )
  {
    _zeqEvents = new ZeqEventsManager( zeqUri );

    _zeqEvents->frameReceived.connect( boost::bind( &SimulationPlayer::requestPlaybackAt,
                                       this, _1 ));
  }

  void SimulationPlayer::requestPlaybackAt( float percentage )
  {
    PlayAt( percentage );
  }

  void SimulationPlayer::sendCurrentTimestamp( void )
  {
    if( _playing )
      _zeqEvents->sendFrame( _startTime, _endTime, _currentTime );
  }

#endif

//void SimulationPlayer::_setZeqUri( const std::string&
//                                   uri_
//  )
//{
//  _zeqConnection = true;
//  _uri =  servus::URI( uri_ );
//  _subscriber = new zeq::Subscriber( _uri );
//  _publisher = new zeq::Publisher( _uri );
//
//  _subscriber->registerHandler( zeq::hbp::EVENT_FRAME,
//      boost::bind( &SimulationPlayer::_onFrameEvent , this, _1 ));
//
//  pthread_create( &_subscriberThread, NULL, _Subscriber, _subscriber );
//
//}
//
//void* SimulationPlayer::_Subscriber( void* subs )
//{
//  zeq::Subscriber* subscriber = static_cast< zeq::Subscriber* >( subs );
//  while ( true )
//  {
//    subscriber->receive( 10000 );
//  }
//  pthread_exit( NULL );
//}
//
//void SimulationPlayer::_onFrameEvent( const zeq::Event& event_ )
//{
//
//  zeq::hbp::data::Frame frame = zeq::hbp::deserializeFrame( event_ );
//
//  float percentage = (frame.current - frame.start) / (frame.end - frame.start);
//
//  if( playerID == masterID )
//  {
//    playbackPositionAt( percentage );
//  }
//
//
//}
//
//#endif


  //*************************************************************************
  //************************ SPIKES SIMULATION PLAYER ***********************
  //*************************************************************************

  SpikesPlayer::SpikesPlayer( void )
  : SimulationPlayer( )
  {
    _simulationType = TSimSpikes;
  }

  SpikesPlayer::SpikesPlayer( const std::string& blueConfigFilePath,
                                    bool loadData )
  : SimulationPlayer( blueConfigFilePath, false )
//  , _spikeReport( nullptr )
  {
    if( loadData )
      LoadData( );

    _simulationType = TSimSpikes;

  }


  void SpikesPlayer::LoadData( void )
  {
//    SimulationPlayer::LoadData( );
//
//    std::cout << "Loading spikes: "
//        << _blueConfig->getSpikeSource( ).getPath( ).c_str( ) << std::endl;
//
//    _spikeReport = new brion::SpikeReport( _blueConfig->getSpikeSource( ),
//                                           brion::MODE_READ );
//
//    _currentSpike = _spikeReport->getSpikes( ).begin( );
//    _previousSpike = _currentSpike;
//
//    _startTime = _spikeReport->getStartTime( );
//    _endTime = _spikeReport->getEndTime( );
//
//    _currentTime = _startTime;

  }

  void SpikesPlayer::LoadData( TDataType dataType,
                           const std::string& networkPath,
                           const std::string& activityPath )
  {
    _simData = new SpikeData( networkPath, dataType, activityPath );

    _gids = _simData->gids( );

    std::cout << "GID Set size: " << _gids.size( ) << std::endl;

    SpikeData* spikes = dynamic_cast< SpikeData* >( _simData );

    std::cout << "Loaded " << spikes->spikes( ).size( ) << " spikes." << std::endl;

    _currentSpike = spikes->spikes( ).begin( );
    _previousSpike = _currentSpike;

    _startTime = spikes->startTime( );
    _endTime = spikes->endTime( );

    _currentTime = _startTime;
  }

  void SpikesPlayer::Clear( void )
  {
    SimulationPlayer::Clear( );

//    if( _spikeReport )
//      delete _spikeReport;
  }

  void SpikesPlayer::Stop( void )
  {
    SimulationPlayer::Stop( );
    _currentSpike = Spikes( ).begin( );
    _previousSpike = _currentSpike;
  }

  void SpikesPlayer::PlayAt( float percentage )
  {
    SimulationPlayer::PlayAt( percentage );

    const brion::Spikes& spikes = Spikes( );

    _currentSpike = Spikes( ).begin( );
    _previousSpike = _currentSpike;

    SpikesCIter last, last2 = _currentSpike;
    for( SpikesCIter spike = _currentSpike ; spike != spikes.end( ); spike++ )
    {
      if( ( *spike ).first  >= _currentTime )
      {
        _currentSpike = last;
        _previousSpike = last2;
        break;
      }
      last2 = last;
      last = spike;
    }

  }

  void SpikesPlayer::FrameProcess( void )
  {
//    const brion::Spikes& spikes = Spikes( );
    const visimpl::TSpikes& spikes = Spikes( );
    _previousSpike = _currentSpike;
    SpikesCIter last;

//    SpikesCIter aux = _currentSpike;
//    aux++;
//    if( aux  == spikes.end( ) )
//    {
//        _finished = true;
//        Finished( );
//        return;
//    }
//

//    for( SpikesCIter spike = _currentSpike ; spike != spikes.end( ); spike++ )
//    {
//      if( ( *spike ).first  >= _currentTime )
//      {
//        _currentSpike = spike;
//        break;
//      }
//      last = spike;
//    }

    SpikesCIter spike = _currentSpike;
    while( ( *spike ).first  < _currentTime )
    {
      if( spike == spikes.end( ))
      {
        _finished = true;
        Finished( );
        return;
      }
      last = spike;
      spike++;
    }
    _currentSpike = spike;
  }

  const visimpl::TSpikes& SpikesPlayer::Spikes( void )
  {
//    return _spikeReport->getSpikes( );
    return dynamic_cast< SpikeData* >( _simData )->spikes( );
  }

//  brion::SpikeReport* SpikesPlayer::spikeReport( void )
//  {
//    return _spikeReport;
//  }

  visimpl::SpikeData* SpikesPlayer::spikeReport( void ) const
  {
    return dynamic_cast< SpikeData* >( _simData );
  }

  SpikesCRange
  SpikesPlayer::spikesAtTime( float time )
  {
    return Spikes( ).equal_range( time );
  }

  SpikesCRange SpikesPlayer::spikesBetween( float startTime_, float endTime_ )
  {
    SpikesCIter start, end;

    const brion::Spikes& spikes = Spikes( );
    if ( startTime_ == endTime_ )
      return std::make_pair( spikes.end( ), spikes.end( ));
    else if( endTime_ < startTime_)
      std::swap( startTime_, endTime_ );

    SpikesCRange res = spikes.equal_range( startTime_ );
    if( res.first != res.second )
      start = res.first;
    else
    {
      for( SpikesCIter spike = spikes.begin( ); spike != spikes.end( ); spike++ )
      {
        if(( *spike ).first >= startTime_ )
        {
          start = spike;
          break;
        }
      }
    }

    res = spikes.equal_range( endTime_ );
    if( res.first != res.second )
      end = res.second--;
    else
    {
      SpikesCIter last;
      for( SpikesCIter spike = spikes.begin( ); spike != spikes.end( ); spike++ )
      {
        if(( *spike ).first < endTime_ )
        {
          last = spike;
        }
        else
        {
          end = last;
          break;
        }
      }
    }

    return std::make_pair( start, end );

  }

  SpikesCRange SpikesPlayer::spikesNow( void )
  {
    return std::make_pair( _previousSpike, _currentSpike );
  }



//*************************************************************************
//************************ VOLTAGES SIMULATION PLAYER ***********************
//*************************************************************************

  VoltagesPlayer::VoltagesPlayer( const std::string& blueConfigFilePath,
                                  const std::string& report,
                                  bool loadData,
                                  const std::pair< float, float>* range )
  : SimulationPlayer( blueConfigFilePath, false)
  , _report( report )
  , _voltReport( nullptr )
  , loadedRange( false )
  {

    if( range )
    {
      _minVoltage = range->first;
      _maxVoltage = range->second;
      loadedRange = true;
    }

    if( loadData)
      LoadData( );

    _simulationType = TSimVoltages;
  }

  void VoltagesPlayer::LoadData( )
  {
//    SimulationPlayer::LoadData( );
//
//    brion::GIDSet old = _gids;
//    brion::GIDSet gidsNew;
//    _voltReport = new brion::CompartmentReport(
//        _blueConfig->getReportSource( _report ),
//        brion::MODE_READ,
//        gidsNew );
//    brion::GIDSet other = _voltReport->getGIDs( );
//    std::cout << "GID Set size: " << gidsNew.size( ) << std::endl;
//    std::cout << "GID Set size: " << other.size( ) << std::endl;
//
//    _deltaTime = _voltReport->getTimestep( );
//    _startTime = _voltReport->getStartTime( );
//    _endTime = _voltReport->getEndTime( );
//
//    std::cout << "Offsets: " << _voltReport->getOffsets( ).size( ) << std::endl;
////
////    std::cout << "Looking for mismatches..." << std::endl;
////    for( auto gid : _gids )
////    {
////      if( other.find( gid ) == other.end( ))
////      {
////        std::cout << "GID :" << gid << " not found!" << std::endl;
////        _gids.erase( gid );
////      }
////    }
//
//    _gids = other;
//
//    std::cout << "GID Set size: " << _gids.size( ) << std::endl;
//
//    unsigned int counter = 0;
//    for( auto gid : _gids )
//    {
//      _gidRef[ gid ] = counter;
//      counter++;
//    }
//
//
//
//    std::cout << "Start time: " << _startTime << std::endl;
//    std::cout << "End time: " << _endTime << std::endl;
//    std::cout << "Delta time: " << _deltaTime << std::endl;
//    if( !loadedRange )
//    {
////      _minVoltage = std::numeric_limits< float >::max( );
////      _maxVoltage = std::numeric_limits< float >::min( );
////      for( float dt = _startTime; dt < _endTime; dt += _deltaTime )
////      {
////        std::cout << "\rdt: " << dt;
////        brion::floatsPtr frame = _voltReport->loadFrame( dt );
////        for( unsigned int i = 0; i < _gids.size( ); i++ )
////        {
////          float voltage = (* frame )[ i ];
////          if( voltage < _minVoltage )
////            _minVoltage = voltage;
////          if( voltage > _maxVoltage )
////            _maxVoltage = voltage;
////        }
////      }
////      std::cout << std::endl;
//      _minVoltage = -87.6816f;
//      _maxVoltage = 47.5082f;
//
//      Stop( );
//    }
//
//    std::cout << "Min Voltage: " << _minVoltage << std::endl;
//    std::cout << "Max Voltage: " << _maxVoltage << std::endl;
//
//    _normalizedVoltageFactor = 1.0f / std::abs( _maxVoltage - _minVoltage );
//    std::cout << "Norm factor: " << _normalizedVoltageFactor << std::endl;
  }

  void VoltagesPlayer::Clear( void )
  {
    SimulationPlayer::Clear( );

    if( _voltReport )
      delete _voltReport;

    _gidRef.clear( );
  }

  void VoltagesPlayer::Stop( void )
  {
    SimulationPlayer::Stop( );
    _voltReport->loadFrame( _startTime );

  }

  void VoltagesPlayer::PlayAt( float percentage )
  {
    SimulationPlayer::PlayAt( percentage );


  }

  void VoltagesPlayer::deltaTime( float /*deltaTime*/ )
  {
    std::cerr << "Err: Delta time cannot be modified in voltage simulations."
              << std::endl;
  }

  float VoltagesPlayer::getVoltage( uint32_t gid )
  {
    unsigned int i = _gidRef[ gid ];
    return (*_currentFrame)[ i ];
  }

  float VoltagesPlayer::minVoltage( void )
  {
    return _minVoltage;
  }

  float VoltagesPlayer::maxVoltage( void )
  {
    return _maxVoltage;
  }

  float VoltagesPlayer::getNormVoltageFactor( void )
  {
    return _normalizedVoltageFactor;
  }


  VoltIter VoltagesPlayer::begin( void )
  {
    iterator it( &(*_currentFrame)[ 0 ]);
    return it;
  }

  VoltIter VoltagesPlayer::end( void )
  {
    iterator it( &(*_currentFrame)[ _gids.size( )] );
    return it;
  }

  VoltIter VoltagesPlayer::find( uint32_t gid )
  {
    iterator it( &(*_currentFrame)[ _gidRef[ gid ]]);
    return it;
  }


  VoltCIter VoltagesPlayer::begin( void ) const
  {
    const_iterator it( &(*_currentFrame)[ 0 ]);
    return it;
  }

  VoltCIter VoltagesPlayer::end( void ) const
  {
    const_iterator it( &(*_currentFrame)[ _gids.size( )] );
    return it;
  }

  VoltCIter VoltagesPlayer::find( uint32_t gid ) const
  {
    std::unordered_map< uint32_t, unsigned int >::const_iterator res =
        _gidRef.find( gid );

    if( res == _gidRef.end( ))
      return end( );

    const_iterator it( &(*_currentFrame)[ res->second ]);
    return it;
  }

//  VoltagesCRange VoltagesPlayer::voltagesNow( void ) const
//  {
////    return std::pair< VoltCIter, VoltCIter >( begin( ), end( ));
//  }

  void VoltagesPlayer::FrameProcess( void )
  {
    _currentFrame = _voltReport->loadFrame( _currentTime );

    if( _currentFrame == 0)
    {
      std::cout << "Last frame loaded at time " << _currentTime << std::endl;
      _finished = true;
      Finished( );
      return;
    }
  }

}
