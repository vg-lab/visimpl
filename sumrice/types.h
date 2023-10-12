/*
 * Copyright (c) 2015-2020 VG-Lab/URJC.
 *
 * Authors: Pablo Toharia Rabasco  <pablo.toharia@urjc.es>
 *
 * This file is part of ViSimpl <https://github.com/vg-lab/visimpl>
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

#ifndef __VISIMPL_TYPES_H__
#define __VISIMPL_TYPES_H__

#include <vector>
#include <set>
#include <map>
#include <unordered_map>
#include <unordered_set>

#include <QColor>
#include <QPainterPath>

#include <vmmlib/vmmlib.h>

#include <simil/simil.h>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

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

  typedef std::pair< float, QColor > TTFColor;
  typedef std::vector< TTFColor > TTransferFunction;

  typedef std::pair< float, float > TSize;
  typedef std::vector< TSize > TSizeFunction;

  typedef glm::vec3 vec3;
  typedef glm::vec4 vec4;

  typedef std::vector< std::pair< float, glm::vec4 >> TColorVec;

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
    { return std::abs(result - other.result) < std::numeric_limits<double>::epsilon(); }

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
