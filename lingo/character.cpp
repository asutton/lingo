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
char const&
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
char const&
Character_stream::get()
{
  lingo_assert(!eof());

  char const*c = first_;

  // Move to the next character.
  ++first_;

  // If the consumed character was a new line, then
  // update the line map.
  if (*c == '\n')
    line_ = new_line();

  // If we just hit the end of the line, then terminate
  // the last line in the file.
  //
  // If the last character was a newline, then the line
  // map will have a 0-character last line. This is 
  // correct.
  if (first_ == last_)
    line_->last = last_;

  return *c;
}


// Create a new line starting at the current position.
//
// TODO: These streams modify the underlying file be memoizing
// lines. What happens if we copy the character stream. There
// should only be one line map.
Line*
Character_stream::start_line(int num)
{
  Line line(num, offset(), first_, nullptr);
  auto ins = file_->lines().insert({offset(), line});
  return &ins.first->second;
}


Line*
Character_stream::new_line()
{
  // Terminate the previous line so that the newline is
  // not included in the string.
  line_->last = first_;

  // And start the new line.
  return start_line(line_->number() + 1);
}


} // namespace lingo
