# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
#   ViSimpl
#   2015-2019 (c) ViSimpl / Universidad Rey Juan Carlos
#   sergio.galindo@urjc.es
#
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

add_definitions(-DVISIMPL_SKIP_GLEW_INCLUDE )

set(PROJECT_VERSION_ABI ${${PROJECT_NAME}_VERSION_ABI})

set(Name Visimpl)
set(NAME VISIMPL)
set(namespace visimpl)
set(NAMESPACE VISIMPL)
set(INCLUDE_NAME visimpl)
configure_file(${CMAKE_SOURCE_DIR}/CMake/common/cpp/api.h
  ${PROJECT_BINARY_DIR}/include/visimpl/api.h @ONLY)
configure_file(${CMAKE_SOURCE_DIR}/CMake/common/cpp/version.h
  ${PROJECT_BINARY_DIR}/include/visimpl/version.h @ONLY)
configure_file(${CMAKE_SOURCE_DIR}/CMake/common/cpp/version.cpp
  ${PROJECT_BINARY_DIR}/src/visimpl/version.cpp @ONLY)

set(UIDIR ${PROJECT_SOURCE_DIR}/ui)
set(QRCDIR ${PROJECT_SOURCE_DIR}/qrc)

#set(CMAKE_AUTOMOC ON)
#set(CMAKE_AUTOUIC ON)

set(VISIMPL_SOURCES
  ${PROJECT_BINARY_DIR}/src/visimpl/version.cpp

  visimpl.ui
  resources.qrc

  visimpl.cpp
  MainWindow.cpp
  OpenGLWidget.cpp

  VisualGroup.cpp
  DomainManager.cpp

  SelectionManagerWidget.cpp
  SubsetImporter.cpp

  prefr/ColorSource.cpp
  prefr/ColorOperationModel.cpp
  prefr/SourceMultiPosition.cpp
  prefr/UpdaterStaticPosition.cpp

  render/Plane.cpp
)

set(VISIMPL_HEADERS
  ${PROJECT_BINARY_DIR}/include/visimpl/version.h
  OpenGLWidget.h
  MainWindow.h

  VisualGroup.h
  DomainManager.h

  SelectionManagerWidget.h
  SubsetImporter.h

  prefr/PrefrShaders.h
  prefr/ColorSource.h
  prefr/ColorOperationModel.h
  prefr/SourceMultiPosition.h
  prefr/UpdaterStaticPosition.h
  render/Plane.h
)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(VISIMPL_LINK_LIBRARIES
  Qt5::Core
  Qt5::Widgets
  Qt5::OpenGL
  ReTo
  SimIL
  prefr
  sumrice
  scoop
  acuterecorder
)

if(WIN32)
  if(MINGW)
  	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mwindows")
  endif(MINGW)
  configure_file("${PROJECT_SOURCE_DIR}/visimpl/visimplWinProps.rc.in" "${PROJECT_BINARY_DIR}/visimplWinProps.rc")
  set( VISIMPL_SOURCES ${VISIMPL_SOURCES} "${PROJECT_BINARY_DIR}/visimplWinProps.rc")
  add_definitions("-DVISIMPL_STATIC")
endif(WIN32)

if ( NOT DEFAULT_CONTEXT_OPENGL_MAJOR )
  set( DEFAULT_CONTEXT_OPENGL_MAJOR 4 )
endif( )
if ( NOT DEFAULT_CONTEXT_OPENGL_MINOR )
  set( DEFAULT_CONTEXT_OPENGL_MINOR 4 )
endif( )

add_definitions( "-DDEFAULT_CONTEXT_OPENGL_MAJOR=${DEFAULT_CONTEXT_OPENGL_MAJOR}" )
add_definitions( "-DDEFAULT_CONTEXT_OPENGL_MINOR=${DEFAULT_CONTEXT_OPENGL_MINOR}" )

if (ZEROEQ_FOUND)
  list(APPEND VISIMPL_LINK_LIBRARIES ZeroEQ ${CMAKE_THREAD_LIBS_INIT})
endif()

if (GMRVLEX_FOUND)
  list( APPEND VISIMPL_LINK_LIBRARIES gmrvlex )
endif()

if (LEXIS_FOUND)
  list( APPEND VISIMPL_LINK_LIBRARIES Lexis )
endif()

if (DEFLECT_FOUND)
  list(APPEND VISIMPL_LINK_LIBRARIES Deflect)
endif()

if (BRION_FOUND)
  list(APPEND VISIMPL_LINK_LIBRARIES Brion Brain)
endif()

if (APPLE)
  set(VISIMPL_ICON visimpl.icns)
endif()

if (UNIX AND NOT APPLE)
  set(VISIMPL_ICON visimpl.png )
endif()

common_application( visimpl GUI ${COMMON_APP_ARGS})
