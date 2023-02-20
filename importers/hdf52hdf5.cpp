/*
 * hdf52hdf5.cpp
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

// should get this from file, not store it here. Use an include
const std::vector<std::string> CELL_TYPES = { "granule_cell", "glomerulus", "golgi_cell", "purkinje_cell",
                                              "stellate_cell", "basket_cell", "mossy_fibers" };
void printUsage(const char *filename)
{
  const std::string name = (filename ? std::string(filename) : "TXTTOCSV");

  std::cout << name << " - Generates HDF5 with filtered ids." << '\n';
  std::cout << "Usage: " << name << " <hdf5_source_file> <hdf5_target_file>" << std::endl;
  std::exit(-1);
}

// #include "toFilter.h"
// modify to filter ids, maybe use another input for this? If empty all ids goes in target.
std::vector<uint32_t> toFilter;

const char CELLS_TAG[]="cells/positions";
const char MAPS_TAG[]="cells/type_maps";
const char CONNECTIONS_TAG[]="cells/connections";
const char MORPHOLOGIES_TAG[] ="morphologies";

// to store cell ids, type and coordinates. Has to be double to write to HDF5 dataset.
struct Coords
{
    double id;
    double type;
    double x;
    double y;
    double z;

    Coords():id{0}, type{0}, x{0}, y{0}, z{0} {};
    Coords(double i, double t, double _x, double _y, double _z): id{i}, type{t}, x{_x}, y{_y}, z{_z} {};
};

auto checkError = [](const int value) { if(value < 0) { std::cerr << "ERROR: " << __FILE__ << ":" << __LINE__ << std::endl; exit(1); } };

void H5WritePositions(hid_t to, const std::vector<Coords> &coords)
{
  if(coords.empty()) return;

  hsize_t dims[2]{coords.size(), 5};

  // Create a dataset
  auto group_id = H5Gopen2(to, "/cells", H5P_DEFAULT);
  auto dspace_id = H5Screate_simple(2, dims, NULL);
  auto dset_id = H5Dcreate2(group_id, "positions", H5T_NATIVE_DOUBLE, dspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

  // Write the data
  auto status = H5Dwrite(dset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, coords.data());
  checkError(status);

  // Close dataset after writing
  status = H5Dclose(dset_id);
  checkError(status);

  status = H5Gclose(group_id);
  checkError(status);
}

void H5CopyAttribute(hid_t from, hid_t to, const char* propertyName)
{
  hid_t attr = H5Aopen(from, propertyName, H5P_DEFAULT);
  auto aSpace = H5Aget_space(attr);

  H5A_info_t aInfo;
  auto status = H5Aget_info(attr, &aInfo);
  checkError(status);
  auto aType = H5Aget_type(attr);

  unsigned char *buffer = new unsigned char[aInfo.data_size];
  memset(buffer, 0, aInfo.data_size);
  status = H5Aread(attr, aType, buffer);
  checkError(status);

  auto newAttr = H5Acreate2(to, propertyName, aType, aSpace,H5P_DEFAULT, H5P_DEFAULT);
  checkError(newAttr);
  status = H5Awrite(newAttr, aType, buffer);
  checkError(status);

  H5free_memory(buffer);
  H5Sclose(aSpace);
  H5Tclose(aType);
  H5Aclose(attr);
  H5Aclose(newAttr);
}

void H5copy(hid_t from, const char *from_tag, hid_t to, const char *to_tag)
{
  auto status = H5Ocopy(from, from_tag, to, to_tag, H5P_DEFAULT, H5P_DEFAULT);
  checkError(status);
}

void H5writeMap(hid_t to, std::string name, std::vector<uint32_t> &gids)
{
  hsize_t dims[1]{gids.size()};

  auto group_id = H5Gopen2(to, "/cells/type_maps", H5P_DEFAULT);

  // Create a dataset
  auto dspace_id = H5Screate_simple(1, dims, NULL);
  auto dset_id = H5Dcreate2( group_id, name.c_str(), H5T_NATIVE_INT, dspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

  // Write the data
  auto status = H5Dwrite(dset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, gids.data());
  checkError(status);

  // Close dataset after writing
  status = H5Dclose(dset_id);
  checkError(status);

  status = H5Gclose(group_id);
  checkError(status);
}

void H5writeConnections(hid_t to, std::string name, std::vector<std::pair<uint32_t, uint32_t>> connections)
{
  std::vector<std::pair<double, double>> others;
  for(size_t i = 0; i < connections.size(); ++i)
  {
    auto value = connections.at(i);

    if(!toFilter.empty())
    {
      auto it1 = std::find(toFilter.cbegin(), toFilter.cend(), value.first);
      auto it2 = std::find(toFilter.cbegin(), toFilter.cend(), value.second);
      if(toFilter.cend() == it1 || toFilter.cend() == it2)
      {
        continue;
      }
    }

    others.emplace_back(value.first, value.second);
  }

  if(others.empty()) return;

  hsize_t dims[2]{others.size(), 2};

  auto group_id = H5Gopen2(to, "/cells/connections", H5P_DEFAULT);
  // Create a dataset
  auto dspace_id = H5Screate_simple(2, dims, NULL);
  auto dset_id = H5Dcreate2( group_id, name.c_str(), H5T_NATIVE_DOUBLE, dspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

  // Write the data
  auto status = H5Dwrite(dset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, others.data());
  checkError(status);

  // Close dataset after writing
  status = H5Dclose(dset_id);
  checkError(status);

  status = H5Gclose(group_id);
  checkError(status);
}

int main(int argc, char* argv[])
{
  if(argc < 3) printUsage(argv[0]);

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

  if(stat(argv[2], &buf) == 0)
  {
    std::cerr << "Destination file: " << argv[2] << " already exists." << std::endl;
    exit(-1);
  }

  // Check whether file referenced by the path is an Hdf5 format file or not.
  if( !H5::H5File::isHdf5( argv[1] ))
  {
    std::cerr << "File " << argv[1] << " is not a HDF5 file!" << std::endl;
    return 0;
  }

  // open files
  auto sourceFile = H5::H5File( argv[1], H5F_ACC_RDONLY );
  auto targetFile = H5Fcreate( argv[2], H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

  for(const auto &tag: { CELLS_TAG, CONNECTIONS_TAG, MORPHOLOGIES_TAG, MAPS_TAG })
  {
    if(!H5Lexists(sourceFile.getLocId(), tag, H5P_DEFAULT) > 0)
    {
      std::cerr << "File " << argv[1] << " has not the correct format, "
                << tag << " tag is missing or misplaced!" << std::endl;
      return -1;
    }
  }

  // copy morphologies
  H5copy(sourceFile.getLocId(), MORPHOLOGIES_TAG, targetFile, MORPHOLOGIES_TAG);

  // open cells and copy/filter
  H5Gcreate2(targetFile, "/cells", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  auto targetCellId = H5Gopen2(targetFile, "/cells", H5P_DEFAULT);
  auto sourceCellId = H5Gopen2(sourceFile.getLocId(), "cells", H5P_DEFAULT);
  H5CopyAttribute(sourceCellId, targetCellId, "types");
  H5Gcreate2(targetCellId, "type_maps", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  H5Gcreate2(targetCellId, "connections", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  H5Gclose(targetCellId);

  auto dataSet = sourceFile.openDataSet(CELLS_TAG);

  hsize_t dims[2];
  dataSet.getSpace().getSimpleExtentDims( dims );
  assert(dataSet.getSpace().getSimpleExtentNdims() == 2);
  assert(dataSet.getTypeClass() == H5T_FLOAT);

  auto buffer = new float[dims[0]*dims[1]];

  // load positions
  dataSet.read( reinterpret_cast<void*>(buffer), H5::PredType::IEEE_F32LE );

  std::vector<Coords> subset;
  std::vector<uint32_t> types;
  for(size_t idx = 0; idx < dims[0]*dims[1]; )
  {
    const auto id = static_cast<uint32_t>(buffer[idx]);
    bool hasId = toFilter.empty();
    if(!hasId)
    {
      auto it = std::find(toFilter.cbegin(), toFilter.cend(), id);
      hasId = it != toFilter.cend();
    }

    if(hasId)
    {
      const auto type = static_cast<uint32_t>(buffer[idx+1]);
      idx += dims[1]-3;
      subset.emplace_back(id, type, buffer[idx+0],buffer[idx+1],buffer[idx+2]);
      idx+=3;
    }
  }

  delete [] buffer;
  buffer = nullptr;

  dataSet.close();

  auto sortCoords = [](Coords &lhs, Coords &rhs){ return lhs.id < rhs.id; };
  std::sort(subset.begin(), subset.end(), sortCoords);

  H5WritePositions(targetFile, subset);

  // group maps.
  if(H5Lexists(sourceFile.getLocId(), MAPS_TAG, H5P_DEFAULT) > 0)
  {
    auto group = sourceFile.openGroup(MAPS_TAG);
    const unsigned int objNum = group.getNumObjs( );

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
      assert(innerDs.getSpace().getSimpleExtentNdims() == 1);

      assert(innerDs.getTypeClass() == H5T_INTEGER);
      auto integerType = innerDs.getIntType();

      if(dims[0] == 0)
        continue;

      buffer = new float[dims[0]];

      innerDs.read( reinterpret_cast<void*>(buffer), H5::PredType::IEEE_F32LE );

      std::vector<uint32_t> gids;
      for(size_t idx = 0; idx < dims[0]; )
      {
        const auto id = static_cast<uint32_t>(buffer[idx]);
        bool hasId = toFilter.empty();
        if(!hasId)
          hasId = std::find(toFilter.cbegin(), toFilter.cend(), id) != toFilter.cend();

        if(hasId)
          gids.emplace_back(id);

        ++idx;
      }

      delete [] buffer;
      buffer = nullptr;
      const auto groupName = "group " + name;
      innerDs.close();

      H5writeMap(targetFile, name, gids);
    }

    group.close();
  }

  // groups by connections
  if(H5Lexists(sourceFile.getLocId(), CONNECTIONS_TAG, H5P_DEFAULT) > 0)
  {
    auto group = sourceFile.openGroup(CONNECTIONS_TAG);
    const unsigned int objNum = group.getNumObjs( );

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

      std::vector<std::pair<uint32_t, uint32_t>> connections;

      std::vector<uint32_t> gids;
      for(size_t idx = 0; idx < dims[0]*dims[1]; )
      {
        auto sou = static_cast<uint32_t>(buffer[idx]);
        auto tar = static_cast<uint32_t>(buffer[idx+1]);
        bool hasIds = toFilter.empty();
        if(!hasIds)
        {
          auto it1 = std::find(toFilter.cbegin(), toFilter.cend(), sou);
          auto it2 = std::find(toFilter.cbegin(), toFilter.cend(), tar);

          hasIds = (it1 != toFilter.cend() && it2 != toFilter.cend());
        }

        if(hasIds)
          connections.emplace_back(sou, tar);

        idx += dims[1];
      }
      delete [] buffer;
      buffer = nullptr;
      const auto groupName = "connection " + name;
      innerDs.close();

      H5writeConnections(targetFile, name, connections);
    }

    group.close();
  }

  sourceFile.close();

  H5Fflush(targetFile, H5F_SCOPE_GLOBAL);
  H5Fclose(targetFile);

  return 0;
}

