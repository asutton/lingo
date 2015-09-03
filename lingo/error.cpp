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


// Coloring for source code locations and carets
#define font_location   font_start font_bold "m"
#define font_caret      font_start font_bold ";" font_cyan "m"
 

// Print the source code location for a bound location or
// span.
void
show_location(std::ostream& os, Diagnostic_info const& info)
{
  if (info.kind == Diagnostic_info::loc_info) {
    Bound_location const& loc = info.data.loc;
    if (loc.is_valid())
      os << font_location << loc << font_end << ':';
  } else {
    Bound_span const& span = info.data.span;
    if (span.is_valid())
      os << font_location << span << font_end << ':';
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
show_line(std::ostream& os, Bound_location const& loc)
{
  Line const& line = loc.line();
  os << indent_ << line.str() << '\n';
  
  // Show the caret, but only if if the caret is valid.
  int caret = loc.column_no() - 1;
  if (caret < 0)
    return;
  os << indent_ << std::string(caret, ' ');
  os << font_caret << '^' << font_end << '\n';
}


// TODO: What if we have multiple lines in the span?
void
show_span(std::ostream& os, Bound_span const& span)
{
  Line const& line = span.line();
  os << indent_ << line.str() << '\n';

  // TODO: Do something better than this. We could print
  // all of the lines like this:
  //
  //  > line1
  //  > line2
  //  > line3
  //
  // Where '>' serves as the caret.
  if (span.is_multiline()) {
    os << indent_ << "...";
    return;
  }

  // Show the underscore, but only if the start position
  // is valid.
  int start = span.start_column_no() - 1;
  int end = span.end_column_no() - 1;
  if (start < 0)
    return;
  os << indent_ << std::string(start, ' ');
  os << font_caret << std::string(end - start, '~') << font_end << '\n';
}


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


void
show_context(std::ostream& os, Diagnostic_info const& info)
{
  if (info.kind == Diagnostic_info::loc_info) {
    Bound_location const& loc = info.data.loc;
    if (loc.is_valid()) {
      show_line(os, loc);
    }
  } else {
    Bound_span const& span = info.data.span;
    if (span.is_valid())
      show_span(os, span);
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



Diagnostic::Diagnostic(Diagnostic_kind k, Bound_location l, String const& m)
  : kind(k), info(l), msg(m)
{ }


Diagnostic::Diagnostic(Diagnostic_kind k, Bound_span s, String const& m)
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
error(Bound_location loc, String const& msg)
{
  diags_.top()->emit({error_diag, loc, msg});
}


// Emit an error diagnostic over the given text span.
void
error(Bound_span span, String const& msg)
{
  diags_.top()->emit({error_diag, span, msg});
}


// Emit a warning diagnostic at the given source location.
//
// TODO: Allow warnings to be treated as errors? This
// requires additional configuration information.
void
warning(Bound_location loc, String const& msg)
{
  diags_.top()->emit({warning_diag, loc, msg});
}


// Emit a warning diagnost over the given text span.
void
warning(Bound_span span, String const& msg)
{
  diags_.top()->emit({warning_diag, span, msg});
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


void
note(Bound_span span, String const& msg)
{
  diags_.top()->emit({note_diag, span, msg});
}

} // namespace lingo
