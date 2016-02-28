set(SUMRICE_PUBLIC_HEADERS
  EditorTF/Gradient.h
  EditorTF/ColorPoints.h
  EditorTF/TransferFunctionEditor.h
  Summary.h
  qt/CustomSlider.h
)

set(SUMRICE_HEADERS
)

set(SUMRICE_SOURCES
  EditorTF/Gradient.cpp
  EditorTF/ColorPoints.cpp
  EditorTF/TransferFunctionEditor.cpp
  Summary.cpp
)

set(SUMRICE_MOC_HEADERS
  EditorTF/ColorPoints.h
  EditorTF/TransferFunctionEditor.h
  Summary.h
)

set(SUMRICE_LINK_LIBRARIES
  Qt5::Core
  Qt5::Widgets
  Brion
  prefr
)

set(SUMRICE_INCLUDE_NAME sumrice)
set(SUMRICE_NAMESPACE sumrice)
add_definitions(-DVISIMPL_SHARED)
common_library( sumrice )
