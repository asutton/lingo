// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#ifndef LINGO_LINE_HPP
#define LINGO_LINE_HPP

#include <lingo/utility.hpp>
#include <lingo/string.hpp>

#include <map>


namespace lingo
{


// A line of text is the sequence of characters from column
// 0 up to its newline. Each line maintains a pair of pointers
// 
//
// The members of this class are public to allow them to be
// updated by a character stream during processing.
class Line
{
public:
  Line(int n, int c, char const* f, char const* l)
    : num_(n), off_(c), first_(f), last_(l)
  { }

  // Returns the line number.
  int number() const { return num_; }

  // Returns the character offset of the line.
  int offset() const { return off_; }

  // Returns a view of the string.
  String_view str() const { return String_view(first_, last_); }

  // Iterators into the string.
  char const* begin() const { return first_; }
  char const* end() const   { return last_; }

  int         num_;    // Line number
  int         off_;    // Offset of the first character
  char const* first_;
  char const* last_;
};


// A line map associates an offset in the source code with
// it's underlying line of text.
struct Line_map : std::map<int, Line>
{
  Locus       locus(int) const;
  Line const& line(int) const;
};


} // namespace lingo


#endif
