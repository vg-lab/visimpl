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

class H5Network
{
public:

  H5Network( void );
  H5Network( std::string fileName, std::string pattern = "neuron" );

  ~H5Network( void );

  void Load( void );
  void Load( std::string fileName, std::string pattern = "neuron" );

  void Clear( void );

  unsigned int subSetsNumber( void );

  visimpl::TGIDSet GetGIDs( void );
  visimpl::TPosVect GetComposedPositions( void );

  unsigned int ComposeID( unsigned int datasetIdx, unsigned int localIdx );

  std::string fileName( void );
  std::string pattern( void );

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
