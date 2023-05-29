file(GLOB_RECURSE TEST_SOURCES "${PROJECT_SOURCE_DIR}/test/*_test.cpp")

set(PROJECT_LIB ${PROJECT_NAME}_lib)

include(GoogleTest)

##########################################
# "make XYZ_test"
##########################################
foreach (test_source ${TEST_SOURCES})
    # Create a human readable name.
    get_filename_component(test_filename ${test_source} NAME)
    string(REPLACE ".cpp" "" test_name ${test_filename})

    # Add the test target separately and as part of "make check-tests".
    # we can exclude from all by add EXCLUDE_FROM_ALL
    add_executable(${test_name} ${test_source})
    # add_dependencies(build-tests ${bustub_test_name})
    # add_dependencies(check-tests ${bustub_test_name})

    target_link_libraries(${test_name} GTest::gtest_main ${PROJECT_LIB})

    # Set test target properties and dependencies.
    set_target_properties(${test_name}
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/test"
    )
    gtest_discover_tests(${test_name}
                        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/test")
endforeach()