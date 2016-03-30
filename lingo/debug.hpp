// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#ifndef LINGI_UTIL_DEBUG_HPP
#define LINGI_UTIL_DEBUG_HPP

#include <lingo/print.hpp>
#include <lingo/node.hpp>

#include <iosfwd>
#include <sstream>
#include <iterator>
#include <vector>

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
    print_chars(p, get_node_name(n));
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


// Debug print the integral value `n`.
template<typename T>
inline typename std::enable_if<std::is_integral<T>::value, void>::type
debug(Printer& p, T n)
{
  print_value(p, (intmax_t)n);
}


// Debug print the integer representation of the enumerated
// value `n`.
template<typename T>
inline typename std::enable_if<std::is_enum<T>::value, void>::type
debug(Printer& p, T n)
{
  print_value(p, (intmax_t)n);
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
// usual debugging facilities.
template<typename T>
inline void
debug(T const& x)
{
  Printer p(default_debug_stream());
  p.needs_newline = true;
  debug(p, x);
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


// Print the word '<null>'.
inline void
debug_null(Printer& p)
{
  print_chars(p, "<null>");
}


// Print the word '<error>'.
inline void
debug_error(Printer& p)
{
  print_chars(p, "<error>");
}


// Write out the contents of the list in a single
// line of text.
template<typename T>
void
debug_flat_list(Printer& p, T const& list)
{
  print_chars(p, '(');
  auto first = list.begin();
  auto last = list.end();
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
debug_nested_list(Printer& p, T const& list)
{
  if (list.empty()) {
    print_chars(p, "()");
    return;
  }

  indent(p);
  print_newline(p);
  auto first = list.begin();
  auto last = list.end();
  while (first != last) {
    if (std::next(first) != last)
      debug_newline(p, *first);
    else
      debug(p, *first);
    ++first;
  }
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
inline typename std::enable_if<is_nullary_node<T>()>::type
debug(Printer& p, T const* node)
{
  sexpr<T> s(p, node, false);
}


// Debug print a unary node.
template<typename T>
inline typename std::enable_if<is_unary_node<T>()>::type
debug(Printer& p, T const* node)
{
  sexpr<T> s(p, node);
  debug(p, node->first);
}


// Debug print a binary node.
template<typename T>
inline typename std::enable_if<is_binary_node<T>()>::type
debug(Printer& p, T const* node)
{
  sexpr<T> s(p, node);
  debug_space(p, node->first);
  debug(p, node->second);
}


// Debug print a ternary node.
template<typename T>
inline typename std::enable_if<is_ternary_node<T>()>::type
debug(Printer& p, T const* node)
{
  sexpr<T> s(p, node);
  debug_space(p, node->first);
  debug_space(p, node->second);
  debug(p, node->third);
}


// Debug print a k-ary node.
template<typename T>
inline typename std::enable_if<is_kary_node<T>()>::type
debug(Printer& p, T const* node)
{
  sexpr<T> s(p, node);
  for (auto iter = node->begin(); iter != node->end(); ++iter) {
    debug(p, *iter);
    if (std::next(iter) != node->end())
      print_space(p);
  }
}


template<typename T>
void
debug(Printer& p, std::vector<T const*> const& v)
{
  debug_flat_list(p, v);
}


} // namespace lingo

#endif
