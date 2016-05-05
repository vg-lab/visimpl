/*
 * @file	H5Network.cpp
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es> 
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *					Do not distribute without further notice.
 */

#include "H5Network.h"

H5Network::H5Network( void )
: _totalRecords( 0 )
{

}

H5Network::H5Network( const std::string& fileName_,
                      const std::string& pattern_ )
: _fileName( fileName_ )
, _pattern( pattern_ )
, _totalRecords( 0 )
{

}

H5Network::~H5Network( void )
{

}

void H5Network::Load( const std::string& fileName_,
                      const std::string& pattern_ )
{
  _fileName = fileName_;
  _pattern = pattern_;

  Load( );
}

void H5Network::Load( void )
{
  if( _fileName.empty( ))
  {
    std::cerr << "Error: file path cannot be empty." << std::endl;
    return;
  }

  if( _pattern.empty( ))
  {
    std::cout << "Warning: an empty pattern will load all available datasets." << std::endl;
  }

  // Check whether file referenced by the path is an Hdf5 format file or not.
  if( !H5::H5File::isHdf5( _fileName ))
  {
    std::cerr << "File " << _fileName << " is not a Hdf5 file..." << std::endl;
    return;
  }

  // Create file
  _file = H5::H5File( _fileName, H5F_ACC_RDONLY );

  // Get the number of outer objects.
  unsigned int outerObjects = _file.getNumObjs( );

  unsigned int records = 0;

  // For each objects...
  for( unsigned int i = 0; i < outerObjects; i++ )
  {
    // Get object type.
    H5G_obj_t ot = _file.getObjTypeByIdx( i );

    // Check if type is a group.
    if( ot != H5G_obj_t::H5G_GROUP )
      continue;

    // Get object name.
    std::string currentName = _file.getObjnameByIdx( i );

    // Check if group name contains the pattern word.
    if( currentName.find( _pattern ) == std::string::npos )
      continue;

    // Open the current group.
    H5::Group group = _file.openGroup( currentName );

    // Get children datasets number
    unsigned int innerDatasets = group.getNumObjs( );

    // If there is more than one object... Skip it.
    if( innerDatasets != 1 )
      continue;

    // Check child type.
    H5G_obj_t childType = group.getObjTypeByIdx( 0 );
    if( childType != H5G_obj_t::H5G_DATASET )
      continue;

    // Get dataset name.
    std::string dataSetName = group.getObjnameByIdx( 0 );

    // Open dataset.
    H5::DataSet dataset = group.openDataSet( dataSetName );

    hsize_t dims[2];
    int totalDims = dataset.getSpace().getSimpleExtentDims( dims );

    if( totalDims != 2)
      continue;

    std::cout << dims[ 1 ] << " -> "<< dims[ 0 ] << std::endl;
    if( dims[ 1 ] != 3 )
      continue;

    if( dims[ 0 ] == 0 )
      continue;

    if( dataset.getDataType( ).getClass( ) != H5T_class_t::H5T_FLOAT )
      continue;

    // Assume dataset is correct so far...

    _offsets.push_back( records );

    records += dims[ 0 ];

    // Store group name.
    _groupNames.push_back( currentName );
    // Store current group.
    _groups.push_back( group );
    // Store current dataset.
    _datasets.push_back( dataset );

  }

  _totalRecords = records;

}

void H5Network::Clear( void )
{

}

visimpl::TGIDSet H5Network::GetGIDs( void ) const
{
  visimpl::TGIDSet result;

  for( unsigned int i = 0; i < _totalRecords; i++ )
  {
    result.insert( i );
  }

  return result;
}

visimpl::TPosVect H5Network::GetComposedPositions( void ) const
{
  visimpl::TPosVect result;

  if( _totalRecords == 0 )
    return result;

  result.reserve( _totalRecords );

  unsigned int currentOffset = 0;
  for( auto dataset : _datasets )
  {
    hsize_t dims[2];
    dataset.getSpace().getSimpleExtentDims( dims );

    std::vector< vmml::Vector3f > subset( dims[ 0 ] );
    dataset.read( subset.data( ), H5::PredType::IEEE_F32LE );

    result.insert( result.end( ), subset.begin( ), subset.end( ));

    currentOffset += dims[ 0 ];
  }

  return result;
}

std::string H5Network::fileName( void ) const
{
  return _fileName;
}

std::string H5Network::pattern( void ) const
{
  return _pattern;
}

unsigned int H5Network::ComposeID( unsigned int datasetIdx,
                                   unsigned int localIdx ) const
{
  return _offsets[ datasetIdx ] + localIdx;
}

unsigned int H5Network::subSetsNumber( void ) const
{
  return _datasets.size( );
}

const std::vector< unsigned int >& H5Network::offsets( void ) const
{
  return _offsets;
}

