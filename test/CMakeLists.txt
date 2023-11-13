#Include gtest
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.11.0
)
FetchContent_MakeAvailable(googletest)
add_library(GTest::GTest INTERFACE IMPORTED)
target_link_libraries(GTest::GTest INTERFACE gtest_main)


# Map test names to test source files
set(tests
    server_test:server_test.cpp
    response_schema_factory_test:response_schema_factory_test.cpp
    tokens_test:tokens_test.cpp
)

# Create a test out of each element in "tests"
foreach(test_info IN LISTS tests) 
    string(REPLACE ":" ";" test_info_list ${test_info}) 
    list(GET test_info_list 0 test_name)
    list(GET test_info_list 1 source_file)

    add_executable(${test_name} ${source_file})
    target_link_libraries(${test_name} PRIVATE GTest::gtest echoserver)
    add_test(${test_name}_gtest ${test_name})
endforeach()