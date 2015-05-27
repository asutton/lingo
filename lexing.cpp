// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "lingo/lexing.hpp"

namespace lingo
{

// Returns a reference to the current character. Note
// that the stream must not be at the end of the file.
char const&
Character_stream::peek() const
{
  lingo_assert(!eof());
  return *first;
}


// Returns the nth caracter past the curent position.
// If the nth caracter is past the end of the file, then
// this returns the null character.
char
Character_stream::peek(int n) const
{
  lingo_assert(!eof());
  if (n >= (last - first))
    return 0;
  return *(first + n);
}


// Returns a pointer to the current element and advances
// the stream position to the next character. This also
// checks to see if we've advanced over a newline and
// updates the source location as needed.
char const&
Character_stream::get()
{
  lingo_assert(!eof());
  char const*c = first;
  if (*c == '\n') {
    ++loc.line;
    loc.col = 1;
  } else {
    ++loc.col;
  }
  ++first;
  return *c;
}


} // namespace lingo
