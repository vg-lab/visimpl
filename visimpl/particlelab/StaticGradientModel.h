//
// Created by gaeqs on 9/06/22.
//

#ifndef SYNCOPA_STATICPARTICLEMODEL_H
#define SYNCOPA_STATICPARTICLEMODEL_H

#include <sumrice/types.h>
#include <reto/ClippingSystem.h>
#include <glm/vec4.hpp>

namespace visimpl
{

  class StaticGradientModel : public plab::CameraModel
  {

    std::shared_ptr< reto::ClippingPlane > _leftPlane;
    std::shared_ptr< reto::ClippingPlane > _rightPlane;

    TSizeFunction _particleSize;
    TColorVec _gradient;
    bool _particleVisibility;
    bool _clippingEnabled;
    float _time;

  public:

    StaticGradientModel(
      const std::shared_ptr< plab::ICamera >& camera ,
      const std::shared_ptr< reto::ClippingPlane >& leftPlane ,
      const std::shared_ptr< reto::ClippingPlane >& rightPlane ,
      const TSizeFunction& particleSize ,
      const TColorVec& gradient ,
      bool particleVisibility ,
      bool clippingEnabled ,
      float time );

    const std::shared_ptr< reto::ClippingPlane >& getLeftPlane( ) const;

    void
    setLeftPlane( const std::shared_ptr< reto::ClippingPlane >& leftPlane );

    const std::shared_ptr< reto::ClippingPlane >& getRightPlane( ) const;

    void
    setRightPlane( const std::shared_ptr< reto::ClippingPlane >& rightPlane );

    TSizeFunction getParticleSize( ) const;

    void setParticleSize( const TSizeFunction& particleSize );

    const TColorVec& getGradient( ) const;

    void setGradient( const TColorVec& gradient );

    bool isParticleVisibility( ) const;

    void setParticleVisibility( bool particleVisibility );

    bool isClippingEnabled( ) const;

    void enableClipping( bool enabled );

    float getTime( ) const;

    void setTime( float time );

    void addTime( float time, float endTime );

    void uploadDrawUniforms( plab::UniformCache& cache ) const override;

  };

}

#endif //SYNCOPA_STATICPARTICLEMODEL_H
