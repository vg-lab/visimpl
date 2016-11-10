#include "ColorOperationModel.h"

namespace prefr
{

  template< class T>
  T add(const T& lhs, const T& rhs){return lhs + rhs;}

  template< class T>
  T sub(const T& lhs, const T& rhs){return lhs - rhs;}

  template< class T>
  T mul(const T& lhs, const T& rhs){return lhs * rhs;}

  template< class T>
  T div(const T& lhs, const T& rhs){return lhs / rhs;}

  ColorOperationModel::ColorOperationModel( float min, float max,
                                            ColorOperation colorOp)
  : Model( min, max )
  {
    setColorOperation(colorOp);
  }


  void ColorOperationModel::setColorOperation(ColorOperation colorOp)
  {
    _colorOperation = colorOp;

    switch ( _colorOperation )
    {
      case ColorOperation::ADDITION:
        colorop = add<glm::vec4>;
        break;
      case ColorOperation::SUBSTRACTION:
        colorop = sub<glm::vec4>;
        break;
      case ColorOperation::MULTIPLICATION:
        colorop = mul<glm::vec4>;
        break;
      case ColorOperation::DIVISION:
        colorop = div<glm::vec4>;
        break;
    }
  }
}
