
add_executable(a0 ${CMAKE_CURRENT_SOURCE_DIR}/a0/A0.cpp)
target_link_libraries(a0 PRIVATE nvl)
target_include_directories(a0 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(a1 ${CMAKE_CURRENT_SOURCE_DIR}/a1/A1.cpp)
target_link_libraries(a1 PRIVATE nvl)
target_include_directories(a1 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(a2-lib SHARED
        a2/entity/Player.cpp
        a2/entity/Player.h
        a2/action/Brake.cpp
        a2/action/Brake.h
        a2/action/Dig.cpp
        a2/action/Dig.h
        a2/action/Jump.cpp
        a2/action/Jump.h
        a2/action/Move.cpp
        a2/action/Move.h
        a2/action/PlayerAction.h
        a2/action/Strafe.cpp
        a2/action/Strafe.h
        a2/action/Teleport.cpp
        a2/action/Teleport.h
        a2/ui/DeathScreen.cpp
        a2/ui/DeathScreen.h
        a2/ui/DebugScreen.cpp
        a2/ui/DebugScreen.h
        a2/ui/PauseScreen.cpp
        a2/ui/PauseScreen.h
        a2/ui/PlayerControls.cpp
        a2/ui/PlayerControls.h
        a2/world/WorldA2.cpp
        a2/world/WorldA2.h
)
target_link_libraries(a2-lib PUBLIC nvl)
target_include_directories(a2-lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(a2 ${CMAKE_CURRENT_SOURCE_DIR}/a2/A2.cpp)
target_link_libraries(a2 PRIVATE a2-lib)
