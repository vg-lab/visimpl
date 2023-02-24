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
  CorrelationComputer.h
  Utils.h
  LoaderThread.h
  LoadingDialog.h
  DataInspector.h
  DisplayManagerWidget.h
  StackViz.h
  ConfigureRESTDialog.h
  ConnectRESTDialog.h
  CloseDataDialog.h
  ColorInterpolator.h
  ReconnectRESTDialog.h
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
  CorrelationComputer.cpp
  Utils.cpp
  LoaderThread.cpp
  LoadingDialog.cpp
  DataInspector.cpp
  DisplayManagerWidget.cpp
  StackViz.cpp
  ConfigureRESTDialog.cpp
  ConnectRESTDialog.cpp
  CloseDataDialog.cpp
  ColorInterpolator.cpp
  ReconnectRESTDialog.cpp
)

set(SUMRICE_LINK_LIBRARIES
  Qt5::Core
  Qt5::Widgets
  ReTo
  SimIL
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

if (OPENMP_FOUND)
  list(APPEND SUMRICE_LINK_LIBRARIES OpenMP::OpenMP_CXX)
endif()

set(SUMRICE_INCLUDE_NAME sumrice)
set(SUMRICE_NAMESPACE sumrice)
add_definitions(-DVISIMPL_SHARED)
common_library( sumrice )
