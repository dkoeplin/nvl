enable_testing()

set(TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Custom target to run all gtests added in this project at once. Can be used as a CLion run target.
add_custom_target(all-gtests COMMAND ${CMAKE_CTEST_COMMAND} USES_TERMINAL)

# Hack placeholder to give Clion a target executable to run when executing the above custom target.
add_executable(test-end ${CMAKE_CURRENT_SOURCE_DIR}/util/Test.cpp)

find_package(GoogleTest REQUIRED)

function(add_gtest test_file)
    get_filename_component(test_name "${test_file}" NAME_WE)
    add_executable(${test_name} ${test_file})
    target_link_libraries(${test_name} PRIVATE nvl gmock gtest_main)
    target_include_directories(${test_name} PRIVATE ${TEST_DIR})
    gtest_discover_tests(${test_name})
    add_dependencies(all-gtests ${test_name})
endfunction()

add_subdirectory(actor)
add_subdirectory(data)
add_subdirectory(entity)
add_subdirectory(geo)
add_subdirectory(math)
add_subdirectory(reflect)
add_subdirectory(world)
