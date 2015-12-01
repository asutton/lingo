
# Bypass the usual cppformat build and build the
# source into this tree.
set(contrib_src
  ${PROJECT_SOURCE_DIR}/cppformat/format.cc
)



# The lingo library
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
target_link_libraries(lingo ${GMP_LIBRARIES} ${Boost_LIBRARIES})
