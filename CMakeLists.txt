cmake_minimum_required(VERSION 3.30.5)

project(AOC2025)

set(CMAKE_CXX_COMPILER g++-15)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include)

set(JANKY_VIM_LINTING_FLAGS "-Wno-pragma-once-outside-header")
set(CMAKE_CXX_FLAGS_DEBUG "${JANKY_VIM_LINTING_FLAGS} -Wall -Wextra -Wconversion -pedantic -O0 -ggdb -fpic -fconcepts-diagnostics-depth=2")
set(CMAKE_CXX_FLAGS_RELEASE "-Werror -DNDEBUG -march=native -Wall -Wextra -pedantic -Wconversion -O3 -fpic")

# Default compile features
set(DEFAULT_COMPILE_FEATURES cxx_std_23)

add_subdirectory(day1)
