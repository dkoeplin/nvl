cmake_minimum_required(VERSION 3.29)
project(nvl)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS " \
-Ofast \
-Wall \
-Wextra \
-Wpedantic \
-Wshadow \
-Werror \
"
)

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/0953a17a4281fc26831da647ad3fcd5e21e6473b.zip
)
set(gtest_force_shared_crt ON CACHE BOOL " " FORCE)
FetchContent_MakeAvailable(googletest)
include(GoogleTest)

add_library(nvl SHARED
        nvl/data/SipHash.cpp
)

target_include_directories(nvl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(test)
