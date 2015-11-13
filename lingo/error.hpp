// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#ifndef LINGO_ERROR_HPP
#define LINGO_ERROR_HPP

// The error module contains facilities for managing error
// reporting.

#include "lingo/location.hpp"
#include "lingo/buffer.hpp"
#include "lingo/print.hpp"

#include <cstdio>
#include <string>
#include <vector>


// Expands to a call to the unreachable function and inserts
// the function and line at which the insertion is called.
#define lingo_unreachable(msg, args...) \
  ::lingo::abort("{}:{}: " msg, __PRETTY_FUNCTION__, __LINE__, ## args)


// Like unreachable except that this indicates a feature
// that should be defined biut is not.
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


// -------------------------------------------------------------------------- //
//                            Diagnostics

// Different kids of diagnostics.
// TODO: Consider adding classes of error messages that correspond
// to translation pahses (lexical errors, syntax error, type errors
// etc.).
enum Diagnostic_kind
{
  error_diag,   // An error message
  warning_diag, // A warning message
  note_diag     // A note attached to another message
};


// Represents the positions where underlining begins and
// ends. A caret represennts a single position within
// the span (if specified).
//
// TODO: This could be better designed.
struct Diagnostic_info
{
  Diagnostic_info(Bound_location loc)
    : kind(loc_info)
  {
    data.loc = loc;
  }

  Diagnostic_info(Bound_span span)
    : kind(span_info)
  {
    data.span = span;
  }

  // Kinds of info.
  enum Kind 
  {
    loc_info,
    span_info
  };

  // These are simple integer types.
  union Data
  {
    Data() { }
    Bound_location loc;
    Bound_span     span;
  };

  Kind kind;
  Data data;
};


// A diagnostic reports a kind of error or informative
// note occurring at a particular location.
//
// TODO: Make diagnostics extensible. It might be (very)
// helpful to fully explain the cause of an error, and that
// would almost certainly require more than a single string's
// length of text.
//
// TODO: Allow notes to be attached to diagnostics instead
// of existing at the same level. Or remove notes altogether.
// Notes are informational and only provide context for 
// an error or warning.
struct Diagnostic
{
  Diagnostic(Diagnostic_kind, Bound_location, String const&);
  Diagnostic(Diagnostic_kind, Bound_span, String const&);

  Diagnostic_kind kind;
  Diagnostic_info info;
  String          msg;
};


// A diagnostic context is a record of all diagnostic messages that
// have been emitted as part some processing phase. 
//
// Note that diagnostics may be suppressed until later. This helps
// to allow tentative processing.
//
// When a diagnostic context is declared (as a variable), it becomes
// the active diagnostic context. When the declaration goes out of 
// scope, the previous context becomes active.
class Diagnostic_context : std::vector<Diagnostic>
{
public:
  Diagnostic_context(bool = false);
  ~Diagnostic_context();

  void emit(Diagnostic const&);
  void emit();

  void reset();

  // Returns true if diagnostics are suppressed.
  bool quiet() const { return !suppress_; }

  // Returns true if the context is error-free.
  bool ok() const    { return errs_ == 0; }

  // Returns the number of errors.
  int errors() const { return errs_; }

private:
  bool suppress_; // True if diagnostics are temporarily suppressed.
  int  errs_;     // Actual error count
};


// -------------------------------------------------------------------------- //
//                          Diagnostic interface

void emit_diagnostics();
void reset_diagnostics();
int error_count();


void error(Bound_location, String const&);
void error(Bound_span, String const&);

void warning(Bound_location, String const&);
void warning(Bound_span, String const&);

void note(Bound_location, String const&);
void note(Bound_span, String const&);


// Resolve a bound location. This is used internally. Do not call.
inline Bound_location
resolve(Buffer const& buf, Location loc)
{
  return buf.location(loc);
}


// Resolve a bound text span. This is used internally. Do not call.
inline Bound_span
resolve(Buffer const& buf, Span span)
{
  return buf.span(span);
}


// -------------------------------------------------------------------------- //
//                          Error messages
//
// Error messages are emitted by calling one of error() functions.
// The most basic usage is:
//
//    error("str", args...)
//
// Where str is a formatting string (see cppformat) and args... is
// the sequence of values to be substituted into the string. This
// uses the to_string() function defined in the print module to
// render the final string, so each argument must have an appropriate
// print() overload.
//
// The error is emitted at the location maintained by the current
// input context (see the buffer module).
//
// Error diagnostics can be emitted at particular source locations
// by providing a Location object:
//
//    error(loc, str, args....)
//
// An error can also be emitted over a region of text.
//
//    error(span, str, args...)
//
// If the span covers multiple lines, only the first is displayed.
//
// TODO: Find a better strategy for diagnosing multi-line errors.


// Emit an error diagnostic.
template<typename Caret, typename... Ts>
inline void
error(Buffer const& buf, Caret caret, char const* msg, Ts const&... args)
{
  error(resolve(buf, caret), format(msg, to_string(args)...));
}


// Emit an error diagnostic using the current buffer to resolve
// the source code location.
template<typename... Ts>
inline void
error(Location loc, char const* msg, Ts const&... args)
{
  error(input_buffer(), loc, msg, args...);
}


// Emit an error diagnostic using the current buffer to resolve
// the source code span.
template<typename... Ts>
inline void
error(Span span, char const* msg, Ts const&... args)
{
  error(input_buffer(), span, msg, args...);
}


// Emit an error diagnostic at the current input location.
template<typename... Ts>
inline void
error(char const* msg, Ts const&... args)
{
  error(input_buffer(), input_location(), msg, args...);
}


// -------------------------------------------------------------------------- //
//                          Warning messages

// Emit a warning diagnostic.
template<typename Caret, typename... Ts>
inline void
warning(Buffer& buf, Caret caret, char const* msg, Ts const&... args)
{
  warning(resolve(buf, caret), format(msg, to_string(args)...));
}


// Emit a warning diagnostic using the current buffer to resolve
// the source code location.
template<typename... Ts>
inline void
warning(Location loc, char const* msg, Ts const&... args)
{
  warning(input_buffer(), loc, msg, args...);
}


// Emit a warning diagnostic using the current buffer to resolve
// the source code location.
template<typename... Ts>
inline void
warning(Span span, char const* msg, Ts const&... args)
{
  warning(input_buffer(), span, msg, args...);
}


// Emit a warning diagnostic at the current input location.
template<typename... Ts>
inline void
warning(char const* msg, Ts const&... args)
{
  warning(input_buffer(), input_location(), msg, args...);
}


// -------------------------------------------------------------------------- //
//                          Notes

// Emit a diagnostic note.
template<typename Caret, typename... Ts>
inline void
note(Buffer& buf, Caret caret, char const* msg, Ts const&... args)
{
  note(resolve(buf, caret), format(msg, to_string(args)...));
}


// Emit a note diagnostic using the current buffer to resolve
// the source code location.
template<typename... Ts>
inline void
note(Location loc, char const* msg, Ts const&... args)
{
  note(input_buffer(), loc, msg, args...);
}


// Emit a note diagnostic using the current buffer to resolve
// the source code location.
template<typename... Ts>
inline void
note(Span span, char const* msg, Ts const&... args)
{
  note(input_buffer(), span, msg, args...);
}


// Emit an informational diagnostic at the current input location.
template<typename... Ts>
inline void
note(char const* msg, Ts const&... args)
{
  note(input_buffer(), input_location(), msg, args...);
}


// -------------------------------------------------------------------------- //
//                          Utilities


// The error count guard is an RAII class that determines
// if any errors were diagnosed during the processing of
// a function.
//
// The `ok()` member functoin returns true if no errors
// have been diagnosed at the point at which it is called.
struct Error_count_guard
{
  Error_count_guard()
    : errs(error_count())
  { }

  bool ok() const { return errs == error_count(); }

  int errs;
};


} // namespace lingo

#endif
