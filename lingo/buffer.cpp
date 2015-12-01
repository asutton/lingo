// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "buffer.hpp"
#include "error.hpp"

#include <iostream>

namespace lingo
{

// Initialize the buffer with the given text. Perform a cursory
// analysis of the input in order to construct the line map
// for the input source.
Buffer::Buffer(String const& str)
  : text_(str)
{
  char const *first = &text_.front();
  char const *last = first + text_.size();
  char const *iter = first;
  int pos = 0;
  while (iter != last) {
    if (*iter == '\n') {
      insert({pos, Line(size() + 1, pos, first + pos, iter)});
      pos = iter - first + 1; // Starts past the newline.
    }
    ++iter;
  }
  insert({pos, Line(size() + 1, pos, first + pos, iter)});
}


} // namespace