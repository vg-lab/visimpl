# ParticleLab tests

set(TEST_LIBRARIES
        VisimplTesting
        ${EXTERNAL_LIBS_DEPENDENCIES})

add_executable(test_scoop_color color.cpp)
target_link_libraries(test_scoop_color ${TEST_LIBRARIES})
add_test(NAME test_scoop_color COMMAND test_scoop_color)

add_executable(test_scoop_color_palette color_palette.cpp)
target_link_libraries(test_scoop_color_palette ${TEST_LIBRARIES})
add_test(NAME test_scoop_color_palette COMMAND test_scoop_color_palette)