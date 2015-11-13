// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "lingo/io.hpp"

#include <iostream>

using namespace lingo;


int 
main()
{
  init_colors();
  std::cout << bright_red("error") << ": this is a test" << '\n';
  std::cout << bright_magenta("warning") << ": this is a test" << '\n';
  
  std::cout << bright_cyan("note") << ": this is a test"<< '\n';
  std::cout << cyan("note") << ": this is a test" << '\n';
  std::cout << dim_cyan("note") << ": this is a test" << '\n';
}