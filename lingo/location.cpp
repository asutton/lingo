// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "lingo/location.hpp"
#include "lingo/buffer.hpp"
#include "lingo/file.hpp"
#include "lingo/error.hpp"
#include "lingo/utility.hpp"

#include <iostream>


namespace lingo
{

// Returns the file associated with the location. If the location is
// not sourced from a file, this returns nullptr.
File const*
Location::file() const
{
  return as<File>(buf_);
}


File*
Location::file()
{
  return as<File>(buf_);
}


// Returns the line number of the source code location.
int
Location::line_number() const
{
  return locus().first;
}


// Returns the column number of the source code location.
int
Location::column_number() const
{
  return locus().second;
}


// Returns the locus of the source code location.
Locus
Location::locus() const
{
  return buf_->lines().locus(off_);
}


Line const&
Location::line() const
{
  return buf_->lines().line(off_);
}


File const*
Region::file() const
{
  return as<File>(buf_);
}


File*
Region::file()
{
  return as<File>(buf_);
}


// FIXME: Guarantee that the start and end lines are the same.
Line const&
Region::line() const
{
  lingo_assert(!is_multiline());
  return buf_->lines().line(start_);
}


Locus
Region::start_locus() const
{
  return buf_->lines().locus(start_);
}


Locus
Region::end_locus() const
{
  return buf_->lines().locus(end_);
}


std::ostream&
operator<<(std::ostream& os, Location const& loc)
{
  if (!loc)
    return os;
  if (loc.file())
    os << loc.file()->path().string() << ":";
  if (loc.buffer()) {
    Locus l = loc.locus();
    os << l.first << ':' << l.second;
  }
  return os;
}


std::ostream&
operator<<(std::ostream& os, Region const& span)
{
  return os << span.start_location();
}


} // namespace lingo
