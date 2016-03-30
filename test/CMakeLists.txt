# Copyright (c) 2015 Andrew Sutton
# All rights reserved

macro(add_test_program test_name target_executable)
  add_executable(${target_executable} ${ARGN})
  add_dependencies(check ${target_executable})
  add_test(NAME ${test_name} COMMAND ${target_executable})
endmacro()

link_libraries(lingo)

add_test_program(font test_font font.cpp)
add_test_program(string test_string string.cpp)
add_test_program(unicode test_unicode unicode.cpp)
add_test_program(character_set_conversion test_character_set_conversion character_set_conversion.cpp)
