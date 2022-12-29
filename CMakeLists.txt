# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
#   ViSimpl
#   2015-2016 (c) ViSimpl / Universidad Rey Juan Carlos
#   sergio.galindo@urjc.es
#
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

# ViSimpl project and version
cmake_minimum_required( VERSION 3.1 FATAL_ERROR )

# visimpl project and version
project( visimpl VERSION 1.8.4 )
set( visimpl_VERSION_ABI 6 )

SET( VISIMPL_LICENSE "GPL")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Disable in source building
if( "${PROJECT_SOURCE_DIR}" STREQUAL "${PROJECT_BINARY_DIR}" )
  message( FATAL "no in source building allowed.")
endif()

list( APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMake
  ${CMAKE_SOURCE_DIR}/CMake/common )

if(NOT EXISTS ${CMAKE_SOURCE_DIR}/CMake/common/Common.cmake)
  message(FATAL_ERROR "CMake/common missing, run: git submodule update --init")
endif()

if( NOT CMAKE_BUILD_TYPE )
  set( CMAKE_BUILD_TYPE Debug )
endif( NOT CMAKE_BUILD_TYPE )

if( CMAKE_BUILD_TYPE MATCHES "Debug" )
  add_definitions(-DDEBUG)
endif()

if( VISIMPL_OPTIONALS_AS_REQUIRED )
  set( VISIMPL_OPTS_FIND_ARGS "REQUIRED" )
else()
  set( VISIMPL_OPTS_FIND_ARGS "" )
endif()


include(GitExternal)

# description
set(VISIMPL_DESCRIPTION "Visimpl")
set(COMMON_PROJECT_DOMAIN es.vg-lab)

include(Common)

option(VISIMPL_WITH_ZEQ "VISIMPL_WITH_ZEQ" ON)
option(VISIMPL_WITH_GMRVLEX "VISIMPL_WITH_GMRVLEX" ON)

set(VISIMPL_WITH_OPENMP ON)

common_find_package( GLM REQUIRED SYSTEM )
common_find_package( Qt5Core 5.4 REQUIRED )
common_find_package( Qt5Gui 5.4 REQUIRED )
common_find_package( Qt5Widgets 5.4 REQUIRED )
common_find_package( Qt5OpenGL REQUIRED )
common_find_package( GLEW REQUIRED SYSTEM )
common_find_package( OpenGL REQUIRED )
common_find_package( Eigen3 REQUIRED SYSTEM)
common_find_package( HDF5 REQUIRED COMPONENTS C CXX )
common_find_package( ReTo REQUIRED )
common_find_package( SimIL REQUIRED )
common_find_package( plab REQUIRED )
common_find_package( scoop REQUIRED )
common_find_package( acuterecorder REQUIRED )

if ( VISIMPL_WITH_OPENMP )
  common_find_package( OpenMP )
endif()
common_find_package( Boost REQUIRED )


list( APPEND VISIMPL_DEPENDENT_LIBRARIES
  OpenGL
  GLEW
  Eigen3
  ReTo
  SimIL
  plab
  Qt5Core
  Qt5Widgets
  Qt5OpenGL
  Boost
  acuterecorder
)

if( OPENMP_FOUND )
  list( APPEND VISIMPL_DEPENDENT_LIBRARIES OpenMP )
endif( )

if( VISIMPL_WITH_ZEQ )
  common_find_package( ZeroEQ )
  if ( ZEROEQ_FOUND )
    list( APPEND VISIMPL_DEPENDENT_LIBRARIES ZeroEQ )
    common_find_package( Threads ${SIMIL_OPTS_FIND_ARGS} )
    list( APPEND VISIMPL_DEPENDENT_LIBRARIES Threads )

    common_find_package( Lexis  ${SIMIL_OPTS_FIND_ARGS} )
    if( LEXIS_FOUND )
      list( APPEND VISIMPL_DEPENDENT_LIBRARIES Lexis )
    endif( )

    if( VISIMPL_WITH_GMRVLEX )
    common_find_package( gmrvlex  ${SIMIL_OPTS_FIND_ARGS} )
    if ( GMRVLEX_FOUND )
      list( APPEND VISIMPL_DEPENDENT_LIBRARIES gmrvlex )
    endif( )
  endif( )
  endif( )
endif( )

if (SIMIL_WITH_REST_API)
  add_definitions(-DSIMIL_WITH_REST_API=1)
endif()  

common_find_package_post()

add_subdirectory( sumrice )
add_subdirectory( visimpl )
add_subdirectory( stackviz )

# importers subdirectory contains code for processing data and convert it 
# to ViSimpl formats, not needed for release.
#add_subdirectory( importers )

include( CPackConfig )
include( DoxygenRule )
