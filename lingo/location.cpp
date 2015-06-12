// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "location.hpp"
#include "buffer.hpp"
#include "file.hpp"
#include "error.hpp"

#include <iostream>

namespace lingo
{

// Static values.
Location Location::none { };


bool
Bound_location::is_file_location() const
{
  return dynamic_cast<File const*>(&buf_);
}


File const& 
Bound_location::file() const
{
  lingo_assert(is_file_location());
  return dynamic_cast<File const&>(buf_);
}


// Return the line of text represented by this location.
Line const&
Bound_location::line() const
{
  return buf_.line(loc_);
}


// Returns the line number for this location.
int 
Bound_location::line_no() const
{
  return buf_.line_no(loc_);
}


// Returns the column number for this location.
int 
Bound_location::column_no() const
{
  return buf_.column_no(loc_);
}


std::ostream& 
operator<<(std::ostream& os, Bound_location const& loc)
{
  if (!loc.is_valid())
    return os;

  if (loc.is_file_location())
    os << loc.file().path() << ":";

  return os << loc.line_no() << ':' << loc.column_no();
}


} // namespace lingo
