// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "config.hpp"

#include "lingo/buffer.hpp"
#include "lingo/error.hpp"

#include <iostream>

namespace lingo
{

// Initialize the buffer with the given text. Perform a cursory
// analysis of the input in order to construct the line map
// for the input source.
Buffer::Buffer(String const& str)
  : text_(str), lines_()
{
  char const *first = &text_.front();
  char const *last = first + text_.size();
  char const *iter = first;
  int pos = 0;
  int line = 0;
  while (iter != last) {
    if (*iter == '\n') {
      lines_.emplace(pos, Line(++line, pos, first + pos, iter));
      pos = iter - first + 1; // One past the newline.
    }
    ++iter;
  }
  lines_.emplace(pos, Line(++line, pos, first + pos, iter));
}


} // namespace lingo
