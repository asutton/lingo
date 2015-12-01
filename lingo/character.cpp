// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "character.hpp"
#include "error.hpp"

// TODO: Let L be the number of lines in a file. The current
// implementation maintains a mapping of lines using an 
// sorted structure to support line lookup of offsets. However,
// we don't actually need to maintain that mapping until we
// start issuing lookups against it. 
//
// We could run a translation phase that builds the line map
// prior to whatever phase is happening here.

namespace lingo
{

// Returns a reference to the current character. Note
// that the stream must not be at the end of the file.
char
Character_stream::peek() const
{
  lingo_assert(!eof());
  return *first_;
}


// Returns the nth caracter past the curent position.
// If the nth caracter is past the end of the file, then
// this returns the null character.
char
Character_stream::peek(int n) const
{
  lingo_assert(!eof());
  if (n >= (last_ - first_))
    return 0;
  return *(first_ + n);
}


// Returns a pointer to the current element and advances
// the stream position to the next character. This also
// checks to see if we've advanced over a newline and
// updates the source location as needed.
char
Character_stream::get()
{
  lingo_assert(!eof());
  return *first_++;
}


} // namespace lingo
