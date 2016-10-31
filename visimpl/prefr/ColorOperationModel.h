#ifndef __VISIMPL__COLOROPERATIONMODEL__
#define __VISIMPL__COLOROPERATIONMODEL__

#include <prefr/prefr.h>

namespace prefr
{

  enum ColorOperation
  {
    ADDITION = 0,
    SUBSTRACTION,
    MULTIPLICATION,
    DIVISION
  };

  class ColorOperationModel : public Model
  {
  public:

    glm::vec4 (*colorop)(const glm::vec4& lhs, const glm::vec4& rhs);

    ColorOperationModel( float min, float max,
                         ColorOperation colorOp = ADDITION );

    void SetColorOperation( ColorOperation colorOp );

  protected:

    ColorOperation _colorOperation;

  };


}


#endif /* __VISIMPL__COLOROPERATIONMODEL__ */
