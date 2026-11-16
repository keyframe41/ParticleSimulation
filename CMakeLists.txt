cmake_minimum_required(VERSION 3.28)
project(ParticleSimulation LANGUAGES CXX)

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

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
# media_generator for Bad Apple pipeline, which is optional as it's gonna be slow
option(BUILD_MEDIA_GENERATOR "Also build the media_generator program" OFF)

set(PRESETS main_original main_new galtonboard breakout1 breakout2 course trail)
if(BUILD_MEDIA_GENERATOR)
    list(APPEND PRESETS media_generator)
endif()

foreach(preset ${PRESETS})
    add_executable(${preset} main/${preset}.cpp)
    target_compile_features(${preset} PRIVATE cxx_std_17)
    target_link_libraries(${preset} PRIVATE sfml-graphics)

    target_include_directories(${preset} PRIVATE ${CMAKE_SOURCE_DIR})

    target_compile_definitions(${preset} PRIVATE ASSET_DIR="${CMAKE_SOURCE_DIR}/assets/")

    if(MSVC)
        target_compile_definitions(${preset} PRIVATE _USE_MATH_DEFINES)
        target_compile_options(${preset} PRIVATE /permissive-)
    endif()
endforeach()
