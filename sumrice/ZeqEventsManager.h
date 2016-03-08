/*
 * ZeqEventsManager.h
 *
 *  Created on: 8 de mar. de 2016
 *      Author: sgalindo
 */

#ifndef __ZEQEVENTSMANAGER_H_
#define __ZEQEVENTSMANAGER_H_

#ifdef VISIMPL_USE_ZEQ
  #include <zeq/zeq.h>
  #include <zeq/hbp/hbp.h>
  #include <servus/uri.h>

  #include <pthread.h>
  #include <mutex>

  #include <boost/signals2/signal.hpp>
  #include <boost/bind.hpp>
#endif

class ZeqEventsManager
{
public:

  ZeqEventsManager( const std::string zeqUri_ );

//  float getLastRelativePosition( void );
//  float getCUrrentRelativePosition( void );
//
//  float elapsedTime( void );
//  float deltaTime( void );


  boost::signals2::signal< void ( float ) > frameReceived;


#ifdef VISIMPL_USE_ZEQ

public:

  void sendFrame( const float& start, const float& end,
                  const float& current );

protected:

  void _onFrameEvent( const zeq::Event& event_ );
  void _setZeqUri( const std::string& );
  static void* _Subscriber( void* subscriber );

  bool _zeqConnection;

  servus::URI _uri;
  zeq::Subscriber* _subscriber;
  zeq::Publisher* _publisher;

  pthread_t _subscriberThread;

  zeq::hbp::data::Frame _lastFrame;
  zeq::hbp::data::Frame _currentFrame;

#endif

};


#endif /* __ZEQEVENTSMANAGER_H_ */
