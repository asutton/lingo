# Copyright (c) 2015 Andrew Sutton
# All rights reserved

# Bypass the usual cppformat build and build the
# source into this tree.
set(contrib_src ${PROJECT_SOURCE_DIR}/cppformat/format.cc)

# Add the Lingo library.
add_library(lingo
  ${contrib_src}
  format.cpp
  assert.cpp
  string.cpp
  io.cpp
  node.cpp
  memory.cpp
  integer.cpp
  utility.cpp
  line.cpp
  location.cpp
  buffer.cpp
  file.cpp
  error.cpp
  print.cpp
  debug.cpp
  character.cpp
  symbol.cpp
  token.cpp)
target_include_directories(
  lingo
    PUBLIC
      "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR};${PROJECT_SOURCE_DIR}/cppformat>"
      ${GMP_INCLUDE_DIRS}
      ${Boost_INCLUDE_DIRS}
)
target_link_libraries(
  lingo
    PUBLIC
      ${GMP_LIBRARIES}
      ${Boost_LIBRARIES}
)
