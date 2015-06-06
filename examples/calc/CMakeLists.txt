

add_executable(calc 
  main.cpp 
  ast.cpp
  lexer.cpp
  parser.cpp)
target_link_libraries(calc lingo)
