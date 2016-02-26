add_definitions(-DNEUROLOTS_SKIP_GLEW_INCLUDE )

set(Name Visimpl)
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

set(Name Visimpl)
set(NAME STACKVIZ)
set(namespace stackviz)
set(NAMESPACE STACKVIZ)
set(INCLUDE_NAME stackviz)
configure_file(${CMAKE_SOURCE_DIR}/CMake/common/cpp/api.h
  ${PROJECT_BINARY_DIR}/include/stackviz/api.h @ONLY)
configure_file(${CMAKE_SOURCE_DIR}/CMake/common/cpp/version.h
  ${PROJECT_BINARY_DIR}/include/stackviz/version.h @ONLY)
configure_file(${CMAKE_SOURCE_DIR}/CMake/common/cpp/version.cpp
  ${PROJECT_BINARY_DIR}/src/version.cpp @ONLY)


set(STACKVIZ_SOURCES
  ${PROJECT_BINARY_DIR}/src/stackviz/version.cpp
  stackviz.cpp
  MainWindow.cpp
)

set(STACKVIZ_HEADERS
  ${PROJECT_BINARY_DIR}/include/stackviz/version.h
  MainWindow.h
  log.h
)

set(STACKVIZ_MOC_HEADERS
  MainWindow.h
)

set(STACKVIZ_UI_FORMS
  ${UIDIR}/stackviz.ui
)

include_directories(${CMAKE_CURRENT_BINARY_DIR})


set(STACKVIZ_RESOURCES
  ${QRCDIR}/resources.qrc
)


set(STACKVIZ_LINK_LIBRARIES
  Qt5::Core
  Qt5::Widgets
  Qt5::OpenGL
  prefr  
  Brion
  sumrice
)

if ( NOT DEFAULT_CONTEXT_OPENGL_MAJOR )
  set( DEFAULT_CONTEXT_OPENGL_MAJOR 3 )
endif( )
if ( NOT DEFAULT_CONTEXT_OPENGL_MINOR )
  set( DEFAULT_CONTEXT_OPENGL_MINOR 3 )
endif( )

add_definitions( "-DDEFAULT_CONTEXT_OPENGL_MAJOR=${DEFAULT_CONTEXT_OPENGL_MAJOR}" )
add_definitions( "-DDEFAULT_CONTEXT_OPENGL_MINOR=${DEFAULT_CONTEXT_OPENGL_MINOR}" )

if (ZEQ_FOUND)
  list(APPEND STACKVIZ_LINK_LIBRARIES zeq zeqHBP ${CMAKE_THREAD_LIBS_INIT})
endif()

if (GMRVZEQ_FOUND)
  list( APPEND STACKVIZ_LINK_LIBRARIES gmrvzeq )
endif()

if (DEFLECT_FOUND)
  list(APPEND STACKVIZ_LINK_LIBRARIES Deflect)
endif()

if (BRION_FOUND)
  list(APPEND STACKVIZ_LINK_LIBRARIES Brion Brain)
endif()

common_application( stackviz )
