//
// Created by gaeqs on 9/06/22.
//

#include <GL/glew.h>

// ParticleLab
#include <plab/plab.h>

#include "StaticGradientModel.h"

#include <QDebug>
#include <cmath>

namespace visimpl
{

  StaticGradientModel::StaticGradientModel(
    const std::shared_ptr< plab::ICamera >& camera ,
    const std::shared_ptr< reto::ClippingPlane >& leftPlane ,
    const std::shared_ptr< reto::ClippingPlane >& rightPlane ,
    const TSizeFunction& particleSize ,
    const TColorVec& gradient ,
    bool particleVisibility ,
    bool clippingEnabled ,
    float time )
    : plab::CameraModel( camera )
    , _leftPlane( leftPlane )
    , _rightPlane( rightPlane )
    , _particleSize( particleSize )
    , _gradient( gradient )
    , _particleVisibility( particleVisibility )
    , _clippingEnabled( clippingEnabled )
    , _time( time )
  {

  }

  const std::shared_ptr< reto::ClippingPlane >&
  StaticGradientModel::getLeftPlane( ) const
  {
    return _leftPlane;
  }

  void StaticGradientModel::setLeftPlane(
    const std::shared_ptr< reto::ClippingPlane >& leftPlane )
  {
    _leftPlane = leftPlane;
  }

  const std::shared_ptr< reto::ClippingPlane >&
  StaticGradientModel::getRightPlane( ) const
  {
    return _rightPlane;
  }

  void StaticGradientModel::setRightPlane(
    const std::shared_ptr< reto::ClippingPlane >& rightPlane )
  {
    _rightPlane = rightPlane;
  }

  TSizeFunction StaticGradientModel::getParticleSize( ) const
  {
    return _particleSize;
  }

  void StaticGradientModel::setParticleSize( const TSizeFunction& particleSize )
  {
    _particleSize = particleSize;
  }

  const TColorVec& StaticGradientModel::getGradient( ) const
  {
    return _gradient;
  }

  void StaticGradientModel::setGradient( const TColorVec& gradient )
  {
    _gradient = gradient;
  }

  bool StaticGradientModel::isParticleVisibility( ) const
  {
    return _particleVisibility;
  }

  void StaticGradientModel::setParticleVisibility( bool particleVisibility )
  {
    _particleVisibility = particleVisibility;
  }

  bool StaticGradientModel::isClippingEnabled( ) const
  {
    return _clippingEnabled;
  }

  void StaticGradientModel::enableClipping( bool enabled )
  {
    _clippingEnabled = enabled;
  }

  float StaticGradientModel::getTime( ) const
  {
    return _time;
  }

  void StaticGradientModel::setTime( float time )
  {
    _time = time;
  }

  void StaticGradientModel::addTime( float time , float endTime )
  {
    _time = std::fmod( _time + time , endTime );
  }

  void
  StaticGradientModel::uploadDrawUniforms( plab::UniformCache& cache ) const
  {
    constexpr int MAX_COLORS = 256;
    constexpr int MAX_SIZES = 16;

    CameraModel::uploadDrawUniforms( cache );

    glUniform1f( cache.getLocation( "time" ) , _time );


    {
      int maxSize = std::min( MAX_SIZES ,
                              static_cast<int>(_particleSize.size( )));
      glUniform1i( cache.getLocation( "sizeSize" ) , maxSize );

      float timeStamps[MAX_SIZES];
      float values[MAX_SIZES];

      for ( int i = 0; i < maxSize; i++ )
      {
        auto& item = _particleSize.at( i );
        timeStamps[ i ] = item.first;
        values[ i ] = item.second;
      }

      glUniform1fv( cache.getLocation( "sizeTimes" ) , maxSize ,
                    timeStamps );
      glUniform1fv( cache.getLocation( "sizeValues" ) , maxSize , values );
    }

    {
      int maxSize = std::min( MAX_COLORS , static_cast<int>(_gradient.size( )));
      glUniform1i( cache.getLocation( "gradientSize" ) , maxSize );

      float timeStamps[MAX_COLORS];
      glm::vec4 colors[MAX_COLORS];

      for ( int i = 0; i < maxSize; i++ )
      {
        auto& item = _gradient.at( i );
        timeStamps[ i ] = item.first;
        colors[ i ] = item.second;
      }

      glUniform1fv( cache.getLocation( "gradientTimes" ) , maxSize ,
                    timeStamps );
      glUniform4fv( cache.getLocation( "gradientColors" ) , maxSize ,
                    ( float* ) colors );
    }

    glUniform1f( cache.getLocation( "particlePreVisibility" ) ,
                 _particleVisibility ? 1.0f : 0.0f );

    // Clipping

    if ( _clippingEnabled )
    {
      glEnable( GL_CLIP_DISTANCE0 );
      glEnable( GL_CLIP_DISTANCE1 );
      glUniform4fv( cache.getLocation( "plane[0]" ) , 1 ,
                    _leftPlane->getEquation( ).data( ));
      glUniform4fv( cache.getLocation( "plane[1]" ) , 1 ,
                    _rightPlane->getEquation( ).data( ));

      glUniform1i(
        cache.getLocation( "isLocal[0]" ) ,
        _leftPlane->getClippingMode( ) == reto::ClippingMode::Local );
      glUniform1i(
        cache.getLocation( "isLocal[1]" ) ,
        _rightPlane->getClippingMode( ) == reto::ClippingMode::Local );
    }
    else
    {
      glDisable( GL_CLIP_DISTANCE0 );
      glDisable( GL_CLIP_DISTANCE1 );
    }

  }
}