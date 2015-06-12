// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#ifndef LINGO_BUFFER_HPP
#define LINGO_BUFFER_HPP

#include "lingo/string.hpp"
#include "lingo/location.hpp"

#include <map>

namespace lingo
{

// -------------------------------------------------------------------------- //
//                                Lines

// A line of text is the sequence of characters from column
// 0 up to its newline.
//
// The members of this class are public to allow them to be
// updated by a character stream during processing.
class Line
{
public:
  Line(int n, int c, char const* f, char const* l)
    : num(n), off(c), first(f), last(l)
  { }

  int number() const { return num; }
  int offset() const { return off; }

  String_view rep() const { return {first, last}; }

  char const* begin() const { return first; }
  char const* end() const   { return last; }

  int         num;    // Tjhe line number
  int         off;    // The character offset
  char const* first;
  char const* last;
};


// A line map associates an offset in the source code with
// it's underlying line of text.
struct Line_map : std::map<int, Line>
{
  // Observers
  Line const& line(Location) const;
  int line_no(Location) const;
  int column_no(Location) const;

  // Mutators
  Line& start_line(int, char const*);
  void  finish_line(Line&, char const*);
};



// -------------------------------------------------------------------------- //
//                                Buffers


// A buffer is a region of text containing code. This class
// contains the underlying character sequence and an interface
// for resolving source code locations.
//
// The buffer class is the base class for different kinds of
// memory buffers. In particular, a file is kind of buffer
// that is read from disk.
//
// TODO: Provide better integration with I/O streams, etc. to
// avoid unnecessary copies.
class Buffer : private Line_map
{
public:
  Buffer()
    : text_()
  { }

  Buffer(String const& str)
    : text_(str)
  { }

  virtual ~Buffer() { }

  // Lines
  using Line_map::start_line;
  using Line_map::finish_line;
  using Line_map::line;
  using Line_map::line_no;
  using Line_map::column_no;

  // Returns a bound location for the offset. Behavior is
  // undefined if `loc` does not represent a location in
  // this buffer.
  Bound_location location(Location loc) const { return {*this,loc}; }

  // Iterators
  char const* begin() const { return text_.c_str(); }
  char const* end() const   { return begin() + text_.size(); }

protected:
  String   text_;
};


} // namespace

#endif
