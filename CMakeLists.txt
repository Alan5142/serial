cmake_minimum_required(VERSION 3.16)
project(serial)

set(CMAKE_CXX_STANDARD 14)

# Third party libraries
add_subdirectory(third_party)

set(SOURCES src/serial.cpp)

if (WIN32)
    set(SOURCES ${SOURCES} src/serial.win.cpp)
endif ()

# Should we build a static library or a shared library?

option(BUILD_AS_SHARED "Build \"serial\" as a shared library" OFF)

if (BUILD_AS_SHARED)
    set(BUILD_TYPE SHARED)
else ()
    set(BUILD_TYPE STATIC)
endif ()

add_library(serial ${BUILD_TYPE} ${SOURCES})
target_include_directories(serial PUBLIC include)
