# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
#   DemoSimPrefr
#   2015 (c) demoSimPrefr / Universidad Rey Juan Carlos
#   sergio.galindo@urjc.es
#
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

# ViSimpl project and version
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

common_find_package( Qt5Core 5.4 REQUIRED )
common_find_package( Qt5Gui 5.4 REQUIRED )
common_find_package( Qt5Widgets 5.4 REQUIRED )
common_find_package( Qt5OpenGL REQUIRED )
common_find_package( GLEW REQUIRED SYSTEM )
# common_find_package( GLUT REQUIRED SYSTEM )
common_find_package( OpenGL REQUIRED )
common_find_package( Eigen3 REQUIRED SYSTEM)
common_find_package( nsol REQUIRED )
common_find_package( neurolots REQUIRED )
common_find_package( prefr REQUIRED )
common_find_package( Brion REQUIRED )

if(VISIMPL_WITH_ZEQ)
  common_find_package(zeq)
  if ( ZEQ_FOUND )
    list(APPEND VISIMPL_DEPENDENT_LIBRARIES zeq)
    common_find_package(Threads REQUIRED)
    list(APPEND VISIMPL_DEPENDENT_LIBRARIES Threads)
  endif( )
endif( )

if(VISIMPL_WITH_GMRVZEQ)
  common_find_package(gmrvzeq)
  if ( GMRVZEQ_FOUND )
    list(APPEND VISIMPL_DEPENDENT_LIBRARIES gmrvzeq)
  endif( )
endif( )

common_find_package_post()

add_subdirectory(src)

include(CPackConfig)
include(DoxygenRule)
