cmake_minimum_required(VERSION 3.20)
project(MultiversalConsciousness VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

if(EMSCRIPTEN)
    set(SDL3_PATH "C:/Users/thism/Emscripten/Release")
    find_package(SDL3 REQUIRED PATHS "${SDL3_PATH}" NO_DEFAULT_PATH)
    find_package(SDL3_ttf REQUIRED PATHS "${SDL3_PATH}" NO_DEFAULT_PATH)
    find_package(SDL3_image REQUIRED PATHS "${SDL3_PATH}" NO_DEFAULT_PATH)
else()
    find_package(SDL3 QUIET)
    if(NOT SDL3_FOUND)
        message(STATUS "SDL3 not found. Please install SDL3 development libraries.")
        message(STATUS "Windows: Download SDL3 development libraries from https://github.com/libsdl-org/SDL/releases")
        message(STATUS "Linux: sudo apt-get install libsdl3-dev (when available) or build from source")
        message(STATUS "macOS: brew install sdl3 (when available) or build from source")
        message(FATAL_ERROR "SDL3 is required to build this project")
    endif()

    find_package(SDL3_ttf QUIET)
    if(NOT SDL3_ttf_FOUND)
        message(STATUS "SDL3_ttf not found. Please install SDL3_ttf development libraries.")
        message(STATUS "Windows: Download SDL3_ttf development libraries from https://github.com/libsdl-org/SDL_ttf/releases")
        message(STATUS "Linux: sudo apt-get install libsdl3-ttf-dev (when available) or build from source")
        message(STATUS "macOS: brew install sdl3_ttf (when available) or build from source")
        message(FATAL_ERROR "SDL3_ttf is required for text rendering")
    endif()

    find_package(SDL3_image QUIET)
    if(NOT SDL3_image_FOUND)
        message(STATUS "SDL3_image not found. Please install SDL3_image development libraries.")
        message(STATUS "Windows: Download SDL3_image development libraries from https://github.com/libsdl-org/SDL_image/releases")
        message(STATUS "Linux: sudo apt-get install libsdl3-image-dev (when available) or build from source")
        message(STATUS "macOS: brew install sdl3_image (when available) or build from source")
        message(FATAL_ERROR "SDL3_image is required for image loading")
    endif()
endif()

include_directories(src)

set(SOURCES
    src/main.cpp
    src/engine/GameEngine.cpp
    src/engine/SDLDeleter.cpp
    src/engine/EntityManager.cpp
    src/engine/System.cpp
    src/engine/TileRenderer.cpp
    src/engine/RealityManager.cpp
    src/engine/RealitySystem.cpp
    src/engine/PossessionSystem.cpp
    src/engine/CameraController.cpp
    src/engine/AgentRenderer.cpp
    src/engine/QuantumSystem.cpp
    src/engine/QuantumNodeRenderer.cpp
    src/engine/InputManager.cpp
    src/engine/MovementSystem.cpp
    src/engine/CoordinationSystem.cpp
    src/engine/PuzzleSystem.cpp
    src/engine/LevelLoader.cpp
    src/engine/AbilitySystem.cpp
    src/engine/PhysicsSystem.cpp
    src/engine/QuantumLoadoutSystem.cpp
    src/engine/InteractiveObstacles.cpp
    src/engine/InteractiveObstacleSystem.cpp
    src/engine/HUDSystem.cpp
    src/engine/VerificationSystem.cpp
    src/engine/MenuSystem.cpp
    src/engine/BackgroundSystem.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} SDL3::SDL3 SDL3_ttf::SDL3_ttf SDL3_image::SDL3_image)

if(EMSCRIPTEN)
    message(STATUS "Configuring Emscripten-specific flags for ${PROJECT_NAME}")
    target_link_options(${PROJECT_NAME} PRIVATE
        "SHELL:--preload-file ${CMAKE_SOURCE_DIR}/assets@assets"
        "SHELL:--preload-file ${CMAKE_SOURCE_DIR}/levels@levels"
        "SHELL:--shell-file ${CMAKE_SOURCE_DIR}/shell_minimal.html"
        "SHELL:-s USE_SDL=0" 
        "SHELL:-s USE_ZLIB=1"
        "SHELL:-s ALLOW_MEMORY_GROWTH=1"
        "SHELL:-s NO_EXIT_RUNTIME=1"
        "SHELL:-s EXPORTED_RUNTIME_METHODS=['ccall','cwrap']"
    )
    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".html")
endif()
if(NOT EMSCRIPTEN)
enable_testing()

include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.4.0
)
FetchContent_MakeAvailable(Catch2)

set(TEST_SOURCES
    tests/test_main.cpp
    src/main.cpp
    src/engine/GameEngine.cpp
    src/engine/SDLDeleter.cpp
    src/engine/EntityManager.cpp
    src/engine/System.cpp
    src/engine/TileRenderer.cpp
    src/engine/RealityManager.cpp
    src/engine/RealitySystem.cpp
    src/engine/PossessionSystem.cpp
    src/engine/CameraController.cpp
    src/engine/AgentRenderer.cpp
    src/engine/QuantumSystem.cpp
    src/engine/QuantumNodeRenderer.cpp
    src/engine/InputManager.cpp
    src/engine/MovementSystem.cpp
    src/engine/CoordinationSystem.cpp
    src/engine/PuzzleSystem.cpp
    src/engine/LevelLoader.cpp
    src/engine/AbilitySystem.cpp
    src/engine/PhysicsSystem.cpp
    src/engine/QuantumLoadoutSystem.cpp
    src/engine/InteractiveObstacles.cpp
    src/engine/InteractiveObstacleSystem.cpp
    src/engine/HUDSystem.cpp
    src/engine/VerificationSystem.cpp
    src/engine/MenuSystem.cpp
    src/engine/BackgroundSystem.cpp
)

add_executable(tests ${TEST_SOURCES})
target_link_libraries(tests Catch2::Catch2WithMain SDL3::SDL3 SDL3_ttf::SDL3_ttf SDL3_image::SDL3_image)
target_include_directories(tests PRIVATE src)

# Copy levels and assets directories to build directory for runtime access
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/levels ${CMAKE_BINARY_DIR}/levels
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets ${CMAKE_BINARY_DIR}/assets
    COMMENT "Copying levels and assets directories to build directory"
)

endif() # NOT EMSCRIPTEN