# Copyright (c) 2015 Andrew Sutton
# All rights reserved

cmake_minimum_required(VERSION 3.0)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(CheckIncludeFileCXX)
include(TestBigEndian)

# Project configuration
project(lingo VERSION 0.0 LANGUAGES C CXX)
enable_testing()

set(PACKAGE_NAME ${PROJECT_NAME})
set(PACKAGE_TARNAME ${PACKAGE_NAME})
set(PACKAGE_VERSION ${PROJECT_VERSION})
set(PACKAGE_STRING "${PACKAGE_NAME} ${PACKAGE_VERSION}")
set(PACKAGE_BUGREPORT "https://github.com/asutton/lingo/issues")
set(PACKAGE_URL "https://github.com/asutton/lingo")


# Run platform checks.
test_big_endian(WORDS_BIGENDIAN)


# Compiler configuration
set(CMAKE_CXX_FLAGS "-Wall -std=c++1y")


# GMP dependencies
find_package(GMP REQUIRED)


# Boost dependencies
find_package(Boost 1.55.0 REQUIRED COMPONENTS system filesystem)


# LLVM dependencies
find_package(LLVM 3.6 REQUIRED CONFIG)
llvm_map_components_to_libnames(LLVM_LIBRARIES core)


# Other dependencies
find_package(iconv REQUIRED)


check_include_file_cxx(unistd.h HAVE_UNISTD_H)
if(NOT HAVE_UNISTD_H)
  message(FATAL_ERROR "${PROJECT_NAME} requires the POSIX C header <unistd.h>.")
endif()


# Generate the configuration header.
configure_file(config.hpp.in config.hpp)


if(NOT TARGET check)
  add_custom_target(check COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target test)
endif()


add_subdirectory(lingo)
add_subdirectory(test EXCLUDE_FROM_ALL)
add_subdirectory(examples EXCLUDE_FROM_ALL)
