// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#ifndef LINGO_LOCATION_HPP
#define LINGO_LOCATION_HPP

// The location module provides facilities for representing
// locations in source code.

#include <iosfwd>

namespace lingo
{

class Buffer;
class File;
class Line;

// The location class is an opaque reference to a location within
// a source code buffer. The interpretation of a location object
// requires the bufer from which it is created.
//
// Internally, this is simply the offset of a character within
// the buffer. The buffer is responsible for associating 
class Location
{
public:
  // Indicates a program element that does not originate within
  // a source file.
  static Location none;

  Location()
    : loc_(-1)
  { }

  explicit Location(int n)
    : loc_(n)
  { }

  // Returns the offset into a buffer.
  int offset() const { return loc_; }

  explicit operator bool() const { return (*this != none); }

  bool operator==(Location l) const { return loc_ == l.loc_; }
  bool operator!=(Location l) const { return loc_ != l.loc_; }

private:
  int loc_;
};


// A span of text is represented by a pair of source 
// locations. Like a location, a text span must be
// resolved against its input buffer.
//
// The start location shall be less than or equal to
// the end location. A span is never empty.
struct Span
{
  Span(Location s, Location e)
    : start_(s), end_(e)
  { }

  Location start() const { return start_; }
  Location end() const { return end_; }

  // Contextually converts to true iff both the start and
  // end are valid offsets.
  explicit operator bool() const { return start_ && end_; }

  Location start_;
  Location end_;
};


// A Bound location associates a location reference with its
// originating buffer. Bound locations are returned from
// the location() method of a buffer.
//
// FIXME: Don't call this "bound"
struct Bound_location
{
  Bound_location(Buffer const& b, Location l)
    : buf_(&b), loc_(l)
  { }
  
  bool is_valid() const { return (bool)loc_; }
  bool is_file_location() const;

  Buffer const& buffer() const { return *buf_; }
  File const& file() const;
  Line const& line() const;
  
  int line_no() const;
  int column_no() const;

  Buffer const*  buf_;
  Location      loc_;
};


// A Bound span associates a location reference with its
// originating buffer. 
//
// TODO: Support line iterators?
struct Bound_span
{
  Bound_span(Buffer const& b, Span s)
    : buf_(&b), span_(s)
  { }
  
  bool is_valid() const     { return (bool)span_; }
  bool is_multiline() const { return start_line_no() != end_line_no(); }
  bool is_file_location() const;

  Buffer const& buffer() const { return *buf_; }
  File const& file() const;
  Line const& line() const;
  
  int start_line_no() const;
  int end_line_no() const;
  int start_column_no() const;
  int end_column_no() const;

  Buffer const* buf_;
  Span          span_;
};


std::ostream& operator<<(std::ostream&, Bound_location const&);
std::ostream& operator<<(std::ostream&, Bound_span const&);


} // namespace lingo


#endif
