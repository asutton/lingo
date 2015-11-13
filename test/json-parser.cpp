// Copyright (c) 2015 Andrew Sutton
// All rights reserved

// This program accepts JSON wrapped as a command line argument.
// Note that the JSON value must be enclosed in quotes. Example:
//
//    json-parser '{"a":1, "b":2}'
//
// If the input is well-formed, it will be pretty printed.

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
    std::cerr << "usage: json-parser '<json-value>'\n";
    return -1;
  }

  // Parse the text.
  if (json::Value* value = json::parse(argv[1]))
    print(value);
}
