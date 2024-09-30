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
        nvl/actor/Actor.h
        nvl/actor/Part.h
        nvl/actor/Status.h
        nvl/data/HasEquality.h
        nvl/data/Iterator.h
        nvl/data/List.h
        nvl/data/Map.h
        nvl/data/Maybe.h
        nvl/data/Once.h
        nvl/data/PointerHash.h
        nvl/data/Range.h
        nvl/data/Ref.h
        nvl/data/Set.h
        nvl/data/SipHash.cpp
        nvl/data/SipHash.h
        nvl/data/UnionFind.h
        nvl/draw/Color.h
        nvl/draw/Draw.h
        nvl/entity/Entity.h
        nvl/geo/At.h
        nvl/geo/Box.h
        nvl/geo/BRTree.h
        nvl/geo/Dir.h
        nvl/geo/HasBBox.h
        nvl/geo/Pos.h
        nvl/geo/RTree.h
        nvl/io/IO.h
        nvl/macros/Abstract.h
        nvl/macros/Aliases.h
        nvl/macros/Assert.h
        nvl/macros/Implicit.h
        nvl/macros/Expand.h
        nvl/macros/Pure.h
        nvl/macros/ReturnIf.h
        nvl/macros/Unreachable.h
        nvl/material/Material.h
        nvl/math/Bitwise.h
        nvl/math/Distribution.h
        nvl/math/Grid.h
        nvl/math/Random.h
        nvl/message/Destroy.h
        nvl/message/Hit.h
        nvl/message/Message.h
        nvl/message/Notify.h
        nvl/reflect/Castable.h
        nvl/reflect/Casting.h
        nvl/reflect/ClassTag.cpp
        nvl/reflect/ClassTag.h
        nvl/reflect/PrimitiveTypes.h
        nvl/time/Duration.cpp
        nvl/time/Duration.h
        nvl/time/TimeScale.h
        nvl/world/World.h
)

target_include_directories(nvl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(test)
