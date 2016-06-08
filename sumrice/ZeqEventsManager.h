/*
 * ZeqEventsManager.h
 *
 *  Created on: 8 de mar. de 2016
 *      Author: sgalindo
 */

#ifndef __ZEQEVENTSMANAGER_H_
#define __ZEQEVENTSMANAGER_H_

#ifdef VISIMPL_USE_ZEROEQ
  #include <zeroeq/zeroeq.h>
//  #include <zeroeq/hbp/hbp.h>
//  #include <servus/uri.h>

//  #include <pthread.h>
  #include <mutex>

  #include <boost/signals2/signal.hpp>
  #include <boost/bind.hpp>

#ifdef VISIMPL_USE_LEXIS
#include <lexis/lexis.h>
#endif

#ifdef VISIMPL_USE_GMRVLEX
  #include <gmrvlex/gmrvlex.h>
#endif

#include <thread>

#endif

class ZeqEventsManager
{
public:

  ZeqEventsManager( const std::string& zeroeqUri_ );
  ~ZeqEventsManager( );
//  float getLastRelativePosition( void );
//  float getCUrrentRelativePosition( void );
//
//  float elapsedTime( void );
//  float deltaTime( void );


  boost::signals2::signal< void ( float ) > frameReceived;
  boost::signals2::signal< void ( unsigned int ) > playbackOpReceived;


#ifdef VISIMPL_USE_ZEROEQ

public:

  void sendFrame( const float& start, const float& end,
                  const float& current ) const;

#ifdef VISIMPL_USE_GMRVLEX

  void sendPlaybackOp( zeroeq::gmrv::PlaybackOperation operation ) const;

protected:
  void _onPlaybackOpEvent( zeroeq::gmrv::ConstPlaybackOpPtr event_ );

#endif

protected:

  void _onFrameEvent( /*lexis::render::ConstFramePtr event_*/ );
  void _setZeqSession( const std::string& );
//  static void* _Subscriber( void* subscriber );

  bool _zeroeqConnection;

  std::string _session;
  zeroeq::Subscriber* _subscriber;
  zeroeq::Publisher* _publisher;

  pthread_t _subscriberThread;

//  zeroeq::hbp::data::Frame _lastFrame;
//  zeroeq::hbp::data::Frame _currentFrame;

  lexis::render::Frame _lastFrame;
  lexis::render::Frame _currentFrame;

  std::thread* _thread;

#endif

};


#endif /* __ZEQEVENTSMANAGER_H_ */
