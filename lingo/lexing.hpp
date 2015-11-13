// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#ifndef LINGO_LEXING_HPP
#define LINGO_LEXING_HPP

// The lexing module provides support for writing lexers
// by providing a number of generic scanners for different
// character sequences.
//
// These algorithms are based on a Lexer concept. Every
// lexer must expose a number of operations.

#include <lingo/string.hpp>
#include <lingo/location.hpp>
#include <lingo/token.hpp>
#include <lingo/algorithm.hpp>
#include <lingo/error.hpp>


namespace lingo
{

// -------------------------------------------------------------------------- //
//                             Character classes


// Returns true if c starts an identifier.
inline bool
is_identifier_start(char c)
{
  return is_alpha(c) || c == '_';
}


// Returns true if c can appear in the rest of an identifier.
inline bool
is_identifier_rest(char c)
{
  return is_identifier_start(c) || is_decimal_digit(c);
}


// -------------------------------------------------------------------------- //
//                             Integer lexers
//
// TODO: Conditionally support a character as a digit separator
// in integer values.
//
// Note that an integer value like '0x' is an error.
//
// TODO: Improve diagnostics.


// Lexically analyze a decimal integer. The current character
// is `s` must be in the set of decimal digits.
template<typename Lexer, typename Stream>
inline Result_type<Lexer>
lex_decimal_integer(Lexer& lex, Stream& s, Location loc)
{
  auto pred = [](Stream& s) { return next_element_if(s, is_decimal_digit); };
  auto range = match_range_after_first(s, pred);
  return lex.on_integer(loc, range.begin(), range.end(), 10);
}


namespace
{

// A helper function for lexing integers with a specified
// base prefix. If successful, returns a character range
// that includes the 2-character prefix.
template<typename Stream, typename P>
inline Range_over<Stream>
match_integer_in_base(Stream& s, P pred)
{
  auto first = get_n(s, 2);
  auto range = match_range_after_first(s, pred);
  if (range)
    return {first, range.end()};
  else
    return range;
}

} // namespace


// Lex a binary integer where the first two characters are
// known to be '0b' or '0B'
template<typename Lexer, typename Stream>
inline Result_type<Lexer>
lex_binary_integer(Lexer& lex, Stream& s, Location loc)
{
  auto pred = [](Stream& s) { return next_element_if(s, is_binary_digit); };
  auto range = match_integer_in_base(s, pred);
  if (!range) {
    error(loc, "expected binary-digit");
    return {};
  }
  return lex.on_integer(loc, range.begin(), range.end(), 2);
}


// Lex an octal integer where the first two characters are
// known to be '0o' or '0O' (that's a 0 and an O).
template<typename Lexer, typename Stream>
inline Result_type<Lexer>
lex_octal_integer(Lexer& lex, Stream& s, Location loc)
{
  auto pred = [](Stream& s) { return next_element_if(s, is_octal_digit); };
  auto range = match_integer_in_base(s, pred);
  if (!range) {
    error(loc, "expected octal-digit");
    return {};
  }
  return lex.on_integer(loc, range.begin(), range.end(), 8);
}


// Lex a hexadecimal integer where the first two characters are
// known to be '0x' or '0X'.
template<typename Lexer, typename Stream>
inline Result_type<Lexer>
lex_hexadecimal_integer(Lexer& lex, Stream& s, Location loc)
{
  auto pred = [](Stream& s) { return next_element_if(s, is_hexadecimal_digit); };
  auto range = match_integer_in_base(s, pred);
  if (!range) {
    error(loc, "expected hexadecimal-digit");
    return {};
  }
  return lex.on_integer(loc, range.begin(), range.end(), 16);
}


// Lex a numeric literal.
//
// TODO: If we know that we start with 0, then we can skip the
// first comparison. This could be two algorithms.
//
// TODO: Repeated comparisons for nth_element may repeatedly
// check for eof. This could be optimized.
//
// TODO: Add support for floating point values.
template<typename Lexer, typename Stream>
inline Result_type<Lexer>
lex_number(Lexer& l, Stream& s, Location loc)
{
  if (s.peek() == '0') {
    if (nth_element_is(s, 1, 'b'))
      return lex_binary_integer(l, s, loc);
    if (nth_element_is(s, 1, 'o'))
      return lex_octal_integer(l, s, loc);
    if (nth_element_is(s, 1, 'x'))
      return lex_hexadecimal_integer(l, s, loc);
  }
  return lex_decimal_integer(l, s, loc);
}

template<typename Lexer, typename Stream>
inline Result_type<Lexer>
lex_identifier(Lexer& l, Stream& s, Location loc)
{
  auto first = s.begin();
  while (!s.eof() && is_identifier_rest(s.peek()))
    s.get();
  return l.on_identifier(loc, first, s.begin());
}




} // namespace lingo

#endif
