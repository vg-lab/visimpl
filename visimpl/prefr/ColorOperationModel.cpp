/*
 * Copyright (c) 2015-2020 GMRV/URJC.
 *
 * Authors: Sergio E. Galindo <sergio.galindo@urjc.es>
 *
 * This file is part of ViSimpl <https://github.com/gmrvvis/visimpl>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3.0 as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

// Visimpl
#include "ColorOperationModel.h"

// C++
#include <exception>

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
      default:
        {
          const auto message = std::string("Invalid ColorOperation value ") +
                               std::to_string(static_cast<int>(_colorOperation)) + " " +
                               std::string(__FILE__) + ":" +
                               std::to_string(__LINE__);
          throw std::out_of_range(message.c_str());
        }
        break;
    }
  }
}
