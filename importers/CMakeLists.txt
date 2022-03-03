set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Core)

set(SOURCES 
	gdftocsv.cpp
	)

set(HEADERS
	)

set(EXTERNAL_LIBS
	Qt5::Core
	)

add_executable(gdf2csv gdf2csv.cpp)
target_link_libraries(gdf2csv ${EXTERNAL_LIBS})

