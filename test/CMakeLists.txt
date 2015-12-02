# Copyright (c) 2015 Andrew Sutton
# All rights reserved


macro(add_test_program target)
  add_executable(${target} ${ARGN})
  target_link_libraries(${target} lingo)
endmacro()


add_test_program(test-font font.cpp)
