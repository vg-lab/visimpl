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

// Qt
#include "MainWindow.h"

#include <QApplication>
#include <QDebug>
#include <QOpenGLWidget>
#include <QDir>

// Project
#include <stackviz/version.h>
#include <sumrice/sumrice.h>

#ifdef VISIMPL_USE_ZEROEQ
// zeroeq
#include <zeroeq/types.h>
#endif

void usageMessage(  char* progName );
void dumpVersion( void );

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

  QApplication application(argc,argv);

  std::string networkFile, activityFile, subsetEventFile;
  std::string zeqUri;
  std::string target;
  std::string correlations;

  simil::TDataType dataType = simil::TBlueConfig;
  // @felix This shouldn't be constexpr? Could change to voltages in the future?
  constexpr simil::TSimulationType simType = simil::TSimSpikes;

  bool fullscreen = false, initWindowSize = false, initWindowMaximized = false;
  int initWindowWidth, initWindowHeight;

  for( int i = 1; i < argc; i++ )
  {
    if ( std::strcmp( argv[i], "--help" ) == 0 ||
         std::strcmp( argv[i], "-h" ) == 0 )
    {
      usageMessage( argv[0] );
      return 0;
    }

    if ( std::strcmp( argv[i], "--version" ) == 0 )
    {
      dumpVersion( );
      return 0;
    }

    if( std::strcmp( argv[ i ], "-zeq" ) == 0 )
    {
#ifdef VISIMPL_USE_ZEROEQ
      if( ++i < argc )
      {
        zeqUri = std::string( argv[ i ]);
        continue;
      }
#else
      std::cerr << "Zeq not supported " << std::endl;
      return -1;
#endif
    }

    if( std::strcmp( argv[ i ], "-bc" ) == 0 )
    {
      if( ++i < argc )
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
      if( i + 2 < argc )
      {
        ++i;
        networkFile = std::string( argv[ i ]);
        ++i;
        activityFile = std::string( argv[ i ]);
        dataType = simil::THDF5;
        continue;
      }
      else
        usageMessage( argv[0] );
    }

    if( std::strcmp( argv[ i ], "-csv") == 0 )
    {
      if( i + 2 < argc )
      {
        ++i;
        networkFile = std::string( argv[ i ]);
        ++i;
        activityFile = std::string( argv[ i ]);
        dataType = simil::TCSV;
        continue;
      }
    }

    if( std::strcmp( argv[ i ], "-rest") == 0 )
    {
#ifdef SIMIL_WITH_REST_API
      if( i + 2 < argc )
      {
        ++i;
        networkFile = std::string( argv[ i ]);
        ++i;
        activityFile = std::string( argv[ i ]);
        dataType = simil::TREST;
        continue;
      }
#else
        std::cerr << "REST API not supported." << std::endl;
        return -1;
#endif
    }

    if( std::strcmp( argv[ i ], "-target" ) == 0 )
    {
      if(++i < argc )
      {
        target = argv[ i ];
        continue;
      }
      else
        usageMessage( argv[0] );
    }

    if( std::strcmp( argv[ i ], "-correlations") == 0 )
    {
      if(++i < argc )
      {
        correlations = argv[ i ];
        continue;
      }
      else
        usageMessage( argv[0] );
    }

    if( strcmp( argv[ i ], "-se" ) == 0 )
    {
      if( ++i < argc )
      {
        subsetEventFile = std::string( argv[ i ]);
        continue;
      }
      else
        usageMessage( argv[ 0 ]);
    }

    if ( strcmp( argv[i], "--fullscreen" ) == 0 ||
         strcmp( argv[i],"-fs") == 0 )
    {
      fullscreen = true;
      continue;
    }

    if ( strcmp( argv[i], "--maximize-window" ) == 0 ||
         strcmp( argv[i],"-mw") == 0 )
    {
      initWindowMaximized = true;
      continue;
    }

    if ( strcmp( argv[i], "--window-size" ) == 0 ||
         strcmp( argv[i],"-ws") == 0 )
    {
      initWindowSize = true;
      if ( i + 2 >= argc )
        usageMessage( argv[0] );
      initWindowWidth = atoi( argv[ ++i ] );
      initWindowHeight = atoi( argv[ ++i ] );
      continue;
    }
  }

  stackviz::MainWindow mainWindow;
  mainWindow.setWindowTitle("StackViz");

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
    mainWindow.show( );

  if( !networkFile.empty( ))
  switch( dataType )
  {
    case simil::TDataType::TBlueConfig:
      mainWindow.openBlueConfig( networkFile, simType, target,
                                 subsetEventFile );
      break;
    case simil::TDataType::THDF5:
      mainWindow.openHDF5File( networkFile, simType, activityFile,
                               subsetEventFile );
      if( !correlations.empty( ))
      {
        QString correls( correlations.c_str( ));
        auto co = correls.split( ";" );
        for( auto c : co )
          mainWindow.addCorrelation( c.toStdString( ));
      }
      mainWindow.calculateCorrelations( );
      break;
    case simil::TDataType::TCSV:
      mainWindow.openCSVFile( networkFile, simType, activityFile, subsetEventFile );
      break;
#ifdef SIMIL_WITH_REST_API
    case simil::TDataType::TREST:
      mainWindow.openRestListener( networkFile, simType, activityFile, subsetEventFile);
      break;
#endif
    default:
      break;
  }

  return application.exec();
}

void usageMessage( char* progName )
{
  std::cerr << std::endl
            << "Usage: "
            << progName << std::endl
            << "\t[ -bc <blue_config_path> [-target <target_name>] | "
            << "-csv <network_path> <activity_path> ] "
            << std::endl
#ifdef SIMIL_WITH_REST_API
            << "\t[ -rest <url> <port> ]"
            << std::endl
#endif
            << "\t[ -se <subset_events_file> ] "
            << std::endl
#ifdef VISIMPL_USE_ZEROEQ
            << "\t[ -zeq <session_name*> ]"
            << std::endl
#endif
            << "\t[ -ws | --window-size ] <width> <height> ]"
            << std::endl
            << "\t[ -fs | --fullscreen ] "
            << std::endl
            << "\t[ -mw | --maximize-window ]"
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
            << "stackviz "
            << stackviz::Version::getMajor( ) << "."
            << stackviz::Version::getMinor( ) << "."
            << stackviz::Version::getPatch( )
            << " (" << stackviz::Version::getRevision( ) << ")"
            << std::endl << std::endl;

  std::cerr << "ZeroEQ support built-in: ";
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

  std::cerr << "REST API support: ";
#ifdef SIMIL_WITH_REST_API
  std::cerr << "\tyes";
#else
  std::cerr << "\tno";
#endif

  std::cerr << std::endl;
  std::cerr << std::endl;
}
