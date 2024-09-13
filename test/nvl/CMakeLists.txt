function(add_gtest test_name test_file)
    add_executable(${test_name} ${test_file})
    target_link_libraries(${test_name} PRIVATE nvl gmock gtest_main)
    gtest_discover_tests(${test_name})
endfunction()

add_subdirectory(data)
add_subdirectory(geo)
