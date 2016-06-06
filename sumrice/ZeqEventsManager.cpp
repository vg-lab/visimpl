/*
 * ZeqEventsManager.cpp
 *
 *  Created on: 8 de mar. de 2016
 *      Author: sgalindo
 */

#include "ZeqEventsManager.h"

ZeqEventsManager::ZeqEventsManager( const std::string& zeqUri_ )
{

#ifdef VISIMPL_USE_ZEROEQ
  _setZeqSession( zeqUri_ );
#endif
}

ZeqEventsManager::~ZeqEventsManager( )
{
  if( _thread )
    delete _thread;

  if( _publisher )
    delete _publisher;

  if( _subscriber )
    delete _subscriber;
}

//  float ZeqEventsManager::getLastRelativePosition( void );
//  float ZeqEventsManager::getCUrrentRelativePosition( void );
//
//  float ZeqEventsManager::elapsedTime( void );
//  float ZeqEventsManager::deltaTime( void );

#ifdef VISIMPL_USE_ZEROEQ

void ZeqEventsManager::sendFrame( const float& start, const float& end,
                  const float& current ) const
{
  unsigned int factor = 10000;

//  zeq::hbp::data::Frame frame;
//  frame.current = current * factor;
//  frame.start = start * factor;
//  frame.end = end * factor;
//  frame.delta = 10000;

  lexis::render::Frame frame;

  frame.setCurrent( current * factor );
  frame.setStart( start * factor );
  frame.setEnd( end * factor );
  frame.setDelta( 10000 );

  _publisher->publish( frame );
}

void ZeqEventsManager::sendPlaybackOp( zeroeq::gmrv::PlaybackOperation operation ) const
{
  zeroeq::gmrv::PlaybackOp op;
  op.setOp(( uint32_t ) operation );
  _publisher->publish( op );
}


//void* ZeqEventsManager::_Subscriber( void* subs )
//{
////  zeroeq::Subscriber* subscriber = dynamic_cast< zeroeq::Subscriber* >( subs );
////  while ( true )
////  {
////    subscriber->receive( 10000 );
////  }
////  pthread_exit( NULL );
//}

void ZeqEventsManager::_onPlaybackOpEvent( zeroeq::gmrv::ConstPlaybackOpPtr event_ )
{
  playbackOpReceived( event_->getOp( ) );
}

void ZeqEventsManager::_onFrameEvent( /*lexis::render::ConstFramePtr event_*/ )
{
  //  _currentFrame = zeq::hbp::deserializeFrame( event_ );

  float invDelta = 1.0f / float( _currentFrame.getDelta( ) );

  float start = _currentFrame.getStart( ) * invDelta;

  float percentage;

  percentage = ( float( _currentFrame.getCurrent( ) ) * invDelta - start )
      / ( float( _currentFrame.getEnd( ) ) * invDelta - start );

//  std::cout << "Received percentage " << percentage << std::endl;
  _lastFrame = _currentFrame;

  assert( percentage >= 0.0f && percentage <= 1.0f );
  frameReceived( percentage );


}

void ZeqEventsManager::_setZeqSession( const std::string& session_ )
{
//  _zeqConnection = true;
//  _uri =  servus::URI( uri_ );
//  _subscriber = new zeq::Subscriber( _uri );
//  _publisher = new zeq::Publisher( _uri );
//
//  _subscriber->registerHandler( zeq::hbp::EVENT_FRAME,
//      boost::bind( &ZeqEventsManager::_onFrameEvent , this, _1 ));
//
//  _subscriber->registerHandler( zeq::gmrv::EVENT_PLAYBACKOP,
//      boost::bind( &ZeqEventsManager::_onPlaybackOpEvent , this, _1 ));
//
//  pthread_create( &_subscriberThread, NULL, _Subscriber, _subscriber );

  _session = session_.empty( ) ? zeroeq::DEFAULT_SESSION : session_;

  _zeroeqConnection = true;

  _subscriber = new zeroeq::Subscriber( _session );
  _publisher = new zeroeq::Publisher( _session );

  _currentFrame.registerDeserializedCallback(
      [&]( ){ _lastFrame = _currentFrame; _onFrameEvent( ); } );

  _subscriber->subscribe( _currentFrame );

  _subscriber->subscribe( zeroeq::gmrv::PlaybackOp::ZEROBUF_TYPE_IDENTIFIER(),
                          [&]( const void* data, const size_t size )
                          {
                            _onPlaybackOpEvent( zeroeq::gmrv::PlaybackOp::create( data, size ));
                          } );

//  _subscriber->subscribe( lexis::render::Frame::ZEROBUF_TYPE_IDENTIFIER(),
//                          [&]( const void* data, const size_t size )
//                          {
//                           _onFrameEvent( ::lexis::render::Frame::create( data, size ));
//                          } );

  _thread = new std::thread( [&]() { while( _zeroeqConnection ) _subscriber->receive( 10000 );});

}

#endif
