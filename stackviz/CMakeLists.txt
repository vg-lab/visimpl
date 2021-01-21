# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
#   ViSimpl
#   2015-2016 (c) ViSimpl / Universidad Rey Juan Carlos
#   sergio.galindo@urjc.es
#
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

add_definitions(-DNEUROLOTS_SKIP_GLEW_INCLUDE )

set(PROJECT_VERSION_ABI ${${PROJECT_NAME}_VERSION_ABI})
 
set(Name Stackviz)
set(NAME STACKVIZ)
set(namespace stackviz)
set(NAMESPACE STACKVIZ)
set(INCLUDE_NAME stackviz)
configure_file(${CMAKE_SOURCE_DIR}/CMake/common/cpp/api.h
  ${PROJECT_BINARY_DIR}/include/stackviz/api.h @ONLY)
configure_file(${CMAKE_SOURCE_DIR}/CMake/common/cpp/version.h
  ${PROJECT_BINARY_DIR}/include/stackviz/version.h @ONLY)
configure_file(${CMAKE_SOURCE_DIR}/CMake/common/cpp/version.cpp
  ${PROJECT_BINARY_DIR}/src/stackviz/version.cpp @ONLY)


set(UIDIR ${PROJECT_SOURCE_DIR}/ui)
set(QRCDIR ${PROJECT_SOURCE_DIR}/qrc)

set(STACKVIZ_SOURCES
  ${PROJECT_BINARY_DIR}/src/stackviz/version.cpp
  stackviz.ui
  resources.qrc
  stackviz.cpp
  MainWindow.cpp
  DisplayManagerWidget.cpp
)

set(STACKVIZ_HEADERS
  ${PROJECT_BINARY_DIR}/include/stackviz/version.h
  MainWindow.h
  DisplayManagerWidget.h
  log.h  
)


include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(STACKVIZ_LINK_LIBRARIES
  Qt5::Core
  Qt5::Widgets
  Qt5::OpenGL
  prefr  
  sumrice
  scoop
)

if(WIN32)
  if(MINGW)
  	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mwindows")
  endif(MINGW)
  configure_file("${PROJECT_SOURCE_DIR}/stackviz/stackvizWinProps.rc.in" "${PROJECT_BINARY_DIR}/stackvizWinProps.rc")
  set( STACKVIZ_SOURCES ${STACKVIZ_SOURCES} "${PROJECT_BINARY_DIR}/stackvizWinProps.rc")  
  add_definitions("-DSTACKVIZ_STATIC")
endif(WIN32)

if ( NOT DEFAULT_CONTEXT_OPENGL_MAJOR )
  set( DEFAULT_CONTEXT_OPENGL_MAJOR 3 )
endif( )
if ( NOT DEFAULT_CONTEXT_OPENGL_MINOR )
  set( DEFAULT_CONTEXT_OPENGL_MINOR 3 )
endif( )

add_definitions( "-DDEFAULT_CONTEXT_OPENGL_MAJOR=${DEFAULT_CONTEXT_OPENGL_MAJOR}" )
add_definitions( "-DDEFAULT_CONTEXT_OPENGL_MINOR=${DEFAULT_CONTEXT_OPENGL_MINOR}" )

if (ZEROEQ_FOUND)
  list(APPEND STACKVIZ_LINK_LIBRARIES ZeroEQ ${CMAKE_THREAD_LIBS_INIT})
endif()

if (GMRVLEX_FOUND)
  list( APPEND STACKVIZ_LINK_LIBRARIES gmrvlex )
endif()

if (LEXIS_FOUND)
  list( APPEND STACKVIZ_LINK_LIBRARIES Lexis )
endif()

if (DEFLECT_FOUND)
  list(APPEND STACKVIZ_LINK_LIBRARIES Deflect)
endif()

if (BRION_FOUND)
  list(APPEND STACKVIZ_LINK_LIBRARIES Brion Brain)
endif()

common_application( stackviz GUI ${COMMON_APP_ARGS})
