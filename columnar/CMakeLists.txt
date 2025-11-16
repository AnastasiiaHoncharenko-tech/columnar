cmake_minimum_required(VERSION 3.20)
cmake_policy(SET CMP0135 NEW)

project(Columnar
        VERSION 1.0.0
        DESCRIPTION "Modern C++ columnar data library"
        LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(columnar INTERFACE)
target_include_directories(columnar INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

if(MSVC)
    target_compile_options(columnar INTERFACE /W4)
else()
    target_compile_options(columnar INTERFACE -Wall -Wextra -Wpedantic)
endif()

option(BUILD_TESTS "Build unit tests" ON)
option(BUILD_EXAMPLES "Build example programs" ON)

include(FetchContent)

set(CMAKE_CXX_STANDARD 20 CACHE STRING "C++ standard" FORCE)
set(CMAKE_CXX_STANDARD_REQUIRED ON CACHE BOOL "Require C++ standard" FORCE)
set(CMAKE_CXX_EXTENSIONS OFF CACHE BOOL "Disable compiler extensions" FORCE)

FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
)
set(gtest_build_tests OFF CACHE BOOL "")
set(gtest_build_samples OFF CACHE BOOL "")
FetchContent_MakeAvailable(googletest)

if(TARGET gtest)
    set_target_properties(gtest PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
    )
endif()
if(TARGET gtest_main)
    set_target_properties(gtest_main PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
    )
endif()

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
