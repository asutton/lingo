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

#include "lingo/string.hpp"
#include "lingo/location.hpp"
#include "lingo/token.hpp"
#include "lingo/algorithm.hpp"
#include "lingo/error.hpp"

namespace lingo
{

// -------------------------------------------------------------------------- //
//                          Character stream


// A character stream provides a sequence of characters and has
// a very simple streaming interface consisting of only 5 functions:
// peek(), get(), and eof(), begin(), and end(). Character streams
// are the input to lexical analyzers.
//
// There are also two support functions: null() returns a non-character
// value (i.e., the null character), and location() returns the source
// location for a character in the stream.
//
// Note that as a general rule for streams, &s.peek() == s.begin().
//
// Hypothetically, the null() function is a mechanism for creating
// a value that contextually evaluates to false upon default construction.
// This is a stronger concept than the NullablePointer concept.
//
// TODO: Add legitimate file support.
//
// TODO: This does not belong in this file.
struct Character_stream
{
  using value_type = char;

  Character_stream(char const* f, char const* l)
    : first(f), last(l), loc()
  { }

  Character_stream(std::string const& s)
    : Character_stream(s.data(), s.data() + s.size())
  { }

  // Stream control
  bool eof() const { return first == last; }
  char const& peek() const;
  char        peek(int) const;
  char const& get();

  // Iterators
  char const* begin()       { return first; }
  char const* begin() const { return first; }
  char const* end()       { return last; }
  char const* end() const { return last; }

  // Returns the source location of the the current character.
  Location location() { return loc.save(); }

  // Returns the empty value.
  static char null() { return 0; }

  char const*   first; // Current character pointer
  char const*   last;  // Past the end of the character buffer
  Location_data loc;   // Current location in file
};



// -------------------------------------------------------------------------- //
//                                 Characters

// Returns true if c is white space.
inline bool
is_space(char c)
{
  return std::isspace(c);
}


// Returns true if c is alphabetical.
inline bool
is_alpha(char c)
{
  return std::isalpha(c);
}


// Returns true if c is a binary digit.
inline bool
is_binary_digit(char c)
{
  return c == '0' || c == '1';
}


// Returns true if c is an octal digit.
inline bool
is_octal_digit(char c)
{
  return '0' <= c && c <= '7';
}


// Returns true if c is a decimial digit.
inline bool
is_decimal_digit(char c)
{
  return std::isdigit(c);
}


// Returns true if c is a hexadecimal digit.
inline bool
is_hexadecimal_digit(char c)
{
  return std::isxdigit(c);
}


// Returns true if c starts an identifier. 
inline bool
is_ident_head(char c)
{
  return is_alpha(c) || c == '_';
}


// Returns true if c can appear in the rest of an identifier.
inline bool
is_ident_tail(char c)
{
  return is_ident_tail(c) || is_decimal_digit(c);
}


// -------------------------------------------------------------------------- //
//                             Integer lexers
//
// TODO: Conditionally support a character as a digit separator
// in integer values.
//
// Note that an integer value like '0x' is an error. Note that the
// lexers for prefixed integer values require the lexer to define 
// the following functions:
// 
//    cxt.on_expected(loc, t)
//    cxt.on_expected(loc, str)


// Lexically analyze a decimal integer. The current character
// is `s` must be in the set of decimal digits.
template<typename Lexer, typename Stream>
inline Result_type<Lexer>
lex_decimal_integer(Lexer& lex, Stream& s, Location loc)
{
  auto pred = [](Stream& s) { return next_element_if(s, is_decimal_digit); };
  auto range = match_range_after_first(s, pred);
  return lex.on_decimal_integer(loc, range.begin(), range.end());
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
  auto first = discard_n(s, 2);
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
    lex.on_expected(loc, "binary-digit");
    return {};
  }
  return lex.on_binary_integer(loc, range.begin(), range.end());
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
    lex.on_expected(loc, "octal-digit");
    return {};
  }
  return lex.on_octal_integer(loc, range.begin(), range.end());
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
    lex.on_expected(loc, "hexadecimal-digit");
    return {};
  }
  return lex.on_hexadecimal_integer(loc, range.begin(), range.end());
}


} // namespace lingo

#endif
