// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#ifndef LINGO_ALGORITHM_HPP
#define LINGO_ALGORITHM_HPP

// The algorithm module defines a set of matching algorithms suitable
// for use in lexers and parsers. These typically operate on character
// and token streams, and support matching against a variety of
// different patterns.
//
// Note that more specialized algorithms can be found in the lexing
// and parsing modules.

#include "lingo/location.hpp"
#include "lingo/error.hpp"

#include <algorithm>
#include <type_traits>
#include <iostream>


namespace lingo
{

// The iterator type of an aggregate.
template<typename T>
using Iterator_type = decltype(std::begin(std::declval<T>()));


// The value type of a stream.
template<typename T>
using Value_type = typename T::value_type;


// The argument type of a transform (lexer or parser).
template<typename T>
using Argument_type = typename T::argument_type;


// The result type of a transform (lexer or parser).
template<typename T>
using Result_type = typename T::result_type;


// Yields the return type of the given call expression.
template<typename Call>
using Result_of = typename std::result_of<Call>::type;


// A Range is a pair of iterators.
template<typename I>
struct Range
{
  Range()
    : first(), last(first)
  { }

  Range(I f, I l)
    : first(f), last(l)
  { }

  bool empty() const { return first == last; }
  I begin() const { return first; }
  I end() const   { return last; }

  // Converts to true iff not empty.
  explicit operator bool() const { return !empty(); }

  I first;
  I last;
};


// An alias used to constraint ranges over containers or streams.
template<typename T>
using Range_over = Range<Iterator_type<T>>;


// -------------------------------------------------------------------------- //
//                              Next element


// Returns true if the next element in the stream is equal to x.
template<typename Stream, typename T>
inline bool 
next_element_is(Stream const& s, T const& x)
{
  return !s.eof() && s.peek() == x;
}


// Returns true if the next element in the stream is distinct
// from x.
template<typename Stream, typename T>
inline bool 
next_element_is_not(Stream const& s, T const& x)
{
  return !s.eof() && s.peek() != x;
}


// Returns true if the nth element in the stream is equal to x.
template<typename Stream, typename T>
inline bool 
nth_element_is(Stream const& s, int n, T const& x)
{
  return !s.eof() && s.peek(n) == x;
}


// Returns true if the next element satisfies the given predicate.
template<typename Stream, typename Pred>
inline bool 
next_element_if_unguarded(Stream const& s, Pred pred)
{
  return pred(s.peek());
}

// Returns true if the next element satisfies the given predicate.
template<typename Stream, typename Pred>
inline bool 
next_element_if(Stream const& s, Pred pred)
{
  return !s.eof() && pred(s.peek());
}


// -------------------------------------------------------------------------- //
//                         Next element in

// Returns false when there are no elements to match.
template<typename Stream>
inline bool
next_element_in(Stream const& s)
{
  return false;
}


// Returns true when the element matches in the set of elements
// given by [first, args...].
template<typename Stream, typename T, typename... Args>
inline bool
next_element_in(Stream const& s, T const& first, Args const&... args)
{
  if (next_element_is(s, first))
    return true;
  else
    return next_element_in(s, args...);
}


// -------------------------------------------------------------------------- //
//                         Next elements are


// Base case. Returns false.
template<typename Stream>
inline bool
next_elements_past_n_are(Stream const& s, int)
{
  return true;
}


// Rerusive step. Match the nth element and check the rest.
template<typename Stream, typename T, typename... Args>
inline bool
next_elements_past_n_are(Stream const& s, int n, T const& first, Args const&... args)
{
  if (nth_element_is(s, n, first))
    return next_elements_past_n_are(s, n + 1, args...);
  return false;
}


// Returns true when the next elements in match those given
// in by args.... Note that this operation requires lookahead
// of sizeof...(Args). 
//
// If the required lookahead exceeds the buffer, this returns
// false.
template<typename Stream, typename T, typename... Args>
inline bool
next_elements_are(Stream const& s, Args const&... args)
{
  return next_elements_past_n_are(s, 0, args...);
}


// -------------------------------------------------------------------------- //
//                               Match 
//
// The matching algorithms operate on streams. A stream `s` is required
// to support the following operations:
//
// - `s.eof()` -- returns true when the stream is at the end of file.
// - `s.peek()` -- returns a reference to the current element
// - `s.get()` -- returns a reference to the current element and advances
//
// Additionally, the iterator type must be the same `&s.get()`, 
// and default constructing the iterator type must be contextually 
// convertible to bool. This is typically a pointer into the stream.


// If the current element of the stream exactly matches t, then
// return an iterator to that element and advance the stream. 
// Otherwise, return a null iterator.
//
// If the stream is past the end, then it does not match.
template<typename Stream, typename T>
inline Iterator_type<Stream>
match(Stream& s, T const& t)
{
  if (!s.eof() && s.peek() == t)
    return &s.get();
  else
    return {};
}



// If the current element stream satisfis the predicate `pred`,
// then return an iterator to that element. Otherwise, return the 
// null iterator.
template<typename Stream, typename P>
inline Iterator_type<Stream>
match_if(Stream& s, P pred)
{
  if (!s.eof() && pred(s.peek()))
    return &s.get();
  else
    return {};
}


// -------------------------------------------------------------------------- //
//                            Match any

// The base case for match_any returns the null element.
template<typename Stream>
inline Iterator_type<Stream>
match_any(Stream& s)
{
  return {};
}


// If the current element in the stream matches any element
// in the argument pack [a, args...], return that element.
// Otherwise, return the null element.
template<typename Stream, typename A, typename... Args>
inline Iterator_type<Stream>
match_any(Stream& s, A const& a, Args const&... args)
{
  if (auto iter = match(s, a))
    return iter;
  else
    return match_any(s, args...);
}


// -------------------------------------------------------------------------- //
//                            Match any if

// The base case for match_any_if returns the null element.
template<typename Stream>
inline Iterator_type<Stream>
match_any_if(Stream& s)
{
  return {};
}


// If the current element in the stream satisfies one of
// the predicates in [pred, preds...], return the position of
// that element and advance the stream. Otherwise, return
// the null element.
template<typename Stream, typename P, typename... Preds>
inline Iterator_type<Stream>
match_any_if(Stream& s, P pred, Preds const&... preds)
{
  if (auto iter = match_if(s, pred))
    return iter;
  else
    return match_any_if(s, preds...);
}


// -------------------------------------------------------------------------- //
//                            Match all

// The base case for match_all returns current element.
template<typename Stream>
inline Iterator_type<Stream>
match_all(Stream& s)
{
  return s.begin();
}


// If the current element in the stream matches each element
// in the argument pack [a, args...], return a pointer past
// the last matched element. Otherwise, returns the null
// element.
template<typename Stream, typename A, typename... Args>
inline Iterator_type<Stream>
match_all(Stream& s, A const& a, Args const&... args)
{
  if (match(s, a))
    return match_all(s, args...);
  else
    return {};
}



// -------------------------------------------------------------------------- //
//                            Match range

// Returns an iterator range corresponding to a sequence
// of matched elements in the stream when the current element
// of the stream satisfies `pred`. 
template<typename Stream, typename P>
inline Range_over<Stream>
match_range_after_first(Stream& s, P pred)
{
  assert(pred(s));
  Iterator_type<Stream> first = &s.get(); // Save the first position
  while (!s.eof() && pred(s)) {
    s.get();
  }
  Iterator_type<Stream> last = s.begin(); // Past the end of the range
  return {first, last};
}


// Returns an iterator range corresponding to a sequence
// of matched elements in the stream. If the current element
// is not in the set designated by match, the resulting range
// is empty. 
template<typename Stream, typename P>
inline Range_over<Stream>
match_range(Stream& s, P pred)
{
  if (s.eof())
    return {s.end(), s.end()};
  if (!match(s))
    return {s.begin(), s.begin()};
  return match_range_after_first(s, match);
}


// -------------------------------------------------------------------------- //
//                            Get n elements

// Get n elements in the stream that satisfy the given
// predicate until reachin the first that does not. Returns
// an iterator to the first element discarded.
template<typename Stream>
inline Iterator_type<Stream>
get_n(Stream& s, int n)
{
  auto iter = s.begin();
  while (!s.eof() && n != 0) {
    s.get();
    --n;
  }
  return iter;
}


// -------------------------------------------------------------------------- //
//                              Discard if

// Discard all elements in the stream that satisfy the given
// predicate until reachin the first that does not. Returns
// an iterator to the first element discarded.
template<typename Stream, typename P>
inline Iterator_type<Stream>
discard_if(Stream& s, P pred)
{
  auto iter = s.begin();
  while (!s.eof() && pred(s))
    s.get();
  return iter;
}


// -------------------------------------------------------------------------- //
//                            Expect element
//
// The expect* algorithms are similar to the match algorithms except
// that diagnostics are emitted when the match fails. To support
// diagnostics, these algorithms also require the stream to provide
// support for source code locations. For any stream `s`
//
//    s.location()
//
// returns the current location in the underlying file or buffer. The
// only requirement for this type is that it is semiregular. 


// If the current element matches of the stream the given kind, 
// advance the stream, returning an iterator to the matched
// element. Otherwise, calls the on_expected error in the
// context.
template<typename Context, typename Stream, typename T>
Iterator_type<Stream>
expect(Context& cxt, Stream& s, T const& t)
{
  if (auto iter = match(s, t))
    return iter;
  
  if (!s.eof())
    error(s.location(), "expected '{}' but got '{}'", t, s.peek());
  else
    error(s.last_location(), "expected '{}' but got end-of-file", t);
  return {};
}


// If the current element matches satisfies the predicate `pred`,
// return an iterator to that element and advance the stream.
// The `cond` string is used to provide feedback to the translation
// via the `on_expected` methods.
template<typename Context, typename Stream, typename P>
Iterator_type<Stream>
expect_if(Context& cxt, Stream& s, P pred, char const* cond)
{
  if (auto iter = match_if(s, pred))
    return iter;

  if (!s.eof())
    error(s.location(), "expected '{}' but got '{}'", cond, s.peek());
  else
    error(s.last_location(), "expected '{}' but got end-of-file", cond);
  return {};
}


// -------------------------------------------------------------------------- //
//                            Require element

// Returns the current token and advances the lexer to the
// next token. The current token is required to have kind k.
template<typename Stream, typename T>
Iterator_type<Stream>
require(Stream& s, T const& t)
{
  assert(s.peek() == t);
  return &s.get();
}


// Returns the current token and advances the lexer to the
// next token. The current token is required to have kind k.
template<typename Stream, typename P>
Iterator_type<Stream>
require_if(Stream& s, P pred)
{
  assert(pred(s.peek()));
  return &s.get();
}

} // namespace lingo

#endif
