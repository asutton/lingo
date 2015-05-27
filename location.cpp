// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "lingo/location.hpp"

#include <iostream>
#include <set>

namespace lingo
{

namespace
{

// FIXME: This is terribly inefficient. Design a more efficient
// data structure for maintaining source locations. Also, don't
// compare pointers.

struct Loc_less
{
  bool 
  operator()(Location_data const& a, Location_data const& b)
  {
    if (a.file < b.file)
      return true;
    if (b.file < a.file)
      return false;
    
    if (a.line < b.line)
      return true;
    if (b.line < a.line)
      return false;

    return a.col < b.col;
  }
};


// A global table of source locations. 
std::set<Location_data, Loc_less> locs_;


} // namespace


// Memoize the current source location, and return a handle
// to it.
Location
Location_data::save() const
{
  auto ins = locs_.insert(*this);
  return &*ins.first;
}


// FIXME: Actually use the file name.
std::ostream&
operator<<(std::ostream& os, File const* f)
{
  if (f)
    os << "<file>"; 
  else
    os << "<input>";
  return os;
}


std::ostream& 
operator<<(std::ostream& os, Location loc)
{
  if (!loc)
    return os << "<internal>";
  else
    return os << loc.file() << ':' << loc.line() << ':' << loc.column();
}


// Static values.
Location Location::none { };


} // namespace lingo
