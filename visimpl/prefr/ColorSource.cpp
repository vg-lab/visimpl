#include "ColorSource.h"

namespace prefr
{

  ColorSource::ColorSource( float emissionRate,
                                        const glm::vec3& position_,
                                        const glm::vec4& color_,
                                        bool still_ )
  : PointSource( emissionRate, position_ )
  , _color( color_ )
  , _size ( 0.0f )
  , _still( still_ )
  {}

  ColorSource::~ColorSource()
  {}

  void ColorSource::color( const glm::vec4& color_ )
  {
    _color = color_;
//    color_.a = 0.0f;
  }

  const glm::vec4& ColorSource::color( void )
  {
    return _color;
  }

  void ColorSource::still( bool still_ )
  {
    _still = still_;
  }

  bool ColorSource::still( void )
  {
    return _still;
  }


  void ColorSource::size( float size_ )
  {
    _size = size_;
  }

  float ColorSource::size( void )
  {
    return _size;
  }

  bool ColorSource::Emits( void ) const
  {
    return true;
  }

}


