// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#ifndef LINGO_LOCATION_HPP
#define LINGO_LOCATION_HPP

// The location module provides facilities for representing
// locations in source code.

#include "lingo/file.hpp"

#include <cstdint>
#include <iosfwd>

namespace lingo
{

// A source code location is a 32-bit integer that encodes the
// file and offset of a source code location. The first 12 bytes
// indicate the file index in the file manager, and the remaining
// 20 give its offset in the source file.
//
// NOTE: Ths places a hard limit of 20MB source files.
struct Location_id
{
  int file   : 12;
  int offset : 20;

  bool operator==(Location_id id)  const
  { 
    return file == id.file && offset == id.offset; 
  }

  bool operator!=(Location_id id) const
  {
    return !(*this == id);
  }
};


// The location provides an interface to a location data. Locations 
// can be contextually converted to bool, to determine whether or 
// not they are "pinned" to an underlying source location.
class Location
{
public:
  // Indicates a program element that does not originate within
  // a source file.
  static Location none;

  Location()
    : loc_{0, 0}
  { }

  Location(File const*, int);

  explicit operator bool() const { return (*this == none); }

  File const& file() const;
  Line const& line() const;
  int         line_number() const;
  int         column_number() const;

  bool operator==(Location l) const { return loc_ == l.loc_; }
  bool operator!=(Location l) const { return loc_ != l.loc_; }

private:
  Location_id loc_;
};


// Return the line number of the current location.
inline int
Location::line_number() const
{
  return line().number();
}


std::ostream& operator<<(std::ostream&, Location);


} // namespace lingo


#endif
