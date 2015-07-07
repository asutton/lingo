// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "lingo/print.hpp"

#include <cstring>
#include <iostream>

namespace lingo
{


namespace
{


// Manage the last character printed to the
// output stream.
struct Last_char_guard
{
  Last_char_guard(Printer& p, char c) 
    : printer(p), last(c)
  { }


  Last_char_guard(Printer& p, char const* str) 
    : printer(p), last(str[std::strlen(str) - 1])
  { }


  Last_char_guard(Printer& p, std::string const& str) 
    : printer(p), last(str[str.size() - 1])
  { }

  ~Last_char_guard()
  {
    printer.last = last;
  }  

  Printer& printer;
  char     last;
};


} // namespace


Printer::~Printer()
{
  if (needs_space && last != ' ') 
    print_space(*this);
  if (needs_newline && last != '\n')
    print_newline(*this);
}


void
print_chars(Printer& p, char c)
{
  Last_char_guard _(p, c);
  p.os << c;
}


void
print_chars(Printer& p, char const* s)
{
  Last_char_guard _(p, s);
  p.os << s;
}


void 
print_chars(Printer& p, char const* first, char const* last)
{
  if (first == last)
    return;
  Last_char_guard _(p, *(last -1));
  while (first != last) {
    print_chars(p, *first);
    ++first;
  }
}


void
print_chars(Printer& p, std::string const& s)
{
  Last_char_guard _(p, s);
  p.os << s;
}


void
print_value(Printer& p, std::intmax_t n)
{
  p.os << n;
  p.last = '0'; // An artificial non-ws character
}


void
print_value(Printer& p, double n)
{
  p.os << n;
  p.last = '0'; // An artifiical non-ws character
}


void 
print_space(Printer& p)
{
  Last_char_guard _(p, ' ');
  p.os << ' ';
}


// Print a newline and indent to the nest indented position.
void 
print_newline(Printer& p)
{
  p.os << '\n';
  if (p.depth)
    print_indent(p);
  else
    p.last = '\n';
}


// Print spaces for an indentation.
void 
print_indent(Printer& p)
{
  for (int n = 0; n < 2 * p.depth; ++n)
    print_space(p);
  p.last = ' ';
}


// Returns the default stream for printing.
std::ostream&
default_print_stream()
{
  return std::cout;
}


} // namespace lingo
