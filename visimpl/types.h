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

#ifndef VISIMPL_TYPES_H_
#define VISIMPL_TYPES_H_

#include <sumrice/sumrice.h>
#include <reto/reto.h>

namespace visimpl
{

  typedef Eigen::Vector3f evec3;
  typedef Eigen::Vector4f evec4;
  typedef Eigen::Matrix4f emat4;

  static inline evec3 vec4ToVec3( evec4 vec )
  {
    return evec3( vec.x( ), vec.y( ), vec.z( ));
  }

  static inline evec4 vec3ToVec4( evec3 vec )
  {
    return evec4( vec.x( ), vec.y( ), vec.z( ), 1.0);
  }

  enum tShaderParticlesType
  {
    T_SHADER_DEFAULT = 0,
    T_SHADER_SOLID,
    T_SHADER_UNDEFINED
  };

  typedef std::pair< vec3, vec3 > tBoundingBox;

  typedef std::unordered_map< unsigned int, vec3 > tGidPosMap;
  typedef std::unordered_map< unsigned int, unsigned int > tUintUMap;
  typedef std::unordered_multimap< unsigned int, unsigned int > tUintUMultimap;
  typedef std::vector< std::pair< unsigned int, unsigned int >> tUintPairs;

  enum tNeuronAttributes
  {
    T_TYPE_MORPHO = 0,
    T_TYPE_FUNCTION,
    T_TYPE_UNDEFINED
  };

  typedef std::tuple< unsigned int,
                      unsigned int > NeuronAttributes;

  typedef std::unordered_map< unsigned int, NeuronAttributes > tNeuronAttribs;

  enum tInitialConfig
  {
    T_DELTATIME = 0,
    T_STEPS_PER_SEC,
    T_DECAY,
    T_SCALE
  };

  typedef std::tuple< float, float, float, float > InitialConfig;

  typedef std::tuple< unsigned int,
                      std::string,
                      std::string,
                      unsigned int > tStatsGroup;

  enum tTypeAttributes
  {
    T_TYPE_VALUE = 0,
    T_TYPE_NAME,
    T_TYPE_LABEL,
    T_TYPE_STATS
  };

  typedef std::vector< std::string > Strings;
  typedef std::vector< tStatsGroup > tAppStats;

  typedef std::tuple< unsigned int,
                      unsigned int,
                      vec3,
                      QPoint,
                      bool
                      > tParticleInfo;

  enum tParticleInfoAttribs
  {
    T_PART_GID = 0,
    T_PART_INTERNAL_GID,
    T_PART_POSITION,
    T_PART_SCREEN_POS,
    T_PART_VALID,
    T_PART_UNDEFINED
  };

  static inline std::string vecToStr( const glm::vec3& vec )
  {
    return std::string( "(" + std::to_string( vec.x) +
                        ", " + std::to_string( vec.y) +
                        ", " + std::to_string( vec.z) + ")");
  }

  static inline std::string vecToStr( const Eigen::Vector3f& vec )
  {
    return std::string( "(" + std::to_string( vec.x( )) +
                        ", " + std::to_string( vec.y( )) +
                        ", " + std::to_string( vec.z( )) + ")");
  }

  static inline Eigen::Vector3f glmToEigen( const glm::vec3& vect )
  {
    return Eigen::Vector3f( vect.x, vect.y, vect.z );
  }

  static inline Eigen::Vector4f glmToEigen( const glm::vec4& vect )
  {
    return Eigen::Vector4f( vect.x, vect.y, vect.z, vect.w );
  }

  static inline glm::vec3 eigenToGLM( const Eigen::Vector3f& vect )
  {
    return glm::vec3( vect.x( ), vect.y( ), vect.z( ) );
  }

  static inline glm::vec4 eigenToGLM( const Eigen::Vector4f& vect )
  {
    return glm::vec4( vect.x( ), vect.y( ), vect.z( ), vect.w( ) );
  }

  static float invRGB = 1.0f / 255;

  static inline evec3 colorQtToEigen( const QColor& color_ )
  {
    return evec3( std::min( 1.0f, std::max( 0.0f, color_.red( ) * invRGB )),
                  std::min( 1.0f, std::max( 0.0f, color_.green( ) * invRGB )),
                  std::min( 1.0f, std::max( 0.0f, color_.blue( ) * invRGB )));
  }

  static inline QColor colorEigenToQt( const evec3& color_ )
  {
    return QColor( std::min( 255, std::max( 0, int( color_.x( ) * 255 ))),
                   std::min( 255, std::max( 0, int( color_.y( ) * 255 ))),
                   std::min( 255, std::max( 0, int( color_.z( ) * 255 ))));
  }

  static inline glm::vec3 floatPtrToVec3( float* floatPos )
  {
    return glm::vec3( floatPos[ 0 ],
                      floatPos[ 1 ],
                      floatPos[ 2 ]);
  }

  static inline glm::mat4x4 floatPtrToMat4( float* floatPos )
  {
    return glm::mat4x4( floatPos[ 0 ], floatPos[ 1 ],
                        floatPos[ 2 ], floatPos[ 3 ],
                        floatPos[ 4 ], floatPos[ 5 ],
                        floatPos[ 6 ], floatPos[ 7 ],
                        floatPos[ 8 ], floatPos[ 9 ],
                        floatPos[ 10 ], floatPos[ 11 ],
                        floatPos[ 12 ], floatPos[ 13 ],
                        floatPos[ 14 ], floatPos[ 15 ]);
  }

  class Camera : public prefr::ICamera, public reto::Camera
  {
  public:

    Camera( void ) : prefr::ICamera( ), reto::Camera( ){ }

#ifdef VISIMPL_USE_ZEROEQ
    Camera( std::string zeqUri_ ): prefr::ICamera( ), reto::Camera( zeqUri_ ){ }
#endif

    glm::vec3 PReFrCameraPosition( void )
    {
      auto viewM = viewMatrix( );
      return glm::vec3( viewM[ 3 ],
                        viewM[ 7 ],
                        viewM[ 11 ]);
    }

    glm::mat4x4 PReFrCameraViewMatrix( void )
    {
      return floatPtrToMat4( viewMatrix( ));
    }

    glm::mat4x4 PReFrCameraViewProjectionMatrix( void )
    {
      return floatPtrToMat4( projectionViewMatrix( ));
    }
  };
}



#endif /* VISIMPL_TYPES_H_ */
