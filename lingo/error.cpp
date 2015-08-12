// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "lingo/error.hpp"

#include <iostream>
#include <stdexcept>
#include <stack>

namespace lingo
{

// Color support.
namespace
{

// FIXME: This is not especially good. We can probably do
// better in tems of selecting font colors. In fact, I'm
// sure this can be *much* better. 

// These are designed to be concatenated during preprocessing

#define font_start   "\033["
#define font_end     "\033[0m"

#define font_normal     "00" // No color, bold or underline
#define font_bold       "01" // Bold
#define font_underscore "04" // Underscore

#define font_black      "30" // Black text
#define font_red        "31" // Red text
#define font_green      "32" // Green text
#define font_yellow     "33" // Yellow text
#define font_blue       "34" // Blue text
#define font_magenta    "35" // Magenta text
#define font_cyan       "36" // Cyan text
#define font_white      "37" // White text


// Generate the common start of diagnostic headers.
#define fmt_diagnostic(color) font_start font_bold ";" font_ ## color "m"


// Start coloring font strings
#define font_error      fmt_diagnostic(red)
#define font_warning    fmt_diagnostic(magenta)
#define font_note       fmt_diagnostic(cyan)


// Coloring for source code locations.
#define font_location   font_start font_bold "m"
 
} // namespace


std::ostream&
operator<<(std::ostream& os, Diagnostic_kind k)
{
  switch (k) {
  case error_diag: return os << font_error "error" font_end;
  case warning_diag: return os << font_warning "warning" font_end;
  case note_diag: return os << font_note "note" << font_end;
  default: break;
  }
  lingo_unreachable("unknown diagnostic kind '{}'", (int)k);
}


std::ostream&
operator<<(std::ostream& os, Diagnostic const& diag)
{
  Bound_location const& loc = diag.loc;

  // Print the error header and general message.
  os << diag.kind << ':';
  if (loc.is_valid())
    os << font_location << loc << font_end << ": ";
  os << ' ' << diag.msg << '\n';

  // If we can, we should retrieve and print the line of
  // code indicated by the diagnostics.
  //
  // TODO: How much should we indent? Should we include line
  // numbers? Maybe some contextual information. It would surely
  // look nice.
  if (loc.is_valid()) {
    Line const& line = loc.line();
    os << "|    " << line.str() << '\n';
  } 

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


// Initialize the root diagnostic context.
//
// FIXME: This should go aways.
void
init_diagnostics()
{
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


// Construct a new error diagnostic for the given source
// location.
void
error(Bound_location loc, String const& msg)
{
  diags_.top()->emit({error_diag, loc, msg});
}


// Construct a new warning diagnostic for the given source
// location.
//
// TODO: Generate errors if warnings are being treated as
// errors, with some additional information about how to
// turn those off.
void
warning(Bound_location loc, String const& msg)
{
  diags_.top()->emit({warning_diag, loc, msg});
}


// Emit a note diagnostic.
//
// TODO: Allow the attachment of notes to other diagnostic 
// objects, allowing them to be nested rather than flat.
void 
note(Bound_location loc, String const& msg)
{
  diags_.top()->emit({note_diag, loc, msg});
}

} // namespace lingo
