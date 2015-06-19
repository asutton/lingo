// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#ifndef LINGI_UTIL_DEBUG_HPP
#define LINGI_UTIL_DEBUG_HPP

#include "lingo/print.hpp"

#include <iosfwd>
#include <sstream>
#include <iterator>
#include <stdexcept>

namespace lingo
{

// A helper facility for printing the header and trailer
// of a node.
template<typename T>
struct sexpr
{
  sexpr(Printer& p, T const* n, bool space = true)
    : p(p)
  {
    print_chars(p, '(');
    print_chars(p, n->node_name());
    if (space)
      print_space(p);
  }
  ~sexpr()
  {
    print_chars(p, ')');
  }
  Printer& p;
};


std::ostream& default_debug_stream();


// A dummy function that provides a name for an
// overload set.
void debug(Printer& p, void*) = delete;


// Debug print the integral value n.
inline void 
debug(Printer& p, std::intmax_t n)
{
  print_value(p, n);
}


// Debug print a string.
inline void
debug(Printer& p, char const* s)
{
  print_chars(p, s);
}


// Debug print a string.
inline void
debug(Printer& p, String const& s)
{
  print_chars(p, s);
}


// Debug print a string.
inline void
debug(Printer& p, String const* s)
{
  print_chars(p, *s);
}


// Debug print a string view.
inline void
debug(Printer& p, String_view s)
{
  print_chars(p, s.begin(), s.end());
}


// Debug print the given element, followed by a newline.
// This function is provided as an entry point to the
// usual debbuging facilities.
template<typename T>
inline void 
debug(T const& x)
{
  Printer p(default_debug_stream());
  p.needs_newline = true;
  debug(p, x);
}


// Return a string containing the debug output for the
// element x.
template<typename T>
inline String
debug_str(T const& x)
{
  std::stringstream ss;
  Printer p(ss);
  debug(p, x);
  return ss.str();
}


// Used in the debug function below.
struct Debug_render
{
  template<typename T>
  void operator()(Printer& p, T const& x) const { debug(p, x); }
};


// Write the formatted debug message followed by a newline.
// For example:
//
//    debug("testing {}", x)
//
// will print 'testing <debug>' where '<debug>' is the
// debug rendering of 'x'.
template<typename... Args>
inline void
debug(char const* msg, Args const&... args)
{
  Printer p(default_debug_stream());
  print_chars(p, format(msg, to_string(args, Debug_render())...));
  print_newline(p);
}


// Debug the given element, and print a space after it.
template<typename T>
inline void 
debug_space(Printer& p, T const& x)
{
  Printer q = p;
  q.needs_space = true;
  debug(q, x);
}


// Debug the given element, and print a newline after it.
// This also indents after the newline.
template<typename T>
inline void 
debug_newline(Printer& p, T const& x)
{
  Printer q = p;
  q.needs_newline = true;
  debug(q, x);
}


// Print the word '(null)'.
inline void
debug_null(Printer& p)
{
  print_chars(p, "(null)");
}


// Write out the contents of the list in a single
// line of text.
template<typename T>
void
debug_flat_list(Printer& p, T const* list)
{
  print_chars(p, '(');
  auto first = list->begin();
  auto last = list->end();
  while (first != last) {
    if (std::next(first) != last)
      debug_space(p, *first);
    else
      debug(p, *first);
    ++first;
  }
  print_chars(p, ')');
}


// Write out the contents of a list so that each element
// appears, indented, on a new line.
template<typename T>
void
debug_nested_list(Printer& p, T const* list)
{
  if (list->empty()) {
    print_chars(p, "()");
    return;
  }

  indent(p);
  print_newline(p);
  print_chars(p, '(');
  indent(p);
  print_newline(p);
  auto first = list->begin();
  auto last = list->end();
  while (first != last) {
    if (std::next(first) != last)
      debug_newline(p, *first);
    else
      debug(p, *first);
    ++first;
  }
  undent(p);
  print_newline(p);
  print_chars(p, ')');
  undent(p);
  print_newline(p);
}


// Debug print a vertical, unindented list of elements.
template<typename T>
void
debug_naked_list(Printer& p, T const* list)
{
  auto first = list->begin();
  auto last = list->end();
  while (first != last) {
    if (std::next(first) != last)
      debug_newline(p, *first);
    else
      debug(p, *first);
    ++first;
  }
}


// Debug print a nullary node. This simply prints
// out the node name.
template<typename T>
inline void
debug_nullary(Printer& p, T const* node)
{
  sexpr<T> s(p, node, false);
}


// Debug print a unary node.
template<typename T>
inline void
debug_unary(Printer& p, T const* node)
{
  sexpr<T> s(p, node);
  debug(p, node->first);
}


// Debug print a binary node.
template<typename T>
inline void
debug_binary(Printer& p, T const* node)
{
  sexpr<T> s(p, node);
  debug_space(p, node->first);
  debug(p, node->second);
}


// Debug print a ternary node.
template<typename T>
inline void
debug_ternary(Printer& p, T const* node)
{
  sexpr<T> s(p, node);
  debug_space(p, node->first);
  debug_space(p, node->second);
  debug(p, node->third);
}


} // namespace lingo

#endif
