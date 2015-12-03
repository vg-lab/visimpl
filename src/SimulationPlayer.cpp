/*
 * SimulationPlayer.cpp
 *
 *  Created on: 3 de dic. de 2015
 *      Author: sgalindo
 */
#include "SimulationPlayer.h"

namespace visimpl
{

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
  , _blueConfigPath( blueConfigFilePath )
  , _blueConfig( nullptr )
  , _circuit( nullptr )
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
    Clear( );

    std::cout << " Loading BlueConfig: " << _blueConfigPath.c_str( ) << std::endl;

    _blueConfig = new brion::BlueConfig( _blueConfigPath );

    std::cout << " Loading Circuit: " << _blueConfig->getCircuitTarget( )
              << " -> " << _blueConfig->getCircuitSource( ).getPath( ).c_str( )
              << std::endl;

    _circuit = new brain::Circuit( *_blueConfig );

    _gids = _circuit->getGIDs( );
  }

  void SimulationPlayer::Clear( void )
  {
    if( _blueConfig )
    {
      delete _blueConfig;

      delete _circuit;

      _gids.clear( );
    }
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


  void SimulationPlayer::Play( float deltaTime_ )
  {
    _playing = true;
    _deltaTime = deltaTime_;
  }

  void SimulationPlayer::Pause( void )
  {
    _playing = !_playing;
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

  void SimulationPlayer::loop( bool loop_ )
  {
    _loop = loop_;
  }
  bool SimulationPlayer::loop( void )
  {
    return _loop;
  }

  brion::BlueConfig* SimulationPlayer::blueConfig( void )
  {
    return _blueConfig;
  }

  brain::Circuit* SimulationPlayer::circuit( void )
  {
    return _circuit;
  }

  const brion::GIDSet& SimulationPlayer::gids( void )
  {
    return _gids;
  }

  brion::Vector3fs SimulationPlayer::positions( void )
  {
    return _circuit->getPositions( _gids );
  }

  void SimulationPlayer::Finished( void )
  {
    if( _loop )
    {
      Stop( );
      Play( );
    }
  }

  //*************************************************************************
  //************************ SPIKES SIMULATION PLAYER ***********************
  //*************************************************************************


  SpikesPlayer::SpikesPlayer( const std::string& blueConfigFilePath,
                                    bool loadData )
  : SimulationPlayer( blueConfigFilePath, false )
  , _spikeReport( nullptr )
  {
    if( loadData )
      LoadData( );

  }


  void SpikesPlayer::LoadData( void )
  {
    SimulationPlayer::LoadData( );

    std::cout << "Loading spikes: "
        << _blueConfig->getSpikeSource( ).getPath( ).c_str( ) << std::endl;

    _spikeReport = new brion::SpikeReport( _blueConfig->getSpikeSource( ),
                                           brion::MODE_READ );

    _currentSpike = _spikeReport->getSpikes( ).begin( );
    _previousSpike = _currentSpike;
  }

  void SpikesPlayer::Clear( void )
  {
    SimulationPlayer::Clear( );

    delete _spikeReport;
  }

  void SpikesPlayer::Stop( void )
  {
    SimulationPlayer::Stop( );
    _currentSpike = Spikes( ).begin( );
    _previousSpike = _currentSpike;
  }

  void SpikesPlayer::FrameProcess( void )
  {
    const brion::Spikes& spikes = Spikes( );
    _previousSpike = _currentSpike;

    if( _currentSpike == spikes.end( ))
    {
        _finished = true;
        return;
    }

    SpikesCIter last;
    for( SpikesCIter spike = _currentSpike ; spike != spikes.end( ); spike++ )
    {
      if( ( *spike ).first  >= _currentTime )
      {
        _currentSpike = spike;
        break;
      }
      last = spike;
    }

  }

  const brion::Spikes& SpikesPlayer::Spikes( void )
  {
    return _spikeReport->getSpikes( );
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

}
