# Copyright (c) 2015 Andrew Sutton
# All rights reserved

macro(add_test_program target)
  add_executable(${target} ${ARGN})
  add_dependencies(check ${target})
endmacro()

link_libraries(lingo)

add_test_program(test-font font.cpp)
add_test_program(test-string string.cpp)
add_test_program(test-unicode unicode.cpp)
