set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Core)
find_package(HDF5 REQUIRED COMPONENTS C CXX)
INCLUDE_DIRECTORIES (${HDF5_INCLUDE_DIR})

set(SOURCES 
	gdf2csv.cpp
	txt2csv.cpp
	hdf52csv.cpp
	hdf52hdf5.cpp
	)

set(HEADERS
	)

# not used, only to annotate dependencies.
set(EXTERNAL_LIBS
	Qt5::Core
	${HDF5_LIBRARIES}
	)

add_executable(gdf2csv gdf2csv.cpp)
add_executable(txt2csv txt2csv.cpp)
add_executable(hdf52csv hdf52csv.cpp)
add_executable(hdf52hdf5 hdf52hdf5.cpp)
target_link_libraries(gdf2csv Qt5::Core)
target_link_libraries(txt2csv Qt5::Core)
target_link_libraries(hdf52csv ${HDF5_LIBRARIES})
target_link_libraries(hdf52hdf5 ${HDF5_LIBRARIES})

