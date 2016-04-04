// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#ifndef LINGO_LOCATION_HPP
#define LINGO_LOCATION_HPP

// The location module provides facilities for representing
// locations in source code.

#include <lingo/assert.hpp>
#include <lingo/utility.hpp>

#include <iosfwd>

namespace lingo
{

class Buffer;
class File;
class Line;


// The location class represents the position of text within
// a source file. This is a pair: the file in which the text
// resides and its the starting offset of that text. Note that
// the ending offset is determined by the kind of element:
// token, comment, etc.
//
// Note that for redirected input, the file may be null.
class Location
{
public:
  Location()
    : buf_(nullptr), off_(-1)
  { }

  Location(int n)
    : buf_(nullptr), off_(n)
  { }

  Location(Buffer* b, int n)
    : buf_(b), off_(n)
  { }

  Buffer const* buffer() const { return buf_; }
  Buffer*       buffer()       { return buf_; }

  File const*   file() const;
  File*         file();

  int           offset() const { return off_; }
  int           line_number() const;
  int           column_number() const;
  Locus         locus() const;
  Line const&   line() const;

  // Convert to true when the location is valid. This
  // is the case when the offset is not -1.
  explicit operator bool() const { return off_ != -1; }

private:
  Buffer* buf_;
  int     off_;
};


inline bool
operator==(Location a, Location b)
{
  return a.buffer() == b.buffer() && a.offset() == b.offset();
}


inline bool
operator!=(Location a, Location b)
{
  return !(a == b);
}


// A region of text is contiguous region of characters within
// a file, possibly spanning multiple lines.
//
// The start location shall be less than or equal to the end
// location. A span is never empty.
//
// TODO: Support multi-line spnas?
struct Region
{
  Region()
    : buf_(nullptr), start_(-1), end_(-1)
  { }

  Region(Buffer* b, int m, int n)
    : buf_(b), start_(m), end_(n)
  { }

  // The locations shall be sourced from the same file.
  Region(Location s, Location e)
    : buf_(s.buffer()), start_(s.offset()), end_(e.offset())
  {
    lingo_assert(buf_ == e.buffer());
  }

  Buffer const* buffer() const { return buf_; }
  Buffer*       buffer()       { return buf_; }

  File const* file() const;
  File*       file();

  // Returns the start and end offsets.
  int start_offset() const { return start_; }
  int end_offset() const   { return end_; }

  // Retursn the start and end source locations.
  Location start_location() const { return {buf_, start_}; }
  Location end_location() const   { return {buf_, end_}; }

  Locus start_locus() const;
  Locus end_locus() const;
  int   start_line_number() const;
  int   end_line_number() const;
  int   start_column_number() const;
  int   end_column_number() const;

  // Returns the line of text associated with the region. Behavior
  // is defined only when is_multiline() is false.
  Line const& line() const;

  // Returns true if the region spans multiple lines of text.
  bool is_multiline() const;

  // Contextually converts to true iff both the start and
  // end are valid offsets.
  explicit operator bool() const { return start_ != -1; }

  Buffer* buf_;
  int     start_;
  int     end_;
};


inline int
Region::start_line_number() const
{
  return start_locus().first;
}


inline int
Region::end_line_number() const
{
  return end_locus().first;
}


inline int
Region::start_column_number() const
{
  return start_locus().second;
}


inline int
Region::end_column_number() const
{
  return end_locus().second;
}


// Returns true if the span covers multiple lines of text.
inline bool
Region::is_multiline() const
{
  return start_line_number() != end_line_number();
}



std::ostream& operator<<(std::ostream&, Location const&);
std::ostream& operator<<(std::ostream&, Region const&);


} // namespace lingo


#endif
