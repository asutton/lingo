// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "lingo/string.hpp"

#include <iostream>

namespace lingo
{

// Streaming.
std::ostream&
operator<<(std::ostream& os, String_view s)
{
  for (char c : s)
    os << c;
  return os;
}


// Returns the a hash value for the characters in the symbol.
// Adapted from an algorithm in Stroustrup's book (not sure
// which, found the reference on StackOverflow).
std::size_t
String_view::hash() const
{
  std::size_t h = 0;
  char const* p = first;
  while (p != last)
    h = h << 1 ^ *p++;
  return h;
}


} // namespace lingo
