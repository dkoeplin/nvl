cmake_minimum_required(VERSION 3.29)
project(nvl)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
message(STATUS "CMAKE_MODULE_PATH: ${CMAKE_MODULE_PATH}")
set(CMAKE_CXX_FLAGS " \
-O3 \
-Wall \
-Wextra \
-Wpedantic \
-Wshadow \
-Werror \
")
# Disabled for now - not found significantly faster, and potential issues with using +/-INF
# -ffast-math \
# -Wno-nan-infinity-disabled \

add_library(nvl SHARED
        nvl/actor/Actor.h
        nvl/actor/Part.h
        nvl/actor/Status.cpp
        nvl/actor/Status.h
        nvl/data/Counter.h
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
        nvl/data/Tensor.cpp
        nvl/data/Tensor.h
        nvl/data/UnionFind.h
        nvl/data/WalkResult.h
        nvl/entity/Block.h
        nvl/entity/Entity.h
        nvl/file/Lines.cpp
        nvl/file/Lines.h
        nvl/geo/BRTree.h
        nvl/geo/Dir.h
        nvl/geo/Face.h
        nvl/geo/HasBBox.h
        nvl/geo/Intersect.h
        nvl/geo/Line.h
        nvl/geo/Orthants.h
        nvl/geo/Polar.h
        nvl/geo/RBox.h
        nvl/geo/Rel.h
        nvl/geo/RTree.h
        nvl/geo/Triangle.h
        nvl/geo/Tuple.h
        nvl/geo/Volume.h
        nvl/io/HasPrint.h
        nvl/io/IO.h
        nvl/macros/Abstract.h
        nvl/macros/Aliases.h
        nvl/macros/Assert.h
        nvl/macros/Expand.h
        nvl/macros/Hot.h
        nvl/macros/Implicit.h
        nvl/macros/Pure.h
        nvl/macros/ReturnIf.h
        nvl/macros/SIMD.h
        nvl/macros/Unreachable.h
        nvl/macros/Unroll.h
        nvl/material/Bulwark.h
        nvl/material/Material.h
        nvl/material/TestMaterial.h
        nvl/math/Bitwise.cpp
        nvl/math/Bitwise.h
        nvl/math/Combinations.h
        nvl/math/Deg.cpp
        nvl/math/Deg.h
        nvl/math/Distribution.h
        nvl/math/Grid.h
        nvl/math/Plane.h
        nvl/math/Random.h
        nvl/math/Rotation.h
        nvl/math/Trig.h
        nvl/message/Created.h
        nvl/message/Destroy.h
        nvl/message/Hit.h
        nvl/message/Message.cpp
        nvl/message/Message.h
        nvl/message/Notify.h
        nvl/reflect/Backtrace.cpp
        nvl/reflect/Backtrace.h
        nvl/reflect/CastablePtr.h
        nvl/reflect/CastableShared.h
        nvl/reflect/Casting.h
        nvl/reflect/ClassTag.cpp
        nvl/reflect/ClassTag.h
        nvl/reflect/PrimitiveTypes.h
        nvl/time/Clock.h
        nvl/time/Duration.cpp
        nvl/time/Duration.h
        nvl/time/TimeScale.h
        nvl/ui/Color.cpp
        nvl/ui/Color.h
        nvl/ui/GlowEffect.h
        nvl/ui/InputEvent.h
        nvl/ui/Key.cpp
        nvl/ui/Key.h
        nvl/ui/Mouse.cpp
        nvl/ui/Mouse.h
        nvl/ui/RayWindow.cpp
        nvl/ui/RayWindow.h
        nvl/ui/Screen.cpp
        nvl/ui/Screen.h
        nvl/ui/ViewOffset.cpp
        nvl/ui/ViewOffset.h
        nvl/ui/Window.cpp
        nvl/ui/Window.h
        nvl/world/World.h
)

set(RAYLIB_VERSION 5.5)
find_package(raylib ${RAYLIB_VERSION} REQUIRED)

target_include_directories(nvl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(nvl PRIVATE raylib)

if (APPLE)
    target_link_libraries(nvl PRIVATE "-framework IOKit")
    target_link_libraries(nvl PRIVATE "-framework Cocoa")
    target_link_libraries(nvl PRIVATE "-framework OpenGL")
endif ()

find_package(GoogleTest REQUIRED)

# nvl-test
add_library(nvl-test SHARED
        nvl/test/Expect.h
        nvl/test/Fuzzing.h
        nvl/test/LabeledBox.h
        nvl/test/NullWindow.h
        nvl/test/TensorWindow.cpp
        nvl/test/TensorWindow.h
)
target_include_directories(nvl-test PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(nvl-test PUBLIC nvl gmock gtest_main)

add_subdirectory(app)
add_subdirectory(test)
