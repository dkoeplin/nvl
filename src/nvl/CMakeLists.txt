
add_library(nvl SHARED
        nvl/data/SipHash.cpp
)

target_include_directories(nvl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
