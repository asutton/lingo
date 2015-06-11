
# Bypass the usual cppformat build and build the
# source into this tree.
set(contrib_src
  ${CMAKE_SOURCE_DIR}/cppformat/format.cc
)


# The lingo library
add_library(lingo
  ${contrib_src}
  node.cpp
  memory.cpp
  string.cpp
  integer.cpp
  location.cpp
  file.cpp
  character.cpp
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
target_link_libraries(lingo ${GMP_LIBRARIES} ${Boost_LIBRARIES})
