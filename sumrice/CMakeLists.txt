set(CMAKE_AUTOMOC ON)

set(SUMRICE_PUBLIC_HEADERS
  EditorTF/Gradient.h
  EditorTF/ColorPoints.h
  EditorTF/TransferFunctionEditor.h
  SimulationPlayer.h
  ZeqEventsManager.h
  Summary.h
  Histogram.h
  CustomSlider.h
  TransferFunctionWidget.h
  ParticleSizeWidget.h
  log.h
)

set(SUMRICE_HEADERS
)

set(SUMRICE_SOURCES
  EditorTF/Gradient.cpp
  EditorTF/ColorPoints.cpp
  EditorTF/TransferFunctionEditor.cpp
  TransferFunctionWidget.cpp
  ParticleSizeWidget.cpp
  Summary.cpp
  Histogram.cpp
  SimulationPlayer.cpp
  ZeqEventsManager.cpp
)

#set(SUMRICE_MOC_HEADERS
#  EditorTF/ColorPoints.h
#  EditorTF/TransferFunctionEditor.h
#  TransferFunctionWidget.h
#  ParticleSizeWidget.h
#  Summary.h
#)

set(SUMRICE_LINK_LIBRARIES
  Qt5::Core
  Qt5::Widgets
  Brion
  prefr
)

if (ZEQ_FOUND)
  list(APPEND SUMRICE_LINK_LIBRARIES zeq zeqHBP ${CMAKE_THREAD_LIBS_INIT})
endif()

if (GMRVZEQ_FOUND)
  list( APPEND SUMRICE_LINK_LIBRARIES gmrvzeq )
endif()

set(SUMRICE_INCLUDE_NAME sumrice)
set(SUMRICE_NAMESPACE sumrice)
add_definitions(-DVISIMPL_SHARED)
common_library( sumrice )
