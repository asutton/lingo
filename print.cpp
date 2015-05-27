// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "lingo/print.hpp"

#include <iostream>

namespace lingo
{


Printer::~Printer()
{
  if (needs_space) 
    print_space(*this);
  
  if (needs_newline) 
    print_newline(*this);
}


void
print_chars(Printer& p, char c)
{
  p.os << c;
}


void
print_chars(Printer& p, char const* s)
{
  p.os << s;
}


void 
print_chars(Printer& p, char const* first, char const* last)
{
  while (first != last) {
    print_chars(p, *first);
    ++first;
  }
}


void
print_chars(Printer& p, std::string const& s)
{
  p.os << s;
}


void
print_value(Printer& p, std::intmax_t n)
{
  p.os << n;
}


void
print_value(Printer& p, double n)
{
  p.os << n;
}


void 
print_space(Printer& p)
{
  p.os << ' ';
}


void 
print_newline(Printer& p)
{
  p.os << '\n';
  print_indent(p);
}


void 
print_indent(Printer& p)
{
  for (int n = 0; n < 2 * p.depth; ++n)
    print_space(p);
}


// Returns the default stream for printing.
std::ostream&
default_print_stream()
{
  return std::cout;
}

} // namespace lingo
