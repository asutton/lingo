// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#ifndef LINGO_ERROR_HPP
#define LINGO_ERROR_HPP

// The error module contains facilities for managing error
// reporting.

#include <lingo/location.hpp>
#include <lingo/buffer.hpp>
#include <lingo/print.hpp>

#include <cstdio>
#include <string>
#include <vector>

namespace lingo
{

// -------------------------------------------------------------------------- //
//                            Diagnostics

// Different kinds of diagnostics.
// TODO: Consider adding classes of error messages that correspond
// to translation phases (lexical errors, syntax error, type errors
// etc.).
enum Diagnostic_kind
{
  error_diag,   // An error message.
  warning_diag, // A warning message.
  note_diag     // A note attached to another message.
};


// Represents the positions where underlining begins and ends. A caret
// represents a single position within the span (if specified).
//
// TODO: This could be better designed.
struct Diagnostic_info
{
  Diagnostic_info(Location loc) : kind(loc_info) { data.loc = loc; }
  Diagnostic_info(Region reg) : kind(reg_info) { data.reg = reg; }

  enum Kind
  {
    loc_info,
    reg_info
  };

  union Data
  {
    Data() { }
    Location loc;
    Region   reg;
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
  Diagnostic(Diagnostic_kind, Location, String const&);
  Diagnostic(Diagnostic_kind, Region, String const&);

  Diagnostic_kind kind;
  Diagnostic_info info;
  String          msg;
};


// Streaming
std::ostream& operator<<(std::ostream&, Diagnostic const&);


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
  int  errs_;     // Actual error count.
};


// -------------------------------------------------------------------------- //
//                          Diagnostic interface

void emit_diagnostics();
void reset_diagnostics();
int error_count();

void error(Location, String const&);
void error(Region, String const&);

void warning(Location, String const&);
void warning(Region, String const&);

void note(Location, String const&);
void note(Region, String const&);


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
//    error(reg, str, args...)
//
// If the span covers multiple lines, only the first is displayed.
//
// TODO: Find a better strategy for diagnosing multi-line errors.


// Emit an error diagnostic using the current buffer to resolve
// the source code location.
template<typename... Args>
inline void
error(Location loc, char const* msg, Args&&... args)
{
  error(loc, format(msg, std::forward<Args>(args)...));
}


// Emit an error diagnostic using the current buffer to resolve
// the source code span.
template<typename... Args>
inline void
error(Region reg, char const* msg, Args&&... args)
{
  error(reg, format(msg, std::forward<Args>(args)...));
}


// Emit an informational diagnostic at the current input location.
template<typename... Args>
inline void
error(char const* msg, Args&&... args)
{
  error(Location(), format(msg, std::forward<Args>(args)...));
}


// -------------------------------------------------------------------------- //
//                          Warning messages


// Emit a warning diagnostic using the current buffer to resolve
// the source code location.
template<typename... Args>
inline void
warning(Location loc, char const* msg, Args&&... args)
{
  warning(loc, format(msg, std::forward<Args>(args)...));
}


// Emit a warning diagnostic using the current buffer to resolve
// the source code location.
template<typename... Args>
inline void
warning(Region reg, char const* msg, Args&&... args)
{
  warning(reg, format(msg, std::forward<Args>(args)...));
}


// Emit an informational diagnostic at the current input location.
template<typename... Args>
inline void
warning(char const* msg, Args&&... args)
{
  warning(Location(), format(msg, std::forward<Args>(args)...));
}


// -------------------------------------------------------------------------- //
//                          Notes

// Emit a note diagnostic using the current buffer to resolve
// the source code location.
template<typename... Args>
inline void
note(Location loc, char const* msg, Args&&... args)
{
  note(loc, format(msg, std::forward<Args>(args)...));
}


// Emit a note diagnostic using the current buffer to resolve
// the source code location.
template<typename... Args>
inline void
note(Region reg, char const* msg, Args&&... args)
{
  note(reg, format(msg, std::forward<Args>(args)...));
}


// Emit an informational diagnostic at the current input location.
template<typename... Args>
inline void
note(char const* msg, Args&&... args)
{
  note(Location(), format(msg, std::forward<Args>(args)...));
}


// -------------------------------------------------------------------------- //
//                          Utilities


// The error count guard is an RAII class that determines
// if any errors were diagnosed during the processing of
// a function.
//
// The `ok()` member function returns true if no errors
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
