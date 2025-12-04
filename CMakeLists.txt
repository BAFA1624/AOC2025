cmake_minimum_required(VERSION 3.30.5)

project(AOC2025)

set(CMAKE_CXX_COMPILER g++-15)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include)

set(JANKY_VIM_LINTING_FLAGS "-Wno-pragma-once-outside-header")

set(GENERAL_FLAGS "-Wall -Wextra -pedantic -Wconversion -fpic -Wno-comma-subscript")
set(CMAKE_CXX_FLAGS_DEBUG "${JANKY_VIM_LINTING_FLAGS} ${GENERAL_FLAGS} -O0 -ggdb -fpic -fconcepts-diagnostics-depth=2")
set(CMAKE_CXX_FLAGS_RELEASE "-Werror -DNDEBUG -march=native ${GENERAL_FLAGS} -O3 -fpic")

# Default compile features
set(DEFAULT_COMPILE_FEATURES cxx_std_23)

# Capture all "day" folders
file(GLOB DAY_DIRS RELATIVE ${CMAKE_SOURCE_DIR} "${CMAKE_SOURCE_DIR}/day*")
foreach(dir ${DAY_DIRS})
    if (IS_DIRECTORY ${CMAKE_SOURCE_DIR}/${dir})
        add_subdirectory(${dir})
    endif()
endforeach()
