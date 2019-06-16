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
#include <QPainterPath>

#include <prefr/prefr.h>
#include <vmmlib/vmmlib.h>

#include <simil/simil.h>

namespace visimpl
{
  typedef simil::Event Event;
  typedef simil::GIDVec GIDVec;
  typedef simil::EventVec EventVec;

  typedef simil::TGIDSet TGIDSet;
  typedef simil::TGIDUSet TGIDUSet;
  typedef simil::TPosVect TPosVect;

  typedef simil::Spike Spike;
  typedef simil::TSpikes TSpikes;

  typedef std::unordered_map< unsigned int, unsigned int > TUIntUintMap;

  typedef std::set< uint32_t > TGIDSet;
  typedef std::vector< vmml::Vector3f > TPosVect;

  typedef std::unordered_set< uint32_t > GIDUSet;
  typedef utils::InterpolationSet< glm::vec4 > TColorMapper;

  typedef std::pair< float, QColor > TTFColor;
  typedef std::vector< TTFColor > TTransferFunction;

  typedef std::pair< float, float > TSize;
  typedef std::vector< TSize > TSizeFunction;

  typedef glm::vec3 vec3;
  typedef glm::vec4 vec4;

  typedef enum
  {
    T_STACK_FIXED = 0,
    T_STACK_EXPANDABLE

  } TStackType;

  struct TEvent
  {
    std::string name;
    QColor color;
    bool visible;

    std::vector< std::pair< float, float >> percentages;

    std::vector< QPainterPath > _cachedCustomSolidRep;
    std::vector< QPainterPath > _cachedCustomTransRep;
    std::vector< QPainterPath > _cachedCustomHistoRep;
    std::vector< QPainterPath > _cachedCommonRep;
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

  struct CorrelationValues
  {
  public:

    double hit;
    double falseAlarm;
    double cr;
    double miss;

    double result;

    double entropy;
    double jointEntropy;
    double mutualInformation;

    bool operator==( const CorrelationValues& other ) const
    { return result == other.result; }

    bool operator>( const CorrelationValues& other ) const
    { return result > other.result; }
  };

  typedef std::unordered_map< uint32_t, CorrelationValues > TNeuronCorrelationUMap;
  typedef TNeuronCorrelationUMap::const_iterator TNeuronCorrelUMapCIt;
  typedef std::pair< TNeuronCorrelUMapCIt,
                     TNeuronCorrelUMapCIt > TNeuronCorrelationRange;

  struct Correlation
  {
  public:

    GIDUSet gids;

    std::string fullName;
    std::string subsetName;
    std::string eventName;

    TNeuronCorrelationUMap values;
  };

}



#endif /* __VISIMPL_TYPES_H__ */
