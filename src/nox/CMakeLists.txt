
add_library(nox SHARED
        nox/nox.cpp
        nox/data/SipHash.cpp
)

target_include_directories(nox PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
