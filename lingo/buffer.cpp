// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "buffer.hpp"
#include "error.hpp"

#include <iostream>

namespace lingo
{

// Start a new line, having the give offset and starting pointer.
Line&
Line_map::start_line(int off, char const* ptr)
{
  auto ins = insert({off, Line(size() + 1, off, ptr, nullptr)});
  return ins.first->second;
}


// Terminate the line so that it ends with the given character.
void
Line_map::finish_line(Line& line, char const* ptr)
{
  line.last = ptr;
}


// Returns the line number for the character offset. 
Line const&
Line_map::line(Location loc) const
{
  lingo_assert(!empty());
  auto iter = lower_bound(loc.offset());
  if (iter == end())
    --iter;
  return iter->second;
}


// Returns the line number for the given location.
int 
Line_map::line_no(Location loc) const
{
  return line(loc).number();
}


// Returns the column number for the given location.
int
Line_map::column_no(Location loc) const
{
  return loc.offset() - line(loc).offset() + 1;
}

namespace
{

Buffer* input_ = nullptr;
Location loc_;

} // namespace


// Returns the current input buffer. This is null if
// there is no current input buffer.
Buffer&
input_buffer()
{
  return *input_;
}


/* Returns the current input location. */
Location
input_location()
{
  return loc_;
}


// Set the current input buffer. The previous buffer
// is not saved. See the Input_guard class.
void
set_input_buffer(Buffer& buf)
{
  input_ = &buf;
}


// Set the current input location.
void
set_input_location(Location loc)
{
  loc_ = loc;
}


} // namespace