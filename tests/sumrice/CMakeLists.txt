# ParticleLab tests

set(TEST_LIBRARIES
        VisimplTesting
        ${EXTERNAL_LIBS_DEPENDENCIES})

add_executable(test_sumrice_color_interpolator color_interpolator.cpp)
target_link_libraries(test_sumrice_color_interpolator ${TEST_LIBRARIES})
add_test(NAME test_sumrice_color_interpolator COMMAND test_sumrice_color_interpolator)