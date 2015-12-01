// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "location.hpp"
#include "buffer.hpp"
#include "file.hpp"
#include "error.hpp"
#include "utility.hpp"

#include <iostream>


namespace lingo
{

// Returns the file associated with the location. If the
// location is not sourced from a file, this returns
// nullptr.
File const*
Location::file() const
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
Span::file() const
{
  return as<File>(buf_);
}


// FIXME: Guarantee that the start and end lines are the same.
Line const&
Span::line() const
{
  lingo_assert(!is_multiline());
  return buf_->lines().line(start_);
}


Locus
Span::start_locus() const
{
  return buf_->lines().locus(start_);
}


Locus
Span::end_locus() const
{
  return buf_->lines().locus(end_);
}


int
Span::start_line_number() const
{
  return start_locus().first;
}


int
Span::end_line_number() const
{
  return end_locus().first;
}


int
Span::start_column_number() const
{
  return start_locus().second;
}


int
Span::end_column_number() const
{
  return end_locus().second;
}


// Returns true if the span covers multiple lines of text.
bool 
Span::is_multiline() const
{
  return start_line_number() != end_line_number();
}


std::ostream& 
operator<<(std::ostream& os, Location const& loc)
{
  if (!loc)
    return os;
  if (loc.file())
    os << loc.file()->path().string() << ":";
  Locus l = loc.locus();
  return os << l.first << ':' << l.second;
}


std::ostream& 
operator<<(std::ostream& os, Span const& span)
{
  return os << span.start_location();
}


} // namespace lingo
