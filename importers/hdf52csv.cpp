/*
 * hdf52csv.cpp
 *
 *  Created on: Jan 31, 2023
 *      Author: felix
 */

#include <H5Cpp.h>

#include <cstring>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cassert>
#include <memory>
#include <sys/stat.h>
#include <limits.h>

struct dotSeparator: std::numpunct<char>
{
    char do_decimal_point() const { return '.'; }
};

template<class T> void ignore( const T& ) { }

void printUsage(const char *filename)
{
  const std::string name = (filename ? std::string(filename) : "HDF52CSV");

  std::cout << name << " - Generates CSV network and connections file from HDF5 data." << '\n';
  std::cout << "Usage: " << name << " <hdf5_source_file>" << std::endl;
  std::exit(-1);
}

const char NETWORK_FILENAME[] = "H5_network.csv";
const char CONNECTIONS_FILENAME[] = "H5_connections.csv";

const char CELLS_TAG[]="cells/positions";
const char MAPS_TAG[]="cells/type_maps";
const char CONNECTIONS_TAG[]="cells/connections";
const char MORPHOLOGIES_TAG[] ="morphologies";

const char SEPARATOR = ',';
const char NEWLINE = '\n';

bool countConnections = true;

std::vector<std::string> CELL_TYPES = { "granule_cell", "glomerulus", "golgi_cell", "purkinje_cell",
                                        "stellate_cell", "basket_cell", "mossy_fibers" };

// Substitute with an include to filtered ids.
std::vector<uint32_t> toFilter;

int main(int argc, char* argv[])
{
  if(argc < 2) printUsage(argv[0]);

  struct stat buf;
  if(stat(argv[1], &buf) != 0)
  {
    std::cerr << "Unable to find file: " << argv[1] << std::endl;
    exit(-1);
  }

  if(!S_ISREG(buf.st_mode))
  {
    std::cerr << "Parameter " << argv[1] << " is not a file." << std::endl;
    exit(-1);
  }

  // Check whether file referenced by the path is an Hdf5 format file or not.
  if( !H5::H5File::isHdf5( argv[1] ))
  {
    std::cerr << "File " << argv[1] << " is not a HDF5 file!" << std::endl;
    std::exit(-1);
  }

  // open file
  auto sourceFile = H5::H5File( argv[1], H5F_ACC_RDONLY );

  for(const auto &tag: { CELLS_TAG, CONNECTIONS_TAG })
  {
    if(!H5Lexists(sourceFile.getLocId(), tag, H5P_DEFAULT) > 0)
    {
      std::cerr << "File " << argv[1] << " has not the correct format, "
                << tag << " tag is missing!" << std::endl;
      std::exit(-1);
    }
  }

  std::ofstream nFile, cFile;
  nFile.open(NETWORK_FILENAME, std::fstream::out|std::fstream::trunc);

  if(nFile.fail())
  {
    std::cerr << "Unable to open output filename: " << NETWORK_FILENAME << std::endl;
    std::exit(-1);
  }

  // write floating point numbers with dot separation no matter the locale
  nFile.imbue(std::locale(nFile.getloc(), new dotSeparator()));
  nFile << "gid" << SEPARATOR << "type" << SEPARATOR << "x" << SEPARATOR << "y" << SEPARATOR << "z" << NEWLINE;
  auto dataSet = sourceFile.openDataSet(CELLS_TAG);

  hsize_t dims[2];
  dataSet.getSpace().getSimpleExtentDims( dims );
  assert(dataSet.getSpace().getSimpleExtentNdims() == 2);
  assert(dataSet.getTypeClass() == H5T_FLOAT);

  auto buffer = new float[dims[0]*dims[1]];

  // load positions
  dataSet.read( reinterpret_cast<void*>(buffer), H5::PredType::IEEE_F32LE);

  unsigned long idCount = 0;
  for(size_t idx = 0; idx < dims[0]*dims[1]; )
  {
    const auto id = static_cast<uint32_t>(buffer[idx]);
    const auto type = static_cast<uint32_t>(buffer[idx+1]);
    bool hasId = toFilter.empty();
    if(!hasId)
    {
      hasId = std::find(toFilter.cbegin(), toFilter.cend(), id) != toFilter.cend();
    }

    if(hasId)
    {
      ++idCount;
      nFile << id << SEPARATOR << CELL_TYPES[type] << SEPARATOR;
      idx += dims[1]-3;
      nFile << buffer[idx+0] << SEPARATOR << buffer[idx+1] << SEPARATOR << buffer[idx+2] << NEWLINE;
    }
    else idx += dims[1]-3;
    idx+=3;
  }

  delete [] buffer;
  buffer = nullptr;
  dataSet.close();

  std::cout << "Write network file: " << NETWORK_FILENAME << " - " << idCount << " ids" << std::endl;

  nFile.flush();
  nFile.close();

  cFile.open(CONNECTIONS_FILENAME, std::fstream::out|std::fstream::trunc);

  if(cFile.fail())
  {
    std::cerr << "Unable to open output filename: " << CONNECTIONS_FILENAME << std::endl;
    std::exit(-1);
  }

  // write floating point numbers with dot separation no matter the locale
  cFile.imbue(std::locale(cFile.getloc(), new dotSeparator()));

  if(countConnections)
    cFile << "source" << SEPARATOR << "target" << SEPARATOR << "count" << SEPARATOR << "type" << NEWLINE;
  else
    cFile << "source" << SEPARATOR << "target" << SEPARATOR << "type" << NEWLINE;

  auto group = sourceFile.openGroup(CONNECTIONS_TAG);
  const unsigned int objNum = group.getNumObjs( );

  idCount = 0;
  for( unsigned int i = 0; i < objNum; i++ )
  {
    // Get object type.
    H5G_obj_t type = group.getObjTypeByIdx( i );

    // Get object name.
    const std::string name = group.getObjnameByIdx( i );

    // Check if type is a dataset.
    if( type != H5G_obj_t::H5G_DATASET )
      continue;

    // Open dataset.
    H5::DataSet innerDs = group.openDataSet( name );
    memset(dims, 0, 2*sizeof(hsize_t));
    innerDs.getSpace().getSimpleExtentDims( dims );
    assert(innerDs.getSpace().getSimpleExtentNdims() == 2);
    assert(innerDs.getTypeClass() == H5T_FLOAT);

    if(dims[0] == 0 || dims[1] == 0)
      continue;

    buffer = new float[dims[0]*dims[1]];

    innerDs.read( reinterpret_cast<void*>(buffer), H5::PredType::IEEE_F32LE );

    if(countConnections)
    {
      uint32_t aid1 = std::numeric_limits<uint32_t>::max();
      uint32_t aid2 = std::numeric_limits<uint32_t>::max();
      unsigned long cCount = 0;

      for(size_t idx = 0; idx < dims[0]*dims[1]; )
      {
        auto id1 = static_cast<uint32_t>(buffer[idx]);
        auto id2 = static_cast<uint32_t>(buffer[idx+1]);

        if(id1 == aid1 && id2 == aid2)
        {
          ++cCount;
          ++idCount;
          idx += dims[1];
          continue;
        }
        else
        {
          if(aid1 != std::numeric_limits<uint32_t>::max() && (aid1 != id1 || aid2 != id2))
          {
            cFile << aid1 << SEPARATOR << aid2 << SEPARATOR << cCount << SEPARATOR << name << NEWLINE;
            aid1 = std::numeric_limits<uint32_t>::max();
            aid2 = std::numeric_limits<uint32_t>::max();
            cCount = 0;
          }
        }

        bool hasIds = toFilter.empty();
        if(!hasIds)
        {
          const auto hasId1 = std::find(toFilter.cbegin(), toFilter.cend(), id1) != toFilter.cend();
          const auto hasId2 = std::find(toFilter.cbegin(), toFilter.cend(), id2) != toFilter.cend();
          hasIds = (hasId1 && hasId2);
        }

        if(hasIds)
        {
          ++idCount;
          ++cCount;
          aid1 = id1;
          aid2 = id2;
        }

        idx += dims[1];
      }

      if(aid1 != std::numeric_limits<uint32_t>::max())
        cFile << aid1 << SEPARATOR << aid2 << SEPARATOR << cCount << SEPARATOR << name << NEWLINE;
    }
    else
    {
      for(size_t idx = 0; idx < dims[0]*dims[1]; )
      {
        auto id1 = static_cast<uint32_t>(buffer[idx]);
        auto id2 = static_cast<uint32_t>(buffer[idx+1]);

        bool hasIds = toFilter.empty();
        if(!hasIds)
        {
          const auto hasId1 = std::find(toFilter.cbegin(), toFilter.cend(), id1) != toFilter.cend();
          const auto hasId2 = std::find(toFilter.cbegin(), toFilter.cend(), id2) != toFilter.cend();
          hasIds = (hasId1 && hasId2);
        }

        if(hasIds)
        {
          ++idCount;
          cFile << id1 << SEPARATOR << id2 << SEPARATOR << name << NEWLINE;
        }

        idx += dims[1];
      }
    }
    delete [] buffer;
    innerDs.close();
  }

  std::cout << "Write connections file: " << CONNECTIONS_FILENAME << " - " << idCount << " connections" << std::endl;

  cFile.flush();
  cFile.close();

  sourceFile.close();

  return 0;
}
