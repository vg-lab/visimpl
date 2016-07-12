#ifndef __VISIMPL__COLOREMISSIONNODE__
#define __VISIMPL__COLOREMISSIONNODE__

#include <prefr/prefr.h>

namespace prefr
{

  class ColorSource : public PointSource
  {
  public:


    ColorSource( float emissionRate, const glm::vec3& _position,
                       const glm::vec4& color, bool still = false);
    virtual ~ColorSource();

    virtual void color( const glm::vec4& color );
    virtual const glm::vec4& color( void );

    virtual void still( bool still );
    virtual bool still( void );

    virtual void size( float size );
    virtual float size( void);

    virtual bool Emits() const;

  protected:

    glm::vec4 _color;
    float _size;
    bool _still;


  };

}

#endif /* __VISIMPL__COLOREMISSIONNODE__ */
