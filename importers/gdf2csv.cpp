/** GDF TO CSV
 * File: gdf2csv.cpp
 * Author: Félix de las Pozas Álvarez
 * Date: 22/02/2022
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

std::string NETWORK_FILENAME = "gdf_network.csv";
std::string ACTiVITY_FILENAME = "gdf_activity.csv";

const double TIME_INTERVAL = 0.1;

struct dotSeparator: std::numpunct<char>
{
    char do_decimal_point() const { return '.'; }
};

template<class T> void ignore( const T& ) { }

void printUsage(const char *filename)
{
  const std::string name = (filename ? std::string(filename) : "GDF2CSV");

  std::cout << name << '\n';
  std::cout << "Generates CSV histograms from GDF files." << '\n';
  std::cout << "Usage: " << name << " <gdf_dir>" << std::endl;
  std::exit(-1);
}

int main(int argc, char* argv[])
{
  if(argc < 2) printUsage(argv[0]);

  std::random_device dev;
  std::mt19937 rng(dev());
  std::uniform_int_distribution<std::mt19937::result_type> dist(1,100);
  std::uniform_int_distribution<std::mt19937::result_type> height(1,40);

  std::ofstream nFile, aFile;
  nFile.open(NETWORK_FILENAME, std::fstream::out|std::fstream::trunc);
  aFile.open(ACTiVITY_FILENAME, std::fstream::out|std::fstream::trunc);

  if(nFile.fail() || aFile.fail())
  {
    std::cerr << "ERROR: Unable to open: " << NETWORK_FILENAME << " or " << ACTiVITY_FILENAME << std::endl;
    std::exit(-1);
  }

  // write floating point numbers with dot separation no matter the locale
  nFile.imbue(std::locale(nFile.getloc(), new dotSeparator()));
  aFile.imbue(std::locale(aFile.getloc(), new dotSeparator()));

  std::cout << "Write network file: " << NETWORK_FILENAME << std::endl;

  for(unsigned int i = 2; i <= 78072; ++i)
  {
    int level = 1000-height(rng);

    if(i > 20684) level -= 50;
    if(i > 26518) level -= 50;
    if(i > 48433) level -= 50;
    if(i > 53912) level -= 50;
    if(i > 58762) level -= 50;
    if(i > 59827) level -= 50;
    if(i > 74222) level -= 50;
    if(i > 77170) level -= 50;

    nFile << i << ", " << dist(rng) << ", " << dist(rng) << ", " << level << '\n';
  }
  nFile.flush();
  nFile.close();

  auto path = std::string(argv[1]);
  if(path.back() != '/')
    path += '/';

  DIR *dir;
  struct dirent *ent;
  std::vector<std::string> filenames;

  if((dir = opendir(path.c_str())) != nullptr)
  {
    /* print all the files and directories within directory */
    while ((ent = readdir (dir)) != nullptr)
    {
      filenames.emplace_back(ent->d_name);
    }
    closedir (dir);
  }
  else
  {
    std::cerr << "Unable to open directory: " << path << std::endl;
    std::exit(-1);
  }

  // remove dotfiles
  auto itr = std::find(filenames.begin(), filenames.end(), ".");
  if (itr != filenames.end()) filenames.erase(itr);
  itr = std::find(filenames.begin(), filenames.end(), "..");
  if (itr != filenames.end()) filenames.erase(itr);

  std::sort(filenames.begin(), filenames.end());
  const int EVENTS = filenames.size();
  std::cout << "Found " << EVENTS << " file(s)" << std::endl;

  unsigned char **buffer = new unsigned char*[100000];
  for(int i = 0; i < 100000; ++i)
  {
    buffer[i] = new unsigned char[78072];
    memset(buffer[i], 0, sizeof(unsigned char)*78072);
  }

  for(int i = 0; i < EVENTS; ++i)
  {
    const auto file = path + filenames.at(i);
    std::cout << "Processing " << i+1 << ": " << file << std::endl;

    std::ifstream dataFile;
    dataFile.open(file, std::fstream::in);

    if(dataFile.fail())
    {
      std::cerr << "ERROR: Unable to open: " << file << std::endl;
      std::exit(-1);
    }

    unsigned int lineNum = 0;
    for(std::string line; getline(dataFile,  line); )
    {
      ++lineNum;
      auto qLine = QString::fromStdString(line);
      auto parts = qLine.split('\t');
      parts.removeAll(QString());
      if(parts.size() != 2)
      {
        std::cerr << "ERROR: Unable parse line " << lineNum << ": " << line << " from file " << file << std::endl;
        std::exit(-1);
      }

      const auto num = parts.first().toInt();
      const auto timeval = static_cast<int>(parts.last().toDouble() * 10);
      buffer[timeval][num] = 1;
      assert(timeval < 100000);
    }

    dataFile.close();
  }

  for(int i = 0; i < 100000; ++i)
  {
    const double time = TIME_INTERVAL * i;
    for(int j = 2; j <= 78072; ++j)
    {
      if(buffer[i][j] == 1)
      {
        aFile << j << ", " << time << '\n';
      }
    }
  }
  aFile.flush();
  aFile.close();

  std::cout << "Write activity file: " << ACTiVITY_FILENAME << std::endl;
  std::cout << "Processed " << filenames.size() << " files successfully" << std::endl;

  return 0;
}

