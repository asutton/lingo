// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#ifndef LINGO_CHARACTER_HPP
#define LINGO_CHARACTER_HPP

#include <lingo/location.hpp>
#include <lingo/buffer.hpp>
#include <lingo/string.hpp>

namespace lingo
{

class Buffer;


// A character stream provides a sequence of characters and has
// a very simple streaming interface consisting of only 5 functions:
// peek(), get(), and eof(), begin(), and end(). Character streams
// are the input to lexical analyzers.
//
// There are also two support functions: null() returns a non-character
// value (i.e., the null character), and location() returns the source
// location for a character in the stream.
//
// Note that as a general rule for streams, &s.peek() == s.begin().
//
// Hypothetically, the null() function is a mechanism for creating
// a value that contextually evaluates to false upon default construction.
// This is a stronger concept than the NullablePointer concept.
class Character_stream
{
public:
  Character_stream(Buffer& b)
    : buf_(b), base_(b.begin()), first_(base_), last_(b.end())
  { }

  // Stream control
  bool eof() const     { return first_ == last_; }
  char peek() const;
  char peek(int) const;
  char get();
  void ignore()       { get(); }

  // Locations
  int      offset() const   { return first_ - base_; }
  Location location() const { return Location(&buf_, offset()); }

  // Buffer
  Buffer const& buffer() const { return buf_; }

  // Iterators
  char const* begin() const { return first_; }
  char const* end() const { return last_; }

private:
  Buffer&       buf_;  // The stream's source file
  char const*   base_;  // The beginning of the stream
  char const*   first_; // Current character pointer
  char const*   last_;  // Past the end of the character buffer
};

} // namespace lingo


#endif
