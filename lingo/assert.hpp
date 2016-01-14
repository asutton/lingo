// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#ifndef LINGO_ASSERT_HPP
#define LINGO_ASSERT_HPP

// The assert facility provides macros for asserting
// and diagnosing logical errors in a program.

#include <lingo/format.hpp>


// Expands to a call to the unreachable function and inserts
// the function and line at which the insertion is called.
#define lingo_unreachable(msg, args...) \
  ::lingo::abort("{}:{}: " msg, __PRETTY_FUNCTION__, __LINE__, ## args)


// Like unreachable except that this indicates a feature
// that should be defined but is not.
#define lingo_unimplemented() \
  ::lingo::abort("{}:{}: unimplemented", __PRETTY_FUNCTION__, __LINE__)


// Expands to a call to the assertion function and inserts
// the function and line at which the assertion is called.
#define lingo_assert(cond)                                       \
  if (!(cond))                                                   \
    ::lingo::abort("{}:{}: assertion failed '{}'", __PRETTY_FUNCTION__, __LINE__, #cond)


// Like lingo_assert, but allows the inclusion of a message.
#define lingo_alert(cond, msg, args...)                          \
  if (!(cond))                                                   \
    ::lingo::abort("%s:%s: " msg, __PRETTY_FUNCTION__, __LINE__, ## args)


namespace lingo
{

// -------------------------------------------------------------------------- //
//                         Assertions and exceptions

// Cause a program to abort when reaching a branch that should
// never be executed.
//
// The default method of aborting is to throw an exception and
// allow a unit testing framework or debugger to catch it.
//
// TODO: Allow this to call abort() based on some configuration
// setting. Also, derive a new unlikely-to-be-caught exception
// to prevent users from doing the wrong thing.
//
// FIXME: cppformat fails to render strings that include
// braces. That's not particularly good for a compiler. I might
// need to revert to using something more printf-like.
template<typename... Args>
[[noreturn]]
inline void
abort(char const* msg, Args const&... args)
{
  // throw std::runtime_error("internal compiler error");
  throw std::runtime_error(format(msg, args...));
}


} // namespace lingo

#endif
