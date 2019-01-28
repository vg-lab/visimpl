/*
 * @file  types.h
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */
#ifndef VISIMPL_TYPES_H_
#define VISIMPL_TYPES_H_

#include <sumrice/sumrice.h>

namespace visimpl
{

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
}



#endif /* VISIMPL_TYPES_H_ */
