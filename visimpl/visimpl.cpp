/*
 * Copyright (c) 2015-2020 VG-Lab/URJC.
 *
 * Authors: Sergio E. Galindo <sergio.galindo@urjc.es>
 *
 * This file is part of ViSimpl <https://github.com/vg-lab/visimpl>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3.0 as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#ifdef WIN32
#include <winsock2.h>
#endif

// Qt
#include <QApplication>
#include <QDebug>
#include <QOpenGLWidget>
#include <QDir>
#include <QFile>
#include <QString>
#include <QApplication>

// Project
#include "MainWindow.h"
#include <visimpl/version.h>

#ifdef VISIMPL_USE_ZEROEQ
// zeroeq
#include <zeroeq/types.h>
#endif

#define GL_MINIMUM_REQUIRED_MAJOR 4
#define GL_MINIMUM_REQUIRED_MINOR 0

constexpr float TESTTIME = 2.f;
constexpr int TESTPARTICLES = 50; // num particles == TESTPARTICLES^3

bool setFormat( void );
void usageMessage(  char* progName );
void dumpVersion( void );
bool atLeastTwo( bool a, bool b, bool c );
bool generateTestFiles(const QString &path, std::string &networkFile, std::string &activityFile);

int main( int argc, char** argv )
{
  // Linux osg obj importer has a bug with non english lang.
#ifndef Win32
  setenv("LANG", "C", 1);
#endif

#if defined(Q_OS_MAC)
  QDir dir( QFileInfo( argv[0] ).dir( )); // e.g. appdir/Contents/MacOS/appname
  dir.cdUp( );
  QCoreApplication::addLibraryPath(
    dir.absolutePath( ) + QString( "/Plugins" ));
#endif

  QApplication application( argc, argv );

  simil::TSimulationType simType = simil::TSimSpikes;
  simil::TDataType dataType = simil::TDataUndefined;

  std::string networkFile, activityFile, zeqUri, subsetEventFile, scaleFactor;

  bool fullscreen = false, initWindowSize = false, initWindowMaximized = false;
  int initWindowWidth, initWindowHeight;

  for( int i = 1; i < argc; i++ )
  {
    if ( std::strcmp( argv[i], "--help" ) == 0 ||
         std::strcmp( argv[i], "-h" ) == 0 )
    {
      usageMessage( argv[0] );
    }
    if ( std::strcmp( argv[i], "--version" ) == 0 )
    {
      dumpVersion( );
    }
    if( std::strcmp( argv[ i ], "-zeq" ) == 0 )
    {
#ifdef VISIMPL_USE_ZEROEQ
      if( ++i < argc )
      {
        zeqUri = std::string( argv[ i ]);
        continue;
      }
      else
        usageMessage(argv[0]);
#else
      std::cerr << "ZeroEQ not supported." << std::endl;
      return -1;
#endif
    }
    if( std::strcmp( argv[ i ], "-bc" ) == 0 )
    {
      if( ++i < argc && dataType == simil::TDataUndefined)
      {
        networkFile = std::string( argv[ i ]);
        dataType = simil::TBlueConfig;
        continue;
      }
      else
        usageMessage( argv[0] );
    }
    if( std::strcmp( argv[ i ], "-h5" ) == 0 )
    {
      if( i + 2 < argc && dataType == simil::TDataUndefined)
      {
        dataType = simil::THDF5;
        networkFile = std::string( argv[ ++i ]);
        activityFile = std::string( argv[ ++i ]);
        continue;
      }
      else
        usageMessage( argv[0] );
    }
    if( std::strcmp( argv[ i ], "-csv") == 0 )
    {
      if( i + 2 < argc && dataType == simil::TDataUndefined)
      {
        dataType = simil::TCSV;
        networkFile = std::string( argv[ ++i ]);
        activityFile = std::string( argv[ ++i ]);
        continue;
      }
      else
        usageMessage(argv[0]);
    }

    else if( std::strcmp( argv[ i ], "-rest") == 0 )
    {
#ifdef SIMIL_WITH_REST_API
      if( i + 2 < argc && dataType == simil::TDataUndefined)
      {
        dataType = simil::TREST;
        networkFile = std::string( argv[ ++i ]);
        activityFile = std::string( argv[ ++i ]);
        continue;
      }
      else
        usageMessage(argv[0]);
#else
      std::cerr << "REST API not supported." << std::endl;
      exit(-1);
#endif
    }

    if( strcmp( argv[ i ], "-se" ) == 0 )
    {
      if( ++i < argc )
      {
        subsetEventFile = std::string( argv[ i ]);
      }
      else
        usageMessage( argv[ 0 ]);
    }

    if( std::strcmp( argv[ i ], "-target" ) == 0 )
    {
      if( ++i < argc )
      {
        activityFile = argv[ i ];
      }
      else
        usageMessage( argv[0] );
    }

    if( std::strcmp( argv[ i ], "-scale" ) == 0 )
    {
      if( ++i < argc )
      {
        scaleFactor = argv[ i ];
      }
      else
        usageMessage( argv[0] );
    }

    if( std::strcmp( argv[ i ], "-spikes" ) == 0 )
    {
      simType = simil::TSimSpikes;
      continue;
    }
    if( std::strcmp( argv[ i ], "-voltages" ) == 0 )
    {
      if(++i < argc )
      {
        simType = simil::TSimVoltages;
        activityFile = std::string( argv[ i ]);
        continue;
      }
      else
        usageMessage( argv[0] );
    }

    if ( strcmp( argv[i], "--fullscreen" ) == 0 ||
         strcmp( argv[i], "-fs") == 0 )
    {
      fullscreen = true;
      continue;
    }

    if ( strcmp( argv[i], "--maximize-window" ) == 0 ||
         strcmp( argv[i], "-mw") == 0 )
    {
      initWindowMaximized = true;
      continue;
    }

    if ( strcmp( argv[i], "--window-size" ) == 0 ||
         strcmp( argv[i], "-ws") == 0 )
    {
      initWindowSize = true;

      if ( i + 2 < argc )
      {
        initWindowWidth = atoi( argv[ ++i ] );
        initWindowHeight = atoi( argv[ ++i ] );
        continue;
      }
      else
        usageMessage( argv[0] );
    }

    if(strcmp(argv[i], "--testFile") == 0)
    {
      QString path;
      if(++i < argc)
      {
        path = QString::fromLocal8Bit(argv[i]);
      }

      if(!QDir(path).exists() || !QDir(path).isReadable())
      {
        std::cerr << "Invalid test file path: " << path.toStdString() << ". Using home directory instead." << std::endl;
        path = QDir::homePath();
      }

      if(!generateTestFiles(path, networkFile, activityFile))
        usageMessage(argv[0]);
      else
        dataType = simil::TCSV;
    }
  }

  if(!setFormat( ))
  {
    std::cerr << "Unable to set OpenGL format, minimum required "
              << GL_MINIMUM_REQUIRED_MAJOR << "." << GL_MINIMUM_REQUIRED_MINOR
              << std::endl;
    exit(-1);
  }

  visimpl::MainWindow mainWindow;
  mainWindow.setWindowTitle("SimPart");

#ifdef VISIMPL_USE_ZEROEQ
  if(zeqUri.empty())
  {
    zeqUri = zeroeq::DEFAULT_SESSION;
  }
#endif

  mainWindow.init( zeqUri );

  if ( initWindowSize )
    mainWindow.resize( initWindowWidth, initWindowHeight );

  if ( initWindowMaximized )
    mainWindow.showMaximized( );
  else if ( fullscreen )
    mainWindow.showFullScreen( );
  else
    mainWindow.showNormal( );

  if( !scaleFactor.empty( ))
  {
    QString qscaleFactor( scaleFactor.c_str( ));

    glm::vec3 scale( 1.0f, 1.0f, 1.0f );

    auto chunks = qscaleFactor.split(',');
    if( chunks.size( ) == 3 )
    {
      scale.x = chunks[ 0 ].toFloat( );
      scale.y = chunks[ 1 ].toFloat( );
      scale.z = chunks[ 2 ].toFloat( );

      mainWindow.setCircuitSizeScaleFactor( scale );
    }
  }

  if(dataType != simil::TDataUndefined)
  {
    mainWindow.loadData(dataType, networkFile, activityFile, simType, subsetEventFile);
  }

  return application.exec();
}

void usageMessage( char* progName )
{
  std::cerr << std::endl
            << "Usage: "
            << progName << std::endl
            << "\t[ -bc <blue_config_path> [-target <target> ] | "
            << "-csv <network_path> <activity_path> ] "
            << std::endl
#ifdef SIMIL_WITH_REST_API
            << "\t[ -rest <url> <port> ]"
            << std::endl
#endif
            << "\t[ -se <subset_events_file> ] "
            << std::endl
//            << "\t[ -spikes ] "
//            << std::endl
//            << "\t[ -voltage report_label ] "
//            << std::endl
            << "\t[ -scale <X,Y,Z> ]"
            << std::endl
            << "\t[ -zeq <session_name*> ]"
            << std::endl
            << "\t[ -ws | --window-size ] <width> <height> ]"
            << std::endl
            << "\t[ -fs | --fullscreen ] "
            << std::endl
            << "\t[ -mw | --maximize-window ]"
            << std::endl
            << "\t[ --testFile [path] ]"
            << std::endl
            << "\t[ --version ]"
            << std::endl
            << "\t[ --help | -h ]"
            << std::endl << std::endl
            << "* session_name: for example test://"
            << std::endl << std::endl;
  exit(-1);
}

void dumpVersion( void )
{
  std::cerr << std::endl
            << "visimpl "
            << visimpl::Version::getMajor( ) << "."
            << visimpl::Version::getMinor( ) << "."
            << visimpl::Version::getPatch( )
            << " (" << visimpl::Version::getRevision( ) << ")"
            << std::endl << std::endl;

  std::cerr << "zeq support built-in: ";
  #ifdef VISIMPL_USE_ZEROEQ
  std::cerr << "\t\tyes";
  #else
  std::cerr << "\t\tno";
  #endif
  std::cerr << std::endl;

  std::cerr << "GmrvZeq support built-in: ";
  #ifdef VISIMPL_USE_GMRVLEX
  std::cerr << "\tyes";
  #else
  std::cerr << "\tno";
  #endif
  std::cerr << std::endl;
  std::cerr << std::endl;

  exit(0);
}

bool setFormat( void )
{
  int ctxOpenGLMajor = DEFAULT_CONTEXT_OPENGL_MAJOR;
  int ctxOpenGLMinor = DEFAULT_CONTEXT_OPENGL_MINOR;
  int ctxOpenGLSamples = 0;
  int ctxOpenGLVSync = 1;

  const auto major = std::getenv("CONTEXT_OPENGL_MAJOR");
  if ( major )
    ctxOpenGLMajor = std::stoi( major );

  const auto minor = std::getenv("CONTEXT_OPENGL_MINOR");
  if ( minor )
    ctxOpenGLMinor = std::stoi( minor );

  const auto samples = std::getenv("CONTEXT_OPENGL_SAMPLES");
  if ( samples )
    ctxOpenGLSamples = std::stoi( samples );

  std::cout << "Setting OpenGL context to "
            << ctxOpenGLMajor << "." << ctxOpenGLMinor << std::endl;

  QSurfaceFormat format;
  format.setVersion( ctxOpenGLMajor, ctxOpenGLMinor);

  if ( ctxOpenGLSamples != 0 )
    format.setSamples( ctxOpenGLSamples );

  format.setSwapInterval( ctxOpenGLVSync );

  if ( std::getenv("CONTEXT_OPENGL_COMPATIBILITY_PROFILE"))
    format.setProfile( QSurfaceFormat::CompatibilityProfile );
  else
    format.setProfile( QSurfaceFormat::CoreProfile );

  QSurfaceFormat::setDefaultFormat( format );

  return ( format.majorVersion() >= GL_MINIMUM_REQUIRED_MAJOR ) &&
         ( format.minorVersion( ) >= GL_MINIMUM_REQUIRED_MINOR );
}

bool atLeastTwo( bool a, bool b, bool c )
{
  return a ^ b ? c : a;
}

bool generateTestFiles(const QString &path, std::string &networkFile, std::string &activityFile)
{
  QDir filePath = path.isEmpty() ? QApplication::applicationDirPath() : path;
  if(!filePath.isReadable())
  {
    std::cerr << "Path of test files cannot be accessed: " << filePath.absolutePath().toStdString() << std::endl;
    return false;
  }

  QFile nFile{filePath.absoluteFilePath("network.csv")};
  QFile aFile{filePath.absoluteFilePath("activity.csv")};

  if(nFile.exists() && aFile.exists())
  {
    std::cout << "Test files already exists in path: " << filePath.absolutePath().toStdString() << std::endl;
    networkFile = nFile.fileName().toStdString();
    activityFile = aFile.fileName().toStdString();
    return true;
  }

  if(!nFile.open(QIODevice::WriteOnly) || !aFile.open(QIODevice::WriteOnly))
  {
    std::cerr << "Unable to create test files in path: " << filePath.absolutePath().toStdString() << std::endl;
    return false;
  }

  std::cout << "Created test files in path: " << filePath.absolutePath().toStdString() << std::endl;
  std::cout << "Network: " << nFile.fileName().toStdString() << std::endl;
  std::cout << "Activity: " << aFile.fileName().toStdString() << std::endl;

  auto printDoubleWithPoint = [](const double v)
  {
    auto value = std::to_string(v);
    std::replace(value.begin(), value.end(), ',', '.');
    return value;
  };

  int index = 0;
  float time = 0.f;

  for(float i = -TESTPARTICLES/2 * 10; i < TESTPARTICLES/2 * 10; i+=10)
  {
    for(float j = -TESTPARTICLES/2 * 10; j < TESTPARTICLES/2 * 10; j+=10)
    {
      for(float k = -TESTPARTICLES/2 * 10; k < TESTPARTICLES/2 * 10; k+=10)
      {
        std::string line = std::to_string(index) + "," + printDoubleWithPoint(i) + "," + printDoubleWithPoint(j) + "," + printDoubleWithPoint(k) + "\n";
        nFile.write(line.c_str());

        line = std::to_string(index) + "," + printDoubleWithPoint(time) + "\n";
        aFile.write(line.c_str());
        line = std::to_string(index) + "," + printDoubleWithPoint((2*TESTTIME)-time) + "\n";
        aFile.write(line.c_str());

        ++index;
      }
    }
    time += TESTTIME/TESTPARTICLES;
  }

  nFile.flush();
  nFile.close();
  aFile.flush();
  aFile.close();

  networkFile = nFile.fileName().toStdString();
  activityFile = aFile.fileName().toStdString();

  return true;
}
