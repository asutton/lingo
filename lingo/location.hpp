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

  explicit operator bool() const { return (*this == none); }

  bool operator==(Location l) const { return loc_ == l.loc_; }
  bool operator!=(Location l) const { return loc_ != l.loc_; }

private:
  int loc_;
};


// A Bound location associates a location reference with its
// originating buffer. Bound locations are returned from
// the location() method of a buffer.
struct Bound_location
{
  Bound_location(Buffer const& b, Location l)
    : buf_(b), loc_(l)
  { }
  
  bool is_valid() const { return loc_ != Location::none; }
  bool is_file_location() const;

  Buffer const& buffer() const { return buf_; }
  
  File const& file() const;
  Line const& line() const;
  
  int line_no() const;
  int column_no() const;

  Buffer const&  buf_;
  Location       loc_;
};


std::ostream& operator<<(std::ostream&, Bound_location const&);


} // namespace lingo


#endif
