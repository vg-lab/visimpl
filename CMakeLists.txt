# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
#   PFM 
#   2015 (c) MIGJRVI / Universidad Rey Juan Carlos
#   r.jarillo@alumnos.urjc.es
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
# common_package( zeq REQUIRED)
# common_package( gmrvzeq REQUIRED)
common_package_post()

add_subdirectory(src)

include(CPackConfig)
include(DoxygenRule)
