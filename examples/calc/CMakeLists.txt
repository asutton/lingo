# Copyright (c) 2015 Andrew Sutton
# All rights reserved


add_executable(calc 
  main.cpp 
  ast.cpp
  lexer.cpp
  parser.cpp)
target_link_libraries(calc lingo)
