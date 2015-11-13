# Copyright (c) 2015 Andrew Sutton
# All rights reserved

cmake_minimum_required(VERSION 3.0)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Project configuration
project(lingo CXX)
enable_testing()

# Compiler configuration
set(CMAKE_CXX_FLAGS "-Wall -std=c++1y")

# GMP dependencies
find_package(GMP REQUIRED)

# Boost dependencies
find_package(Boost 1.55.0 REQUIRED COMPONENTS system filesystem)

add_subdirectory(lingo)
add_subdirectory(test)
add_subdirectory(examples)
