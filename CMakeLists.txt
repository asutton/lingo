# Copyright (c) 2015 Andrew Sutton
# All rights reserved

cmake_minimum_required(VERSION 3.0)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(CheckIncludeFileCXX)


# Project configuration
project(lingo CXX)
enable_testing()


# Compiler configuration
set(CMAKE_CXX_FLAGS "-Wall -std=c++1y")


# GMP dependencies
find_package(GMP REQUIRED)


# Boost dependencies
find_package(Boost 1.55.0 REQUIRED COMPONENTS system filesystem)


# LLVM dependencies
find_package(LLVM 3.6 REQUIRED CONFIG)
llvm_map_components_to_libnames(LLVM_LIBRARIES core)
include_directories(${LLVM_INCLUDE_DIRS})


# Other dependencies
check_include_file_cxx(unistd.h HAVE_UNISTD_H)
if(NOT HAVE_UNISTD_H)
  message(FATAL_ERROR "${PROJECT_NAME} requires the POSIX C header <unistd.h>.")
endif()


if(NOT TARGET check)
  add_custom_target(check COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target test)
endif()


add_subdirectory(lingo)
add_subdirectory(test EXCLUDE_FROM_ALL)
add_subdirectory(examples EXCLUDE_FROM_ALL)
