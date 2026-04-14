cmake_minimum_required(VERSION 3.20)
project(QuantumBifurcation VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")


if(DEFINED ENV{SDL3_DIR} AND NOT CMAKE_PREFIX_PATH)
    set(CMAKE_PREFIX_PATH "$ENV{SDL3_DIR}")
    message(STATUS "Using SDL3_DIR from environment variable: $ENV{SDL3_DIR}")
endif()

find_package(SDL3 QUIET)

if(NOT SDL3_FOUND)
    message(STATUS "SDL3 not found.")
    message(STATUS "If you have installed SDL3 in a custom location, please do one of the following:")
    message(STATUS "1. Set the SDL3_DIR environment variable to your SDL3 installation directory.")
    message(STATUS "2. Pass the path to cmake with: cmake -DCMAKE_PREFIX_PATH=/path/to/sdl3 ..")
    message(STATUS "")
    message(STATUS "For installation instructions, please see SETUP.md.")
    message(FATAL_ERROR "SDL3 is required to build this project.")
endif()

message(STATUS "Found SDL3: ${SDL3_LIBRARIES}")

include_directories(src)

set(SOURCES
    src/main.cpp
    src/engine/GameEngine.cpp
    src/engine/SDLDeleter.cpp
    src/engine/EntityManager.cpp
    src/engine/System.cpp
    src/engine/TileRenderer.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3)
