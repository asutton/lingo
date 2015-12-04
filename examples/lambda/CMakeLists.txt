# Copyright (c) 2015 Andrew Sutton
# All rights reserved


add_executable(lambda
  main.cpp
  ast.cpp
  lexer.cpp
  parser.cpp
  evaluator.cpp
  substitution.cpp)
target_link_libraries(lambda lingo)
