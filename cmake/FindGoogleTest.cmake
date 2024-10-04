if (NOT _GOOGLETEST_FOUND)
    set(_GOOGLETEST_FOUND TRUE)

    include(FetchContent)
    FetchContent_Declare(
            googletest
            URL https://github.com/google/googletest/archive/0953a17a4281fc26831da647ad3fcd5e21e6473b.zip
    )
    set(gtest_force_shared_crt ON CACHE BOOL " " FORCE)
    FetchContent_MakeAvailable(googletest)
    include(GoogleTest)
endif ()
