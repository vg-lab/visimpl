/*
 * SpineRetParticlePrototype.cpp
 *
 *  Created on: 05/12/2014
 *      Author: sergio
 */

#include "ColorOperationPrototype.h"

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

  ColorOperationPrototype::
  ColorOperationPrototype(float min, float max,
                            const ParticleCollection& particlesArray,
                            ColorOperation colorOp)
  :ParticlePrototype(min, max, particlesArray)
  {
    SetColorOperation(colorOp);
  }


  void ColorOperationPrototype::SetColorOperation(ColorOperation colorOp)
  {
    colorOperation = colorOp;

    switch ( colorOperation )
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
