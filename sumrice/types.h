/*
 * @file	types.h
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es> 
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *					Do not distribute without further notice.
 */

#ifndef __VISIMPL_TYPES_H__
#define __VISIMPL_TYPES_H__

#include <vector>
#include <set>
#include <map>

#include <vmmlib/vmmlib.h>

namespace visimpl
{
  typedef std::set< uint32_t > TGIDSet;
  typedef std::vector< vmml::Vector3f > TPosVect;
  typedef std::multimap< float, uint32_t > TSpikesMap;

  typedef enum
  {
    TUndefined = 0,
    TSpikes,
    TVoltages
  } TSimulationType;

  typedef enum
  {
    TBlueConfig = 0,
    THDF5
  } TDataType;
}



#endif /* __VISIMPL_TYPES_H__ */
