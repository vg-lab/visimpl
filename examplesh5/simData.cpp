/*
 * @file	simData.cpp
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *					Do not distribute without further notice.
 */

#include <sumrice/sumrice.h>


int main( int argc, char** argv )
{

  if( argc < 2 )
  {
    std::cerr << "Error: a file must be provided as a parameter." << std::endl;
    return  1 ;
  }

  std::string path = argv[ 1 ];

  std::string simtype;

  if( argc == 3)
    simtype = argv[ 2 ];

  visimpl::TDataType dataType( visimpl::TDataType::THDF5 );

  if( simtype == "-bc")
    dataType = visimpl::TDataType::TBlueConfig;

  visimpl::SimulationData simData( path, dataType );

  visimpl::TGIDSet gids = simData.gids( );

  std::cout << "Loaded GIDS: " << gids.size( ) << std::endl;

  visimpl::TPosVect positions = simData.positions( );

  std::cout << "Loaded positions: " << positions.size( ) << std::endl;

  return 0;
}

