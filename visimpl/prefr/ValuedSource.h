#ifndef __VISIMPL__VALUEDSOURCE__
#define __VISIMPL__VALUEDSOURCE__

#include <prefr/prefr.h>

namespace prefr
{

  class ValuedSource : public Source
  {
  public:


    ValuedSource( float emissionRate, const glm::vec3& position,
                  const glm::vec4& color, bool still = false );

    virtual ~ValuedSource();

    virtual void color( const glm::vec4& color);
    virtual const glm::vec4& color();

    virtual void still( bool still );
    virtual bool still();

    virtual void size( float size );
    virtual float size();

    virtual void particlesLife( float life );
    virtual float particlesLife( void );

  protected:

    glm::vec4 _color;
    float _size;
    bool _still;

    float _particlesLife;

  };

}

#endif /* __VISIMPL__VALUEDSOURCE__ */
