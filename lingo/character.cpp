// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "config.hpp"

#include "lingo/character.hpp"
#include "lingo/error.hpp"

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
  if (eof())
    return 0;
  else
    return *first_;
}


// Returns the nth character past the current position.
// If the nth character is past the end of the file, then
// this returns the null character.
char
Character_stream::peek(int n) const
{
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
  if (eof())
    return 0;
  else
    return *first_++;
}


} // namespace lingo
