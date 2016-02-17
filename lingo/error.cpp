// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "lingo/error.hpp"
#include "lingo/io.hpp"

#include <iostream>
#include <stdexcept>
#include <stack>


namespace lingo
{

namespace
{

// Print the source code location for a bound location
// or span.
void
show_location(std::ostream& os, Diagnostic_info const& info)
{
  if (info.kind == Diagnostic_info::loc_info) {
    Location const& loc = info.data.loc;
    if (loc)
      os << bright_white(loc) << ':';
  } else {
    Region const& reg = info.data.reg;
    if (reg)
      os << bright_white(reg) << ':';
  }
}


// The indent string used to show context.
char const* indent_ = "|    ";


// Show the line at which an error occurred along with a second
// line showing the context. The location must be valid.
//
// TODO: How much should we indent the context?
//
// TODO: Show line numbers in the context?
void
show_line(std::ostream& os, Location const& loc)
{
  Line const& line = loc.line();
  os << indent_ << line.str() << '\n';

  // Show the caret, but only if if the caret is valid.
  int caret = loc.column_number();
  if (caret < 0)
    return;
  os << indent_ << std::string(caret, ' ');
  os << bright_cyan('^') << '\n';
}


// TODO: What if we have multiple lines in the region?
void
show_region(std::ostream& os, Region const& reg)
{
  Line const& line = reg.line();
  os << indent_ << line.str() << '\n';

  // TODO: Do something better than this. We could print
  // all of the lines like this:
  //
  //  > line1
  //  > line2
  //  > line3
  //
  // Where '>' serves as the caret.
  if (reg.is_multiline()) {
    os << indent_ << "...";
    return;
  }

  // Show the underscore, but only if the start position
  // is valid.
  int start = reg.start_column_number() - 1;
  int end = reg.end_column_number() - 1;
  if (start < 0)
    return;
  os << indent_ << std::string(start, ' ');
  os << bright_cyan(std::string(end - start, '~')) << '\n';
}


} // namespace


std::ostream&
operator<<(std::ostream& os, Diagnostic_kind k)
{
  switch (k) {
  case error_diag: return os << bright_red("error");
  case warning_diag: return os << bright_magenta("warning");
  case note_diag: return os << bright_cyan("note");
  default: break;
  }
  lingo_unreachable("unknown diagnostic kind '{}'", (int)k);
}


void
show_context(std::ostream& os, Diagnostic_info const& info)
{
  if (info.kind == Diagnostic_info::loc_info) {
    Location const& loc = info.data.loc;
    if (loc) {
      show_line(os, loc);
    }
  } else {
    Region const& reg = info.data.reg;
    if (reg)
      show_region(os, reg);
  }
}



std::ostream&
operator<<(std::ostream& os, Diagnostic const& diag)
{
  os << diag.kind << ':';
  show_location(os, diag.info);
  os << ' ' << diag.msg << '\n';
  show_context(os, diag.info);
  return os;
}


namespace
{

// The diagnostic stack.
std::stack<Diagnostic_context*> diags_;


// The root diagnostic context. Note that this self-registers
// as the top diagnostic context.
Diagnostic_context root_;


} // namespace



Diagnostic::Diagnostic(Diagnostic_kind k, Location l, String const& m)
  : kind(k), info(l), msg(m)
{ }


Diagnostic::Diagnostic(Diagnostic_kind k, Region s, String const& m)
  : kind(k), info(s), msg(m)
{ }


Diagnostic_context::Diagnostic_context(bool suppress)
  : suppress_(suppress), errs_(0)
{
  diags_.push(this);
}


Diagnostic_context::~Diagnostic_context()
{
  diags_.pop();
}


// Emit a single diagnostic. If this context is suppressing
// diagnostics, then the save them for later.
void
Diagnostic_context::emit(Diagnostic const& diag)
{
  if (diag.kind == error_diag)
    ++ errs_;
  if (suppress_)
    push_back(diag);
  else
    std::cerr << diag;
}


// Reset the diagnotic context to a prinstine state.
void
Diagnostic_context::reset()
{
  clear();
  errs_ = 0;
}


// Emit all saved diagnostics. This does nothing if
// the context is not supporessing diagnostics.
void
Diagnostic_context::emit()
{
  if (suppress_)
    for (Diagnostic const& diag : *this)
      std::cerr << diag;
}


// Print all sved diagnostics. This is useful for
// replaying diagnostcs when suppressed.
void
emit_diagnostics()
{
  diags_.top()->emit();
}


// Rest the diagnotic context to a pristine state.
void
reset_diagnostics()
{
  diags_.top()->reset();
}


// Return the number of errors in the current diagnostic
// context.
int
error_count()
{
  return diags_.top()->errors();
}


// Emit an error diagnostic at the given source location.
void
error(Location loc, String const& msg)
{
  diags_.top()->emit({error_diag, loc, msg});
}


// Emit an error diagnostic over the given text span.
void
error(Region span, String const& msg)
{
  diags_.top()->emit({error_diag, span, msg});
}


// Emit a warning diagnostic at the given source location.
//
// TODO: Allow warnings to be treated as errors? This
// requires additional configuration information.
void
warning(Location loc, String const& msg)
{
  diags_.top()->emit({warning_diag, loc, msg});
}


// Emit a warning diagnost over the given text span.
void
warning(Region reg, String const& msg)
{
  diags_.top()->emit({warning_diag, reg, msg});
}



// Emit a note diagnostic.
//
// TODO: Allow the attachment of notes to other diagnostic
// objects, allowing them to be nested rather than flat.
void
note(Location loc, String const& msg)
{
  diags_.top()->emit({note_diag, loc, msg});
}


void
note(Region reg, String const& msg)
{
  diags_.top()->emit({note_diag, reg, msg});
}

} // namespace lingo
