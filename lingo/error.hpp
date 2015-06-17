// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#ifndef LINGO_ERROR_HPP
#define LINGO_ERROR_HPP

// The error module contains facilities for managing error
// reporting.
//
// TODO: The diagnostic framework needs a bit of work. In particular,
// we should be emitting errors as they occurr instead of (or in
// addition to?) keeping them for later. 

#include "lingo/location.hpp"
#include "lingo/buffer.hpp"
#include "lingo/print.hpp"

#include <string>
#include <vector>


// Expands to a call to the unreachable function and inserts
// the function and line at which the insertion is called.
#define lingo_unreachable(msg, args...) \
  lingo::abort("{}:{}: " msg, __FUNCTION__, __LINE__, args)


// Expands ot a call to the assertion function and inserts
// the function and line at which the assertion is called.
#define lingo_assert(cond)                                       \
  if (!(cond))                                                   \
    lingo::abort("{}:{}: assertion failed '{}'", __PRETTY_FUNCTION__, __LINE__, #cond)


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
template<typename... Args>
[[noreturn]] 
inline void
abort(char const* msg, Args const&... args)
{
  throw std::runtime_error(to_string(msg, args...));
}


// -------------------------------------------------------------------------- //
//                            Diagnostics

// Different kids of diagnostics.
enum Diagnostic_kind
{
  error_diag,   // An error message
  warning_diag, // A warning message
  note_diag     // A note attached to another message
};


// A diagnostic reports a kind of error or informative
// note occurring at a particular location.
struct Diagnostic
{
  Diagnostic(Diagnostic_kind k, Bound_location l, String const& m)
    : kind(k), loc(l), msg(m)
  { }

  Diagnostic_kind kind;
  Bound_location  loc;
  String          msg;
};


// A diagnostic context is a record of all diagnostic messages
// that have been emitted as part some processing phase.
//
// Note diagnostics may be suppressed until later. This helps
// to allow tentative processing.
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

void init_diagnostics();
void emit_diagnostics();
void reset_diagnostics();
int error_count();


void error(Bound_location, String const&);
void warning(Bound_location, String const&);
void note(Bound_location, String const&);


// Emit an error diagnostic.
template<typename... Ts>
inline void
error(Buffer const& buf, Location loc, char const* msg, Ts const&... args)
{
  error(buf.location(loc), format(msg, to_string(args)...));
}


// Emit an error diagnostic using the current buffer to resolve
// the source code location.
template<typename... Ts>
inline void
error(Location loc, char const* msg, Ts const&... args)
{
  error(input_buffer(), loc, msg, args...);
}


// Emit a warning diagnostic.
template<typename... Ts>
inline void
warning(Buffer& buf, Location loc, char const* msg, Ts const&... args)
{
  warning(buf.location(loc), format(msg, to_string(args)...));
}


// Emit a warning diagnostic using the current buffer to resolve
// the source code location.
template<typename... Ts>
inline void
warning(Location loc, char const* msg, Ts const&... args)
{
  warning(input_buffer(), loc, msg, args...);
}


// Emit a diagnostic note.
template<typename... Ts>
inline void
note(Buffer& buf, Location loc, char const* msg, Ts const&... args)
{
  note(buf.location(loc), format(msg, to_string(args)...));
}


// Emit a note diagnostic using the current buffer to resolve
// the source code location.
template<typename... Ts>
inline void
note(Location loc, char const* msg, Ts const&... args)
{
  note(input_buffer(), loc, msg, args...);
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
