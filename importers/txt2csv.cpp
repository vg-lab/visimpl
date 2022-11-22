/** TXT TO CSV
 * File: txttocsv.cpp
 * Author: Félix de las Pozas Álvarez
 * Date: 18/11/2022
 *
 */

// C++
#include <iostream>
#include <fstream>
#include <random>
#include <locale>
#include <sys/stat.h>
#include <dirent.h>
#include <algorithm>
#include <cassert>

// Qt5
#include <QString>
#include <QStringList>

std::string NETWORK_FILENAME = "H5_network.csv";
std::string ACTiVITY_FILENAME = "H5_activity.csv";

const double TIME_INTERVAL = 0.1;

struct dotSeparator: std::numpunct<char>
{
    char do_decimal_point() const { return '.'; }
};

template<class T> void ignore( const T& ) { }

void printUsage(const char *filename)
{
  const std::string name = (filename ? std::string(filename) : "TXTTOCSV");

  std::cout << name << '\n';
  std::cout << "Generates CSV network from HDF5 extracted txt coordinates." << '\n';
  std::cout << "Usage: " << name << " <txt_file>" << std::endl;
  std::exit(-1);
}

int main(int argc, char* argv[])
{
  if(argc < 2) printUsage(argv[0]);

  std::ifstream dataFile;
  dataFile.open(argv[1], std::fstream::in);

  if(dataFile.fail())
  {
    std::cerr << "ERROR: Unable to open input filename: " << argv[1] << std::endl;
    std::exit(-1);
  }

  std::ofstream nFile;
  nFile.open(NETWORK_FILENAME, std::fstream::out|std::fstream::trunc);

  if(nFile.fail())
  {
    std::cerr << "ERROR: Unable to open output filename: " << NETWORK_FILENAME << std::endl;
    std::exit(-1);
  }

  // write floating point numbers with dot separation no matter the locale
  nFile.imbue(std::locale(nFile.getloc(), new dotSeparator()));

  const char SEPARATOR = ',';

  unsigned int count = 0;
  unsigned int id = 0;
  unsigned int idsCount = 0;
  float coords[3]{0.,0.,0.};
  std::string content;
  while(dataFile.good() && std::getline(dataFile, content, SEPARATOR))
  {
    if(!content.empty())
    {
      switch(count)
      {
        case 0:
          id = std::stoi(content);
          break;
        case 1:
          break; // discard
        case 2:
        case 3:
        case 4:
          coords[count-2] = std::stof(content);
          break;
      }
    }

    ++count;
    if(count == 5)
    {
      count = 0;
      nFile << id << SEPARATOR << coords[0] << SEPARATOR <<  coords[1] << SEPARATOR <<  coords[2] << std::endl;
      ++idsCount;
    }

    if(dataFile.eof()) break;
  }
  dataFile.close();

  std::cout << "Write network file: " << NETWORK_FILENAME << " - " << idsCount << " ids" << std::endl;

  nFile.flush();
  nFile.close();

  return 0;
}
