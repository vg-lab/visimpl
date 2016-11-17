# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
#   DemoSimPrefr
#   2015 (c) demoSimPrefr / Universidad Rey Juan Carlos
#   sergio.galindo@urjc.es
#
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

# ViSimpl project and version
cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

# visimpl project and version
project(visimpl VERSION 0.1.0)
set(visimpl_VERSION_ABI 1)

# Disable in source building
if("${PROJECT_SOURCE_DIR}" STREQUAL "${PROJECT_BINARY_DIR}")
  message(FATAL "no in source building allowed.")
endif()

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMake
  ${CMAKE_SOURCE_DIR}/CMake/common)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif(NOT CMAKE_BUILD_TYPE)

if (CMAKE_BUILD_TYPE MATCHES "Debug")
  add_definitions(-DDEBUG)
endif()

include(GitExternal)

# description
set(VISIMPL_DESCRIPTION "Visimpl")
set(COMMON_PROJECT_DOMAIN gmrv.es)

include(Common)

option(VISIMPL_WITH_ZEQ "VISIMPL_WITH_ZEQ" ON)
option(VISIMPL_WITH_GMRVLEX "VISIMPL_WITH_GMRVLEX" ON)

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
common_find_package( prefr REQUIRED )
common_find_package( Lexis )
common_find_package( OpenMP )


list( APPEND VISIMPL_DEPENDENT_LIBRARIES 
  OpenGL 
  GLEW 
  Eigen3  
  ReTo
  SimIL
  prefr 
)

if (QT5CORE_FOUND)
  list(APPEND VISIMPL_DEPENDENT_LIBRARIES Qt5Core)
endif( )

if (QT5WIDGETS_FOUND)
  list(APPEND VISIMPL_DEPENDENT_LIBRARIES Qt5Widgets)
endif( )

if (QT5OPENGL_FOUND)
  list(APPEND VISIMPL_DEPENDENT_LIBRARIES Qt5OpenGL)
endif( )

if( OPENMP_FOUND )
  list( APPEND VISIMPL_DEPENDENT_LIBRARIES OpenMP )
endif( )

if(VISIMPL_WITH_ZEQ)
  common_find_package(ZeroEQ)
  if ( ZEROEQ_FOUND )
    list(APPEND VISIMPL_DEPENDENT_LIBRARIES ZeroEQ)
    common_find_package(Threads REQUIRED)
    list(APPEND VISIMPL_DEPENDENT_LIBRARIES Threads)
    
    common_find_package(Lexis)
    if(LEXIS_FOUND)
      list(APPEND VISIMPL_DEPENDENT_LIBRARIES Lexis)
    endif()
    
    if(VISIMPL_WITH_GMRVLEX)
    common_find_package(gmrvlex)
    if ( GMRVLEX_FOUND )
      list(APPEND VISIMPL_DEPENDENT_LIBRARIES gmrvlex)
    endif( )
  endif( )
    
  endif( )
endif( )
common_find_package_post()

add_subdirectory( sumrice )
add_subdirectory( visimpl )
add_subdirectory( stackviz )
#add_subdirectory( examplesh5 )

include(CPackConfig)
include(DoxygenRule)
