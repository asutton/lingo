// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#ifndef LINGO_PRINT_HPP
#define LINGO_PRINT_HPP

// This module defines services for printing elements of
// a program. This is typically used for pretty printing,
// compiler diagnostics, and debug output.
//
// There are three primary components of the print module:
//
//    - a Printer class that maintains state and flags
//      for the printed text,
//    - a set of primitive functions for printing strings
//      and values, and
//    - and a set of functions for printing nodes.
//
// This library is designed around the concept `Printable`.
// All Printable types overload the `print` function.
//
// In the cotnext of nodes representing programs, we assume
// that printing yields text that represents the program in
// its original text. In contrast, debug printing generates
// an alternative textual representation.
//
// TODO: The design needs to be more carefully considered.
// It's entirely possible that a simple change to cppformat
// to accept pointers would eliminate a lot of the
// redundancies applied here. Same with debug printing.

#include "lingo/format.hpp"
#include "lingo/string.hpp"

#include <cstdint>

namespace lingo
{

// The printer class maintains information needed to format
// the output of pretty printing functions.
struct Printer
{
  Printer(std::ostream& s)
    : os(s), depth(0), needs_space(false), needs_newline(false)
  { }

  // Copy the output stream and indent depth, not the
  // formatting flags.
  Printer(Printer const& p)
    : os(p.os), depth(p.depth), needs_space(false), needs_newline(false)
  { }

  ~Printer();

  std::ostream& os;
  int depth;
  bool needs_space;
  bool needs_newline;
};


// Increase the indentation depth of the printer by
// one level.
inline void 
indent(Printer& p)
{
  ++p.depth;
}


// Decrease the indentation depth of the printer by
// one level.
inline void
undent(Printer& p)
{
  --p.depth;
}


std::ostream& default_print_stream();


// Core printing functions
void print_chars(Printer&, char);
void print_chars(Printer&, char const*);
void print_chars(Printer&, char const*, char const*);
void print_chars(Printer&, String const&);
void print_value(Printer&, std::intmax_t);
void print_value(Printer&, double);
void print_space(Printer&);
void print_newline(Printer&);
void print_indent(Printer&);


// -------------------------------------------------------------------------- //
//                              Pretty printing
//
// An object t has Printable type iff the following
// expression is valid:
//
//    print(p, t)
//
// Here, p is a printer object.


// Print the character `c`.
inline void 
print(Printer& p, char c)
{
  print_chars(p, c);
}


// Print the C-string `s`.
inline void 
print(Printer& p, char const* s)
{
  print_chars(p, s);
}


// Print the string `s`.
inline void 
print(Printer& p, String const& s)
{
  print_chars(p, s);
}


// Print the string pointed to by `s`.
inline void 
print(Printer& p, String const* s)
{
  print_chars(p, *s);
}


// Print the string view `s`.
inline void
print(Printer& p, String_view s)
{
  print_chars(p, s.begin(), s.end());
}


// Print the integral value `n`.
template<typename T>
inline typename std::enable_if<std::is_integral<T>::value, void>::type
print(Printer& p, T n)
{
  print_value(p, (intmax_t)n);
}


// Print the integer representation of the enumerated
// value `n`.
template<typename T>
inline typename std::enable_if<std::is_enum<T>::value, void>::type
print(Printer& p, T n)
{
  print_value(p, (intmax_t)n);
}


// Print the value `n`.
inline void
print(Printer& p, double n)
{
  print_value(p, n);
}


// Return a string containing the textual representation
// of `x`, using the `print` function to generate the 
// resulting text. 
//
// Requires that `print(p, x)` is well-formed.
template<typename T, typename P>
inline String
to_string(T const& x, P print)
{
  std::stringstream ss;
  Printer p(ss);
  print(p, x);
  return ss.str();
}


// Returns a string containing the textual representation
// of `x`. 
//
// Requires that `T` is a Printable type.
template<typename T>
inline String
to_string(T const& x)
{
  std::stringstream ss;
  Printer p(ss);
  print(p, x);
  return ss.str();
}


// Return a string containing the given `msg` string
// with the formatted with `args`.
//
// This is essentially equivalent to cppformat's format()
// function (which underlies all of the formatting and
// printing facilities), except that it is adapted to
// handle Node pointers.
template<typename... Args>
inline String
to_string(char const* msg, Args const&... args)
{
  return format(msg, to_string(args)...);
}


// Print a formatted sequence message. Note that each
// argument following the formatting string must be
// a Printable type.
//
//    print(p, "operator {}", x)
//
// will print 'operator ${x}' where '${x}' is the textual
// representation of 'x'.
template<typename... Args>
inline void
print(Printer& p, char const* msg, Args const&... args)
{
  print_chars(p, format(msg, to_string(args)...));
}


// Prrint a terminal (unary) node.
template<typename T>
inline void
print_terminal(Printer& p, T const* node)
{
  print(p, node->first);
}


// TODO: Generalize this so that the inner print call can
// be a function.
template<typename T>
inline void
print_enclosed(Printer& p, char left, char right, T const& x)
{
  print(p, left);
  print(p, x);
  print(p, right);
}


// Print an element enclosed in quotes. 
template<typename T>
inline void
print_quoted(Printer& p, T const& x)
{
  print_enclosed(p, '\"', '\"', x);
}


// Print an element enclosed in parens.
template<typename T>
inline void
print_paren_enclosed(Printer& p, T const& x)
{
  print_enclosed(p, '(', ')', x);
}


// Print a nested Range of Printable values as
// a comma-separated sequence of terms.
template<typename T>
inline void
print_list(Printer& p, T const* range)
{
  auto first = range->begin();
  auto last = range->end();
  
  // Don't print anything for empty ranges.
  if (first == last)
    return;

  while (first != last) {
    print(p, *first);
    if (std::next(first) != last)
      print_chars(p, ", ");
    ++first;
  }
}


// Print a nested Range of Printable values.
//
// TODO: Be smart about this? Try printing to
// a string to determine if it exceeds the printer's
// width, and if it does, then do a nested print.
template<typename T>
inline void
print_nested(Printer& p, T const& range)
{
  auto first = range.begin();
  auto last = range.end();
  
  // Don't print anything for empty ranges.
  if (first == last)
    return;

  indent(p);
  print_newline(p);
  while (first != last) {
    if (std::next(first) != last) {
      print(p, *first);

      // FIXME: This shouldn't be here.
      print(p, ',');
      
      print_newline(p);
    } else {
      print(p, *first);
    }
    ++first;
  }
  undent(p);
  print_newline(p);
}


// Print the given element, followed by a newline.
template<typename T>
inline void 
print(T const& x)
{
  Printer p(default_print_stream());
  p.needs_newline = true;
  print(p, x);
}


// Print the formatted message, followed by a
// newline.
template<typename... Args>
inline void
print(char const* msg, Args const&... args)
{
  Printer p(default_print_stream());
  p.needs_newline = true;
  print_chars(p, format(msg, to_string(args)...));
}


} // namespace lingo

#endif
