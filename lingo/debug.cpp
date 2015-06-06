// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "lingo/debug.hpp"

#include <iostream>

namespace lingo
{

// Returns the default debugging output stream.
std::ostream&
default_debug_stream()
{
  return std::cout;
}


} // namespace lingo
