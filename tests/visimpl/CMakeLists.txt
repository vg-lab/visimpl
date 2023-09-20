# ViSimpl tests

set(TEST_LIBRARIES
        VisimplTesting
        ${EXTERNAL_LIBS_DEPENDENCIES})

add_executable(test_visimpl_domain_manager domain_manager.cpp)
target_link_libraries(test_visimpl_domain_manager ${TEST_LIBRARIES})
add_test(NAME test_visimpl_domain_manager COMMAND test_visimpl_domain_manager)