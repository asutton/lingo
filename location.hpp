// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#ifndef LINGO_LOCATION_HPP
#define LINGO_LOCATION_HPP

// The location module provides facilities for representing
// locations in source code.

#include <iosfwd>

namespace lingo
{

struct Location;


// FIXME: Add legitimate support for files.
struct File { };


// The Location_data class represents a location is a source
// file. Note that source locations are maintained privately
// by this module. The primary interface is the Location
// class below.
struct Location_data
{
  Location_data()
    : Location_data(nullptr, 1, 1)
  { }

  Location_data(File* f, int l, int c)
    : file(f), line(l), col(c)
  { }

  Location save() const;

  File* file;
  int line;
  int col;
};


// The location provides an interface to a location data. Locations 
// can be contextually converted to bool, to determine whether or 
// not they are "pinned" to an underlying source location.
struct Location
{
  // Indicates a program element that does not originate within
  // a source file.
  static Location none;

  // The location is in a command line argument.
  static Location cli;

  Location()
    : Location(nullptr)
  { }

  Location(Location_data const* l)
    : loc_(l)
  { }

  explicit operator bool() const { return loc_; }

  File const* file() const { return loc_->file; }
  int         line() const { return loc_->line; }
  int         column() const { return loc_->col; }
  
  Location_data const* loc_;
};


// Equality comparison.
inline bool
operator==(Location a, Location b)
{
  return a.loc_ == b.loc_;
}


inline bool
operator!=(Location a, Location b)
{
  return a.loc_ == b.loc_;
}


std::ostream& operator<<(std::ostream&, Location);


} // namespace lingo


#endif
