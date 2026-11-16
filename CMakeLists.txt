cmake_minimum_required(VERSION 3.28)
project(ParticleSimulation LANGUAGES CXX)

# Default to an optimized build unless another build type is chosen.
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# ---- SFML 2.6 (fetched automatically at configure time) ----
# Only Graphics (with Window and System) is used, so skip Audio and Network.
set(SFML_BUILD_AUDIO OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 2.6.2
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM
)
FetchContent_MakeAvailable(SFML)

# ---- One executable per preset in main/ ----
# in and out are the Bad Apple pipeline: slow, memory-heavy, and they need
# extra input files, so they're only built when asked for.
option(BUILD_BAD_APPLE "Also build the in/out Bad Apple presets" OFF)

set(PRESETS main_original main_new galtonboard breakout1 breakout2 course trail)
if(BUILD_BAD_APPLE)
    list(APPEND PRESETS in out)
endif()

foreach(preset ${PRESETS})
    add_executable(${preset} main/${preset}.cpp)
    target_compile_features(${preset} PRIVATE cxx_std_17)
    target_link_libraries(${preset} PRIVATE sfml-graphics)

    # Lets the presets include "solvers/...", "thread.hpp", etc. from the repo root.
    target_include_directories(${preset} PRIVATE ${CMAKE_SOURCE_DIR})

    # Absolute path to assets/, so the program finds its font and texture
    # no matter which folder it's launched from.
    target_compile_definitions(${preset} PRIVATE ASSET_DIR="${CMAKE_SOURCE_DIR}/assets/")

    if(MSVC)
        # M_PI needs _USE_MATH_DEFINES on MSVC; /permissive- accepts `and`/`or`.
        target_compile_definitions(${preset} PRIVATE _USE_MATH_DEFINES)
        target_compile_options(${preset} PRIVATE /permissive-)
    endif()
endforeach()

