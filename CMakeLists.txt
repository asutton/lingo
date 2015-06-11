# Copyright (c) 2015 Andrew Sutton
# All rights reserved

# Project configuration.
cmake_minimum_required(VERSION 3.0)
project(lingo CXX)
enable_testing()


# Compiler configuration
set(CMAKE_CXX_FLAGS "-Wall -std=c++1y")


# GMP Dependencies
find_path(GMP_INCLUDE_DIR NAMES gmp.h)
find_library(GMP_LIBRARIES NAMES gmp libgmp)
MESSAGE(STATUS "GMP libs: " ${GMP_LIBRARIES})


# Boost dependencies
find_package(Boost 1.55.0 REQUIRED
  COMPONENTS system filesystem)



# Build configuration.
include_directories(. ${GMP_INCLUDE_DIR} ${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

add_subdirectory(lingo)
add_subdirectory(test)
add_subdirectory(examples)
