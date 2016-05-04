/*
 * @file	H5Network.h
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es> 
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *					Do not distribute without further notice.
 */

#ifndef __H5NETWORK_H__
#define __H5NETWORK_H__

#include <string>
#include <vector>

#include <H5Cpp.h>

#include "types.h"

class H5Activity;
class H5Spikes;

class H5Network
{
  friend class H5Activity;
  friend class H5Spikes;

public:

  H5Network( void );
  H5Network( const std::string& fileName ,
             const std::string& pattern = "neuron" );

  ~H5Network( void );

  void Load( void );
  void Load( const std::string& fileName ,
             const std::string& pattern = "neuron" );

  void Clear( void );

  unsigned int subSetsNumber( void ) const;

  visimpl::TGIDSet GetGIDs( void ) const;
  visimpl::TPosVect GetComposedPositions( void ) const;

  const std::vector< unsigned int >& offsets( void ) const;

  unsigned int ComposeID( unsigned int datasetIdx,
                          unsigned int localIdx ) const;

  std::string fileName( void ) const;
  std::string pattern( void ) const;

protected:

  std::string _fileName;
  std::string _pattern;

  unsigned int _totalRecords;

  H5::H5File _file;
  std::vector< std::string > _groupNames;
  std::vector< H5::Group > _groups;
  std::vector< H5::DataSet > _datasets;

  std::vector< unsigned int > _offsets;


};



#endif /* __H5NETWORK_H__ */
