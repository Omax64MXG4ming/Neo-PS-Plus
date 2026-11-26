cmake_minimum_required(VERSION 3.21)

project(Template VERSION 1.0.0)

# --------------------------------------------------
# C++23
# --------------------------------------------------

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --------------------------------------------------
# Architectures
# --------------------------------------------------

if ("${CMAKE_SYSTEM_NAME}" STREQUAL "iOS" OR IOS)
    set(CMAKE_OSX_ARCHITECTURES "arm64")
elseif (APPLE)
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
endif()

# --------------------------------------------------
# Compiler configuration
# --------------------------------------------------

set(CMAKE_CXX_VISIBILITY_PRESET hidden)

set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
set(CMAKE_CXX_MODULE_STD OFF)

cmake_policy(SET CMP0069 NEW)

# --------------------------------------------------
# Geode SDK
# --------------------------------------------------

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR
        "Unable to find Geode SDK! Please define GEODE_SDK."
    )
endif()

message(STATUS "Found Geode: $ENV{GEODE_SDK}")

add_subdirectory(
    $ENV{GEODE_SDK}
    ${CMAKE_CURRENT_BINARY_DIR}/geode
)

# --------------------------------------------------
# Dependencies
# --------------------------------------------------

include(FetchContent)

set(BOX2D_BUILD_TESTBED OFF CACHE BOOL "" FORCE)
set(BOX2D_BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(BOX2D_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    box2d
    GIT_REPOSITORY https://github.com/erincatto/box2d.git
    GIT_TAG v2.4.1
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(box2d)

CPMAddPackage("gh:xblazegmd/xblaze-geode-api2#b7c7689")

# --------------------------------------------------
# Source files
# --------------------------------------------------

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp src/Hooks/*.cpp)
add_library(${PROJECT_NAME} SHARED ${SOURCES})
# --------------------------------------------------
# Include directories
# --------------------------------------------------

target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${box2d_SOURCE_DIR}/include"
)

# --------------------------------------------------
# Link libraries
# --------------------------------------------------

target_link_libraries(${PROJECT_NAME}
    box2d
    xblaze-geode-api
)
# --------------------------------------------------
# Configure Geode mod
# --------------------------------------------------

setup_geode_mod(${PROJECT_NAME})

# --------------------------------------------------
# Build information
# --------------------------------------------------

message(STATUS "Neo PS LauncherCore sources:")

foreach(SOURCE ${SOURCES})
    message(STATUS "  ${SOURCE}")
endforeach()
