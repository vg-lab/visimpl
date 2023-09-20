# ParticleLab tests

set(TEST_LIBRARIES
        VisimplTesting
        ${EXTERNAL_LIBS_DEPENDENCIES})

add_executable(test_plab_cluster cluster.cpp)
target_link_libraries(test_plab_cluster ${TEST_LIBRARIES})
add_test(NAME test_plab_cluster COMMAND test_plab_cluster)