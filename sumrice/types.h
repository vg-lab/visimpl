/*
 * @file	types.h
 * @brief
 * @author Pablo Toharia Rabasco  <pablo.toharia@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *					Do not distribute without further notice.
 */

#ifndef __VISIMPL_TYPES_H__
#define __VISIMPL_TYPES_H__

#include <vector>
#include <set>
#include <map>
#include <unordered_map>
#include <unordered_set>

#include <QColor>

#include <prefr/prefr.h>
#include <vmmlib/vmmlib.h>

namespace visimpl
{
  typedef std::set< uint32_t > TGIDSet;
  typedef std::vector< vmml::Vector3f > TPosVect;
  typedef std::multimap< float, uint32_t > TSpikes;

  typedef std::unordered_set< uint32_t > GIDUSet;
  typedef utils::InterpolationSet< glm::vec4 > TColorMapper;

  typedef enum
  {
    T_STACK_FIXED = 0,
    T_STACK_EXPANDABLE

  } TStackType;

  struct TimeFrame
  {
    std::string name;
    QColor color;

    std::vector< std::pair< float, float >> percentages;
//    float startPercentage;
//    float endPercentage;
  };

  typedef enum
  {
    TSimNetwork = 0,
    TSimSpikes,
    TSimVoltages
  } TSimulationType;

  typedef enum
  {
    TBlueConfig = 0,
    THDF5
  } TDataType;

  typedef enum
  {
    T_COLOR_LINEAR = 0,
//    T_COLOR_EXPONENTIAL,
    T_COLOR_LOGARITHMIC,
    T_COLOR_UNDEFINED
  } TColorScale;

  typedef enum
  {
    T_NORM_GLOBAL = 0,
    T_NORM_MAX
  } TNormalize_Rule;

  typedef enum
  {
    T_REP_DENSE = 0,
    T_REP_CURVE

  } TRepresentation_Mode;
}



#endif /* __VISIMPL_TYPES_H__ */
