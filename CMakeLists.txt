cmake_minimum_required(VERSION 4.2)
project(RavensNestVTT)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(ThirdParty/SDL-release-3.4.8/)
add_subdirectory(ThirdParty/SDL_net-main/)

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

set_target_properties(NestVTTLib PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_target_properties(SDL3-shared PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_target_properties(SDL3_net-shared PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_target_properties(Server PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_target_properties(Client PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
