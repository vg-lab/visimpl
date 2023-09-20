
find_package(Qt5 REQUIRED COMPONENTS Core Gui Network Widgets Test)

enable_testing()

set(VISIMPL_DIR ${CMAKE_SOURCE_DIR}/visimpl)
set(STACKVIZ_DIR ${CMAKE_SOURCE_DIR}/stackviz)
set(SUMRICE_DIR ${CMAKE_SOURCE_DIR}/sumrice)
set(RETO_DIR ${CMAKE_SOURCE_DIR}/ReTo)
set(SIMIL_DIR ${CMAKE_SOURCE_DIR}/SimIL)
set(PLAB_DIR ${CMAKE_SOURCE_DIR}/plab)
set(SCOOP_DIR ${CMAKE_SOURCE_DIR}/scoop)

include_directories(
        .
        ${CMAKE_BINARY_DIR}
        ${CMAKE_BINARY_DIR}/include
        ${CMAKE_BINARY_DIR}/visimpl/include
        ${CMAKE_BINARY_DIR}/stackviz/include
        ${CMAKE_BINARY_DIR}/sumrice/include
        ${CMAKE_BINARY_DIR}/ReTo/include
        ${CMAKE_BINARY_DIR}/SimIL/include
        ${CMAKE_BINARY_DIR}/plab/include
        ${CMAKE_BINARY_DIR}/scoop/include
        ${CMAKE_BINARY_DIR}/acuterecorder/include
        ${CMAKE_BINARY_DIR}/Brion/include
        ${CMAKE_BINARY_DIR}/gmrvlex/include
        ${CMAKE_BINARY_DIR}/Lexis/include
        ${CMAKE_BINARY_DIR}/Servus/include
        ${CMAKE_BINARY_DIR}/ZeroEQ/include
        ${CMAKE_BINARY_DIR}/ZeroBuf/include
        ${CMAKE_BINARY_DIR}/vmmlib/include

        ${CMAKE_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/visimpl/
        ${CMAKE_SOURCE_DIR}/stackviz/
        ${CMAKE_SOURCE_DIR}/sumrice/
        ${CMAKE_SOURCE_DIR}/ReTo/
        ${CMAKE_SOURCE_DIR}/SimIL/
        ${CMAKE_SOURCE_DIR}/plab/
        ${CMAKE_SOURCE_DIR}/scoop/
        ${CMAKE_SOURCE_DIR}/acuterecorder/
        ${CMAKE_SOURCE_DIR}/Brion/
        ${CMAKE_SOURCE_DIR}/gmrvlex/
        ${CMAKE_SOURCE_DIR}/Lexis/
        ${CMAKE_SOURCE_DIR}/Servus/
        ${CMAKE_SOURCE_DIR}/ZeroEQ/
        ${CMAKE_SOURCE_DIR}/ZeroBuf/
        ${CMAKE_SOURCE_DIR}/vmmlib
)

# Core
qt5_add_resources(TEST_RESOURCES
        # Qt resources if needed
        )
qt5_wrap_cpp(TEST_MOCS
        # Qt classes MOCs if needed for testing.
        )

common_find_package(Boost COMPONENTS system filesystem unit_test_framework REQUIRED)

if (NOT Boost_FOUND)
    return()
endif ()

set(EXTERNAL_LIBS_DEPENDENCIES
        ${QT_LIBRARIES}
        Qt5::Test
        ReTo
        SimIL
        plab
        sumrice
        scoop
        visimpl_test_lib
        ${Boost_FILESYSTEM_LIBRARY}
        ${Boost_SYSTEM_LIBRARY}
        ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})

set(DEPENDENCIES_SOURCES
        visimpl_test_utils.h
        visimpl_test_utils.cpp)

add_library(VisimplTesting SHARED ${DEPENDENCIES_SOURCES})
target_link_libraries(VisimplTesting ${EXTERNAL_LIBS_DEPENDENCIES})

common_find_package_post()

add_definitions(-DBOOST_TEST_DYN_LINK)

if (VISIMPL_BUILD_TESTS)

    add_subdirectory(visimpl)
    add_subdirectory(sumrice)
    #  add_subdirectory(stackviz)
    add_subdirectory(ReTo)
    #  add_subdirectory(SimIL)
    add_subdirectory(plab)
    add_subdirectory(scoop)

endif (VISIMPL_BUILD_TESTS)
