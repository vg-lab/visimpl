# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
#   DemoSimPrefr
#   2015 (c) demoSimPrefr / Universidad Rey Juan Carlos
#   sergio.galindo@urjc.es
#
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

cmake_minimum_required(VERSION 2.8)
project(visimpl)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMake
  ${CMAKE_SOURCE_DIR}/CMake/common)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif(NOT CMAKE_BUILD_TYPE)

if (CMAKE_BUILD_TYPE MATCHES "Debug")
  add_definitions(-DDEBUG)
endif()

include(GitExternal)

# QtSimpleViewer version
set(LAST_RELEASE 0.0.0)
set(VERSION_MAJOR "0")
set(VERSION_MINOR "1")
set(VERSION_PATCH "0")
set(VERSION_ABI 1)

# description
set(VISIMPL_DESCRIPTION "Visimpl")
set(COMMON_PROJECT_DOMAIN gmrv.es)

include(Common)

option(VISIMPL_WITH_ZEQ "VISIMPL_WITH_ZEQ" ON)

common_package( Qt5Core 5.4 REQUIRED )
common_package( Qt5Gui 5.4 REQUIRED )
common_package( Qt5Widgets 5.4 REQUIRED )
common_package( Qt5OpenGL REQUIRED )
common_package( GLEW REQUIRED SYSTEM )
# common_package( GLUT REQUIRED SYSTEM )
common_package( OpenGL REQUIRED )
common_package( Eigen3 REQUIRED SYSTEM)
common_package( nsol REQUIRED )
common_package( neurolots REQUIRED )
common_package( prefr REQUIRED )
common_package( Brion REQUIRED )

list( APPEND VISIMPL_DEPENDENT_LIBRARIES 
  OpenGL 
  GLEW 
  Eigen3 
  nsol 
  neurolots 
  prefr 
  Brion 
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

if(VISIMPL_WITH_ZEQ)
  common_package(zeq)
  if ( ZEQ_FOUND )
    list(APPEND VISIMPL_DEPENDENT_LIBRARIES zeq)
    common_package(Threads REQUIRED)
    list(APPEND VISIMPL_DEPENDENT_LIBRARIES Threads)
  endif( )
endif( )

if(VISIMPL_WITH_GMRVZEQ)
  common_package(gmrvzeq)
  if ( GMRVZEQ_FOUND )
    list(APPEND VISIMPL_DEPENDENT_LIBRARIES gmrvzeq)
  endif( )
endif( )

common_package_post()

add_subdirectory( sumrice )
add_subdirectory( visimpl )
add_subdirectory( stackviz )

include(CPackConfig)
include(DoxygenRule)
