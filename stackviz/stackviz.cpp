/*
 * @file  stackviz.cpp
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */

#include <QApplication>
#include "MainWindow.h"
#include <QDebug>
#include <QOpenGLWidget>
#include <QDir>

#include <stackviz/version.h>
#include <sumrice/sumrice.h>

void setFormat( void );
void usageMessage(  char* progName );
void dumpVersion( void );
bool atLeastTwo( bool a, bool b, bool c );

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

  std::string networkFile;
  std::string activityFile;
  std::string zeqUri;
  std::string target ( "" );
  std::string report ( "" );
  std::string subsetEventFile( "" );
  std::string correlations( "" );

  simil::TDataType dataType = simil::TBlueConfig;
  simil::TSimulationType simType = simil::TSimSpikes;


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
      }
      else
        usageMessage( argv[0] );

    }
    else if( std::strcmp( argv[ i ], "-h5" ) == 0 )
    {
      if( i + 2 < argc )
      {
        ++i;
        networkFile = std::string( argv[ i ]);
        ++i;
        activityFile = std::string( argv[ i ]);
        dataType = simil::THDF5;
      }
      else
        usageMessage( argv[0] );
    }

    if( std::strcmp( argv[ i ], "-target" ) == 0 )
    {
      if(++i < argc )
      {
        target = argv[ i ];
      }
      else
        usageMessage( argv[0] );
    }

    if( std::strcmp( argv[ i ], "-correlations") == 0 )
    {
      if(++i < argc )
      {
        correlations = argv[ i ];
      }
      else
        usageMessage( argv[0] );
    }

    if( std::strcmp( argv[ i ], "-spikes" ) == 0 )
    {
       simType = simil::TSimSpikes;
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

    if ( strcmp( argv[i], "--fullscreen" ) == 0 ||
         strcmp( argv[i],"-fs") == 0 )
    {
      fullscreen = true;
    }
    if ( strcmp( argv[i], "--maximize-window" ) == 0 ||
         strcmp( argv[i],"-mw") == 0 )
    {
      initWindowMaximized = true;
    }
    if ( strcmp( argv[i], "--window-size" ) == 0 ||
         strcmp( argv[i],"-ws") == 0 )
    {
      initWindowSize = true;
      if ( i + 2 >= argc )
        usageMessage( argv[0] );
      initWindowWidth = atoi( argv[ ++i ] );
      initWindowHeight = atoi( argv[ ++i ] );

    }
  }

  stackviz::MainWindow mainWindow;
  mainWindow.setWindowTitle("StackViz");

  if ( initWindowSize )
    mainWindow.resize( initWindowWidth, initWindowHeight );

  if ( initWindowMaximized )
    mainWindow.showMaximized( );

  if ( fullscreen )
    mainWindow.showFullScreen( );

  mainWindow.show( );
  mainWindow.init( zeqUri );

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
            << "\t[ -bc blue_config_path [-target target_name]"
            << std::endl
            << "\t[ -spikes ] "
            << std::endl
            << "\t[ -zeq schema* ]"
            << std::endl
            << "\t[ -ws | --window-size ] width height ]"
            << std::endl
            << "\t[ -fs | --fullscreen ] "
            << std::endl
            << "\t[ -mw | --maximize-window ]"
            << std::endl
            << "\t[ --version ]"
            << std::endl
            << "\t[ --help | -h ]"
            << std::endl << std::endl
            << "* schema: for example hbp://"
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

  std::cerr << "Deflect support built-in: ";
  #ifdef VISIMPL_USE_DEFLECT
  std::cerr << "\tyes";
  #else
  std::cerr << "\tno";
  #endif
  std::cerr << std::endl;

  std::cerr << std::endl;

}
