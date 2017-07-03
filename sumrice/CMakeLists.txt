# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
#   ViSimpl
#   2015-2016 (c) ViSimpl / Universidad Rey Juan Carlos
#   sergio.galindo@urjc.es
#
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

set(CMAKE_AUTOMOC ON)

set(SUMRICE_PUBLIC_HEADERS
  EditorTF/Gradient.h
  EditorTF/ColorPoints.h
  EditorTF/TransferFunctionEditor.h  
  Summary.h
  Histogram.h
  FocusFrame.h
  CustomSlider.h
  TransferFunctionWidget.h
  log.h
  EventWidget.h  
)

set(SUMRICE_HEADERS
)

set(SUMRICE_SOURCES
  EditorTF/Gradient.cpp
  EditorTF/ColorPoints.cpp
  EditorTF/TransferFunctionEditor.cpp
  TransferFunctionWidget.cpp
  Summary.cpp
  Histogram.cpp
  FocusFrame.cpp
  EventWidget.cpp
)

set(SUMRICE_LINK_LIBRARIES
  Qt5::Core
  Qt5::Widgets
  ReTo
  SimIL
  prefr
  scoop
)

if (ZEROEQ_FOUND)
  list(APPEND SUMRICE_LINK_LIBRARIES ZeroEQ ${CMAKE_THREAD_LIBS_INIT})
endif()

if (GMRVLEX_FOUND)
  list( APPEND SUMRICE_LINK_LIBRARIES gmrvlex )
endif()

if (LEXIS_FOUND)
  list( APPEND SUMRICE_LINK_LIBRARIES Lexis )
endif()

set(SUMRICE_INCLUDE_NAME sumrice)
set(SUMRICE_NAMESPACE sumrice)
add_definitions(-DVISIMPL_SHARED)
common_library( sumrice )


