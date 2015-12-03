/*
 * SimulationPlayer.h
 *
 *  Created on: 1 de dic. de 2015
 *      Author: sgalindo
 */

#ifndef __SIMULATIONPLAYER_H__
#define __SIMULATIONPLAYER_H__

#include <brion/brion.h>
#include <brain/brain.h>

namespace visimpl
{
  class SimulationPlayer
  {

  public:

    SimulationPlayer( const std::string& blueConfigFilePath,
                      bool loadData = true );

    virtual ~SimulationPlayer( );

    virtual void LoadData( void );

    virtual void Clear( void );

    virtual void Frame( void );

    virtual void Play( float deltaTime = 0.01f );

    virtual void Pause( void );

    virtual void Stop( void );

    virtual void GoTo( float timeStamp );

    void deltaTime( float deltaTime );
    float deltaTime( void );

  //  void startTime( float startTime );
    float startTime( void );

  //  void endTime( float endTime );
    float endTime( void );

    void loop( bool loop );
    bool loop( void );

    brion::BlueConfig* blueConfig( void );
    brain::Circuit* circuit( void );
    const brion::GIDSet& gids( void );
    brion::Vector3fs positions( void );

  protected:

    virtual void FrameProcess( void ) = 0;
    virtual void Finished( void );

    float _currentTime;
    float _previousTime;

    float _deltaTime;

    float _startTime;
    float _endTime;

    bool _playing;
    bool _loop;
    bool _finished;

    std::string _blueConfigPath;

    brion::BlueConfig* _blueConfig;
    brain::Circuit* _circuit;
    brion::GIDSet _gids;

  };

  typedef brion::Spikes::iterator SpikesIter;
  typedef brion::Spikes::const_iterator SpikesCIter;

  typedef std::pair< SpikesIter, SpikesIter > SpikesRange;
  typedef std::pair< SpikesCIter, SpikesCIter > SpikesCRange;

  class SpikesPlayer : public SimulationPlayer
  {
  public:

    SpikesPlayer( const std::string& blueConfigFilePath,
                     bool loadData = true );

    virtual void LoadData( void );
    virtual void Clear( void );
    virtual void Stop( void );

    virtual const brion::Spikes& Spikes( void );

    SpikesCRange spikesAtTime( float time );

    SpikesCRange spikesBetween( float startTime, float endTime );

    SpikesCRange spikesNow( void );

  protected:

    virtual void FrameProcess( void );

    SpikesCIter _previousSpike;
    SpikesCIter _currentSpike;

    brion::SpikeReport* _spikeReport;
  };

}
#endif /* SRC_SIMULATIONPLAYER_H_ */
