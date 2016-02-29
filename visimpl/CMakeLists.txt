add_definitions(-DNEUROLOTS_SKIP_GLEW_INCLUDE )

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
  ${PROJECT_BINARY_DIR}/src/version.cpp @ONLY)


set(VISIMPL_SOURCES
  ${PROJECT_BINARY_DIR}/src/visimpl/version.cpp
  visimpl.cpp
  MainWindow.cpp
  OpenGLWidget.cpp
  CShader.cpp  
  CMakeSetup.rc  
  prefr/ColorEmissionNode.cpp
  prefr/ColorOperationPrototype.cpp
  prefr/CompositeColorEmitter.cpp
  prefr/CompositeColorUpdater.cpp
  prefr/DirectValuedEmissionNode.cpp
  prefr/DirectValuedUpdater.cpp
  prefr/DirectValuedEmitter.cpp
)

set(VISIMPL_HEADERS
  ${PROJECT_BINARY_DIR}/include/visimpl/version.h
  CShader.h
  OpenGLWidget.h
  MainWindow.h  
  prefr/ColorEmissionNode.h
  prefr/ColorOperationPrototype.h
  prefr/CompositeColorEmitter.h
  prefr/CompositeColorUpdater.h
  prefr/DirectValuedEmissionNode.h
  prefr/DirectValuedUpdater.h
  prefr/DirectValuedEmitter.h  
)

set(VISIMPL_MOC_HEADERS
  MainWindow.h
  OpenGLWidget.h
)

set(VISIMPL_UI_FORMS
  ${UIDIR}/visimpl.ui
)

include_directories(${CMAKE_CURRENT_BINARY_DIR})


set(VISIMPL_RESOURCES
  ${QRCDIR}/resources.qrc
)


set(VISIMPL_LINK_LIBRARIES
  Qt5::Core
  Qt5::Widgets
  Qt5::OpenGL
  nlfem
  nlgenerator
  nlrender
  prefr  
  Brion
  sumrice
)

if ( NVIDIAOPENGL_FOUND )
  link_directories(${NVIDIA_OPENGL_gl_LIBRARY_PATH})
  set(VISIMPL_LINK_LIBRARIES ${VISIMPL_LINK_LIBRARIES} ${NVIDIA_OPENGL_gl_LIBRARY}) 
else( )
  set(VISIMPL_LINK_LIBRARIES ${VISIMPL_LINK_LIBRARIES} ${OPENGL_LIBRARIES}) 
endif( )

if ( NOT DEFAULT_CONTEXT_OPENGL_MAJOR )
  set( DEFAULT_CONTEXT_OPENGL_MAJOR 3 )
endif( )
if ( NOT DEFAULT_CONTEXT_OPENGL_MINOR )
  set( DEFAULT_CONTEXT_OPENGL_MINOR 3 )
endif( )

add_definitions( "-DDEFAULT_CONTEXT_OPENGL_MAJOR=${DEFAULT_CONTEXT_OPENGL_MAJOR}" )
add_definitions( "-DDEFAULT_CONTEXT_OPENGL_MINOR=${DEFAULT_CONTEXT_OPENGL_MINOR}" )

if (ZEQ_FOUND)
  list(APPEND VISIMPL_LINK_LIBRARIES zeq zeqHBP ${CMAKE_THREAD_LIBS_INIT})
endif()

if (GMRVZEQ_FOUND)
  list( APPEND VISIMPL_LINK_LIBRARIES gmrvzeq )
endif()

if (DEFLECT_FOUND)
  list(APPEND VISIMPL_LINK_LIBRARIES Deflect)
endif()

if (BRION_FOUND)
  list(APPEND VISIMPL_LINK_LIBRARIES Brion Brain)
endif()

if (NEUROLOTS_FOUND)
  list(APPEND VISIMPL_LINK_LIBRARIES nlrender nlgenerator nlfem )
endif()

common_application( visimpl )
