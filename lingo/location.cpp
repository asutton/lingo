// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "lingo/location.hpp"
#include "lingo/buffer.hpp"
#include "lingo/file.hpp"
#include "lingo/error.hpp"

#include <iostream>

namespace lingo
{

// Static values.
Location Location::none;


// -------------------------------------------------------------------------- //
//                                Location


bool
Bound_location::is_file_location() const
{
  return dynamic_cast<File const*>(buf_);
}


File const&
Bound_location::file() const
{
  lingo_assert(is_file_location());
  return dynamic_cast<File const&>(*buf_);
}


// Return the line of text represented by this location.
Line const&
Bound_location::line() const
{
  return buf_->line(loc_);
}


// Returns the line number for this location.
int
Bound_location::line_no() const
{
  return buf_->line_no(loc_);
}


// Returns the column number for this location.
int
Bound_location::column_no() const
{
  return buf_->column_no(loc_);
}

// -------------------------------------------------------------------------- //
//                                    Span

bool
Bound_span::is_file_location() const
{
  return dynamic_cast<File const*>(buf_);
}


File const&
Bound_span::file() const
{
  lingo_assert(is_file_location());
  return dynamic_cast<File const&>(*buf_);
}


// Return the first line of text in the span.
Line const&
Bound_span::line() const
{
  return buf_->line(span_.start());
}


// Returns the line number for this location.
int
Bound_span::start_line_no() const
{
  return buf_->line_no(span_.start());
}


int
Bound_span::end_line_no() const
{
  return buf_->line_no(span_.end());
}


// Returns the start column for the span.
int
Bound_span::start_column_no() const
{
  return buf_->column_no(span_.start());
}


// Returns the end column for the span.
int
Bound_span::end_column_no() const
{
  return buf_->column_no(span_.end());
}


std::ostream&
operator<<(std::ostream& os, Bound_location const& loc)
{
  if (!loc.is_valid())
    return os;

  if (loc.is_file_location())
    os << loc.file().path().string() << ":";

  return os << loc.line_no() << ':' << loc.column_no();
}


std::ostream&
operator<<(std::ostream& os, Bound_span const& span)
{
  if (!span.is_valid())
    return os;

  if (span.is_file_location())
    os << span.file().path().string() << ":";

  return os << span.start_line_no() << ':' << span.start_column_no();
}


} // namespace lingo
