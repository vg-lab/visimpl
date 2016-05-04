/*
 * @file	H5Activity.h
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *					Do not distribute without further notice.
 */

#ifndef __H5ACTIVITY_H__
#define __H5ACTIVITY_H__

#include "types.h"
#include "H5Network.h"

class H5Activity
{

public:

  H5Activity( const H5Network& network,
              const std::string& fileName_,
              const std::string& pattern_ );

  virtual ~H5Activity( void );

  virtual void Load( void ) = 0;

  std::string fileName( void ) const;
  std::string pattern( void ) const;

protected:

  std::string _fileName;
  std::string _pattern;

  const H5Network* _network;

  H5::H5File _file;
  std::vector< std::string > _groupNames;
  std::vector< H5::Group > _groups;

};

class H5Spikes : public H5Activity
{
public:

  H5Spikes( const H5Network& network,
            const std::string& fileName_,
            const std::string& pattern_ = "neuron" );

  ~H5Spikes( void );

  void Load( void );

  visimpl::TSpikes spikes( void );
  float startTime( void );
  float endTime( void );

protected:

  float _startTime;
  float _endTime;

  unsigned int _totalRecords;

  std::vector< H5::DataSet > _spikeTimes;
  std::vector< H5::DataSet > _spikeIds;

};



#endif /* __H5ACTIVITY_H__ */
