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

  // Find the line whose starting offset is greater than that
  // of the location. The location's line number is the
  // previous location.
  auto iter = --upper_bound(loc.offset());
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


namespace
{

Buffer*  buf_ = nullptr;
Location loc_;


} // namespace


// Returns the current input buffer. This is null if
// there is no current input buffer.
Buffer&
input_buffer()
{
  return *buf_;
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
  buf_ = &buf;
}


// Set the current input location.
void
set_input_location(Location loc)
{
  loc_ = loc;
}


// Update the current source location.
Input_context::Input_context(Location loc)
  : saved_buf(buf_), saved_loc(loc_)
{ 
  loc_ = loc;
}


// Update the current input buffer. Note that the source
// location is set to "none".
//
// TODO: Set the source location to something meaningful?
Input_context::Input_context(Buffer& buf)
  : saved_buf(buf_), saved_loc(loc_)
{
  buf_ = &buf;
  loc_ = Location::none;
}


// Update the current input context to the given buffer
// and source location.
Input_context::Input_context(Buffer& buf, Location loc)
  : saved_buf(buf_), saved_loc(loc_)
{
  buf_ = &buf;
  loc_ = loc;
}


// Restore the input context on scope exit.
Input_context::~Input_context()
{
  buf_ = saved_buf;
  loc_ = saved_loc;
}


} // namespace