/*
 * ZeqEventsManager.cpp
 *
 *  Created on: 8 de mar. de 2016
 *      Author: sgalindo
 */

#include "ZeqEventsManager.h"


ZeqEventsManager::ZeqEventsManager( const std::string& zeqUri_ )
{

#ifdef VISIMPL_USE_ZEQ
  _setZeqUri( zeqUri_ );
#endif
}



//  float ZeqEventsManager::getLastRelativePosition( void );
//  float ZeqEventsManager::getCUrrentRelativePosition( void );
//
//  float ZeqEventsManager::elapsedTime( void );
//  float ZeqEventsManager::deltaTime( void );

#ifdef VISIMPL_USE_ZEQ

void ZeqEventsManager::sendFrame( const float& start, const float& end,
                  const float& current ) const
{
  unsigned int factor = 10000;

  zeq::hbp::data::Frame frame;
  frame.current = current * factor;
  frame.start = start * factor;
  frame.end = end * factor;
  frame.delta = 10000;

  _publisher->publish( zeq::hbp::serializeFrame( frame ));
}

void ZeqEventsManager::sendPlaybackOp( zeq::gmrv::PlaybackOperation operation ) const
{
  _publisher->publish( zeq::gmrv::serializePlaybackOperation( operation ));
}


void* ZeqEventsManager::_Subscriber( void* subs )
{
  zeq::Subscriber* subscriber = static_cast< zeq::Subscriber* >( subs );
  while ( true )
  {
    subscriber->receive( 10000 );
  }
  pthread_exit( NULL );
}

void ZeqEventsManager::_onPlaybackOpEvent( const zeq::Event& event_ )
{
  zeq::gmrv::PlaybackOperation operation =
      zeq::gmrv::deserializePlaybackOperation( event_ );

  playbackOpReceived( ( unsigned int ) operation );

}

void ZeqEventsManager::_onFrameEvent( const zeq::Event& event_ )
{
  _lastFrame = _currentFrame;
  _currentFrame = zeq::hbp::deserializeFrame( event_ );

  float invDelta = 1.0f / float( _currentFrame.delta );

  float start = _currentFrame.start * invDelta;

  float percentage;

//    percentage = (float(_currentFrame.current - _currentFrame.start) /
//                       float(_currentFrame.end - _currentFrame.start) * invDelta) ;

  percentage = ( float( _currentFrame.current ) * invDelta - start )
      / ( float( _currentFrame.end ) * invDelta - start );

  std::cout << "Received percentage " << percentage << std::endl;

  assert( percentage >= 0.0f && percentage <= 1.0f );
  frameReceived( percentage );

}

void ZeqEventsManager::_setZeqUri( const std::string& uri_ )
{
  _zeqConnection = true;
  _uri =  servus::URI( uri_ );
  _subscriber = new zeq::Subscriber( _uri );
  _publisher = new zeq::Publisher( _uri );

  _subscriber->registerHandler( zeq::hbp::EVENT_FRAME,
      boost::bind( &ZeqEventsManager::_onFrameEvent , this, _1 ));

  _subscriber->registerHandler( zeq::gmrv::EVENT_PLAYBACKOP,
      boost::bind( &ZeqEventsManager::_onPlaybackOpEvent , this, _1 ));

  pthread_create( &_subscriberThread, NULL, _Subscriber, _subscriber );

}

#endif
