/*
 * @file	TestH5.cpp
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es> 
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *					Do not distribute without further notice.
 */

#include <string>
#include <iostream>

#include <vector>

#include <H5Cpp.h>


int main( int argc, char** argv )
{
  if( argc < 3)
    return 1;

  std::string filePath( argv[ 1 ]);

  if( !H5::H5File::isHdf5( filePath ))
  {
    std::cerr << "File " << filePath << " is not a Hdf5 file..." << std::endl;
    exit( 1 );
  }

  H5::H5File file( filePath, H5F_ACC_RDONLY );

  unsigned int totalObjects = file.getNumObjs( );

  std::vector< hid_t > ids( totalObjects, -1 );

  std::cout << "Detected " << totalObjects << " objects in file." << std::endl;

  file.getObjIDs( H5F_OBJ_ALL, totalObjects, ids.data( ) );

  std::vector< std::string > objNames( totalObjects );
  std::vector< H5::Group > groups;

  std::vector< H5::DataSet > datasets;

  for( unsigned int i = 0; i < totalObjects; i++ )
  {
    std::cout << "Reading object by idx " << i << std::endl;

    std::string currentName = file.getObjnameByIdx( i );

    if( currentName.find( std::string( "neuron" )) == std::string::npos )
      continue;

    H5G_obj_t ot = file.getObjTypeByIdx( i );
    std::cout << "Object type:  " << int( ot ) << std::endl;

    objNames[ i ] = currentName;
    std::cout << "Object name: '" << currentName << "'" << std::endl;

    switch( ot )
    {
      case H5G_GROUP:
      {
//        std::cout << "Reading group " << currentName << std::endl;
        H5::Group group = file.openGroup( currentName );
        groups.push_back( group );

        unsigned int numChildren = group.getNumObjs( );
        std::cout << "Found " << numChildren << " groups" << std::endl;

        for( unsigned int j = 0; j < numChildren; j++)
        {
          std::string name = group.getObjnameByIdx( j );
          H5G_obj_t childType = group.getObjTypeByIdx( j );

          std::cout << "\tChild name: " << name << std::endl;
          std::cout << "\tChild type: " << ( int )childType << std::endl;

          H5::DataSet dataset = group.openDataSet( name );
          datasets.push_back( dataset );

          int dimensions = dataset.getSpace( ).getSimpleExtentNdims( );
          std::cout << "\tDataset dimensions " << dimensions << std::endl;

          hsize_t dims[2];
          dataset.getSpace().getSimpleExtentDims( dims );

          std::cout << "\tDataset elements dim 0: " << dims[ 0 ] << std::endl;
          std::cout << "\tDataset elements dim 1: " << dims[ 1 ] << std::endl;
        }

        break;
      }
      default:
        std::cout << "Error: Incompatible object type:  " << ( int ) ot << std::endl;
        break;
    }

  }

  return 0;
}


