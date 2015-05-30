# Copyright (c) 2015 Andrew Sutton
# All rights reserved

# Project configuration.
cmake_minimum_required(VERSION 3.0)
project(lingo CXX)


# Compiler configuration
set(CMAKE_CXX_FLAGS "-Wall -std=c++1y")


# GMP Dependencies
find_path(GMP_INCLUDE_DIR NAMES gmp.h )
find_library(GMP_LIBRARIES NAMES gmp libgmp )
MESSAGE(STATUS "GMP libs: " ${GMP_LIBRARIES})



# Build configuration.
include_directories(.. ${GMP_INCLUDE_DIR})


# Bypass the usual cppformat build and build the
# source into this tree.
set(contrib_src
  cppformat/format.cc
)

# The lingo library
add_library(lingo
  ${contrib_src}
  node.cpp
  memory.cpp
  string.cpp
  integer.cpp
  location.cpp
  error.cpp
  print.cpp
  debug.cpp
  symbol.cpp
  token.cpp
  algorithm.cpp
  lexing.cpp
  parsing.cpp
  json.cpp
  cli.cpp)
target_link_libraries(lingo ${GMP_LIBRARIES})


add_subdirectory(test)
