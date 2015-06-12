// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#ifndef LINGO_STRING_HPP
#define LINGO_STRING_HPP

// The string module defines facilities and abstractions for
// working with character strings.

#include <cstring>
#include <algorithm>
#include <string>
#include <iosfwd>

namespace lingo
{

// A string is a sequence of characters.
using String = std::string;


// A view of a string in a source file. A string view is
// represented as a pair of pointers into text owned by
// another object.
class String_view
{
public:
  String_view(char const* f, char const* l)
    : first(f), last(l)
  { }

  String_view(char const* s)
    : first(s), last(s + std::strlen(s))
  { }

  int size() const { return last - first; }
  std::size_t hash() const;
  std::string str() const { return {first, last}; }

  char operator[](int n) { return *(first + n); }

  // Iterators
  char const* begin() { return first; }
  char const* end() { return last; }

  char const* begin() const { return first; }
  char const* end() const { return last; }

private:
  char const* first;
  char const* last;
};


// Reference equality
inline bool 
operator==(String_view const& a, String_view const& b) 
{
  return a.begin() == b.begin() && a.end() == b.end();
}


inline bool 
operator!=(String_view const& a, String_view const& b) 
{
  return !(a == b);
}


// Streaming
std::ostream& operator<<(std::ostream&, String_view);


// This hash function is used by the symbol table to hash
// string views.
struct String_view_hash
{
  std::size_t 
  operator()(String_view s) const
  { 
    return s.hash(); 
  }
};


// Value equality for strings. This is used to compare entries
// in the symbol table when collisions occur.
struct String_view_eq
{
  bool 
  operator()(String_view a, String_view b) const
  {
    return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin());
  }
};


} // namespace lingo


#endif
