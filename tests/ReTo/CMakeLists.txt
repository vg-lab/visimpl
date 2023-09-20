# ReTo tests

configure_file(reto_test_data.in ${CMAKE_CURRENT_BINARY_DIR}/reto_test_data.h)

if (NOT Boost_FOUND)
    return()
endif ()

set(TEST_LIBRARIES
        VisimplTesting
        ${EXTERNAL_LIBS_DEPENDENCIES})

add_executable(test_reto_camera camera.cpp)
include_directories(test_reto_camera PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(test_reto_camera ${TEST_LIBRARIES})
add_test(NAME test_reto_camera COMMAND test_reto_camera)

add_executable(test_reto_model model.cpp)
include_directories(test_reto_model PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(test_reto_model ${TEST_LIBRARIES})
add_test(NAME test_reto_model COMMAND test_reto_model)

add_executable(test_reto_spline spline.cpp)
include_directories(test_reto_spline PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(test_reto_spline ${TEST_LIBRARIES})
add_test(NAME test_reto_spline COMMAND test_reto_spline)

add_executable(test_reto_shader_program shader_program.cpp)
include_directories(test_reto_shader_program PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(test_reto_shader_program ${TEST_LIBRARIES})
add_test(NAME test_reto_shader_program COMMAND test_reto_shader_program)

