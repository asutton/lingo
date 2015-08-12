// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#ifndef LINGO_CHARACTER_HPP
#define LINGO_CHARACTER_HPP

#include "lingo/location.hpp"
#include "lingo/buffer.hpp"
#include "lingo/string.hpp"

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
  using value_type = char;

  Character_stream(Buffer& b, char const* f, char const* l)
    : buf_(b), base_(f), first_(f), last_(l)
  { }

  Character_stream(Buffer& b, String const& s)
    : Character_stream(b, s.data(), s.data() + s.size())
  { }

  Character_stream(Buffer& b)
    : Character_stream(b, b.begin(), b.end())
  { }

  // Stream control
  bool        eof() const     { return first_ == last_; }
  char const& peek() const;
  char        peek(int) const;
  char const& get();

  // Iterators
  char const* begin()       { return first_; }
  char const* begin() const { return first_; }
  char const* end()       { return last_; }
  char const* end() const { return last_; }

  // Locations
  Location location() const { return Location(offset()); }

  // Buffer
  Buffer const& buffer() const { return buf_; }

private:
  int offset() const { return first_ - base_; }
  
  Buffer&       buf_;  // The stream's source file
  char const*   base_;  // The beginning of the stream
  char const*   first_; // Current character pointer
  char const*   last_;  // Past the end of the character buffer
};

} // namespace lingo


#endif
