cmake_minimum_required(VERSION 4.2)
project(RavensNestVTT)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(ThirdParty/SDL-release-3.4.8/)
add_subdirectory(ThirdParty/SDL_net-main/)

IF(WIN32)
    add_custom_command(TARGET SDL3-shared POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            "${CMAKE_BINARY_DIR}/ThirdParty/SDL-release-3.4.8/SDL3.dll"
            "${CMAKE_BINARY_DIR}/SDL3.dll"
            COMMENT "Copying sdl3 dll to bin dir"
            VERBATIM
    )

    add_custom_command(TARGET SDL3_net::SDL3_net-shared POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            "${CMAKE_BINARY_DIR}/ThirdParty/SDL_net-main/SDL3_net.dll"
            "${CMAKE_BINARY_DIR}/SDL3_net.dll"
            COMMENT "Copying sdl3 net dll to bin dir"
            VERBATIM
    )
ENDIF()


add_library(NestVTTLib STATIC
    src/VTTLib/Logging/Logger.cpp
    src/VTTLib/Clock/AppClock.cpp
    src/VTTLib/Net/Socket.cpp
    src/Entrypoints/main.cpp
)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(NestVTTLib PRIVATE CORE_DEBUG)
endif ()

target_include_directories(NestVTTLib PUBLIC
    src/
    ThirdParty/SDL-release-3.4.8/include/
    ThirdParty/SDL_net-main/include/
)

target_link_libraries(NestVTTLib PRIVATE
    SDL3_net::SDL3_net
    SDL3::SDL3
)

add_executable(Client
    src/Implementations/ClientApp.cpp
)

target_link_libraries(Client PRIVATE
    NestVTTLib
)

add_executable(Server
    src/Implementations/ServerApp.cpp
)

target_link_libraries(Server PRIVATE
    NestVTTLib
)