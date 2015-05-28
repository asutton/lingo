// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "lingo/json.hpp"

#include <iostream>
#include <fstream>
#include <iterator>

using namespace lingo;

using Iter = std::istreambuf_iterator<char>;

int 
main(int argc, char* argv[])
{
  if (argc != 2) {
    std::cerr << "no input files given\n";
    return -1;
  } 

  // Read the contents into a string.
  std::string text;
  if (argv[1] == std::string("-")) {
    text.insert(text.end(), Iter{std::cin}, Iter{});
  } else {
    std::ifstream ifs(argv[1]);
    if (!ifs) {
      std::cerr << "could not open file '" << argv[1] << "'\n";
      return -1;
    } 
    text.insert(text.end(), Iter{ifs}, Iter{});
  }

  // Parse the text.
  if (json::Value* value = json::parse(text))
    print(value);
}
