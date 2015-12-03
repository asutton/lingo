// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "ast.hpp"

#include "lingo/memory.hpp"
#include "lingo/error.hpp"

#include <iostream>

namespace calc
{

Span
Int::span() const
{
  // FIXME: We shouldn't need to render this to a string
  // just to see how long it is.
  String str = to_string(first);

  return {location(), Location(location().offset() + str.size())};
}


// Compute the span of a unary expression.
Span
Unary::span() const
{
  Location start = location();
  Location end = arg()->span().end_location();
  return {start, end};
}


// Compute the span of a binary expression.
Span
Binary::span() const
{
  Location start = left()->span().start_location();
  Location end = right()->span().end_location();
  return {start, end};
}


// -------------------------------------------------------------------------- //
//                                  Evaluations

// Dispatch table for evaluation.
struct Eval_fn
{
  Integer operator()(Int const* e) const
  {
    return e->value();
  }

  Integer operator()(Add const* e) const
  {
    return evaluate(e->left()) + evaluate(e->right());
  }

  Integer operator()(Sub const* e)
  {
    return evaluate(e->left()) - evaluate(e->right());
  }

  Integer operator()(Mul const* e)
  {
    return evaluate(e->left()) * evaluate(e->right());
  }

  Integer operator()(Div const* e)
  {
    return evaluate(e->left()) / evaluate(e->right());
  }

  Integer operator()(Mod const* e)
  {
    return evaluate(e->left()) % evaluate(e->right());
  }

  Integer operator()(Neg const* e)
  {
    return -evaluate(e->arg());
  }

  Integer operator()(Pos const* e)
  {
    return evaluate(e->arg());
  }
};


// Compute the integer evaluation of the expression.
Integer
evaluate(Expr const* e)
{
  return apply(e, Eval_fn());
}


// -------------------------------------------------------------------------- //
//                                  Printing


namespace
{

// Comptues the precedence of an expression.
// See the precedence table below.
struct Precedence_fn
{
  int operator()(Int const* e) const { return 0; }

  int operator()(Neg const* e) const { return 1; }
  int operator()(Pos const* e) const { return 1; }

  int operator()(Mul const* e) const { return 2; }
  int operator()(Div const* e) const { return 2; }
  int operator()(Mod const* e) const { return 2; }

  int operator()(Add const* e) const { return 3; }
  int operator()(Sub const* e) const { return 3; }
};


// Returns the precendence of the term e. The precedence of terms
// is given by the following table:
//
//    0   primary expressions
//    1   unary expressions
//    2   multiplicative expressoins
//    3   additive expressions
int
precedence(Expr const* e)
{
  return apply(e, Precedence_fn());
}


// A subexpression needs parens only when its prcedence is
// greater than that of the subexpression. Note that we could
// easily implement the following policies:
//
//    1. Use parens for less or equal precedence expressions.
//       This minimizes the use of parens.
//    2. Use parens for non-primary expressions.
//       This reflects the parse.
//    3. Always use parens.
//       Ths is unnecessarily verbose.
//
// This currently implements extended policy #2.
inline bool
needs_parens(Expr const* expr, Expr const* sub)
{
  return precedence(sub) != 0;
  // return precedence(expr) <= precedence(sub);
}


// Print a subexpression of an outer expression. This will automatically
// add parentheses if they are needed (and omit them if they are not).
inline void
print_subexpr(Printer& p, Expr const* expr, Expr const* sub)
{
  if (needs_parens(expr, sub))
    print_paren_enclosed(p, sub);
  else
    print(p, sub);
}


// Return the character representing the operator of an expression.
struct Operator_fn
{
  char operator()(Int const* e) const { lingo_unreachable(); }
  char operator()(Add const* e) const { return '+'; }
  char operator()(Sub const* e) const { return '-'; }
  char operator()(Mul const* e) const { return '*'; }
  char operator()(Div const* e) const { return '/'; }
  char operator()(Mod const* e) const { return '%'; }
  char operator()(Neg const* e) const { return '-'; }
  char operator()(Pos const* e) const { return '+'; }
};


// Prints the textual representation of an operator for
// that node.
inline void
print_operator(Printer& p, Expr const* e)
{
  print(p, apply(e, Operator_fn()));
}


void
print(Printer& p, Int const* e)
{
  print(p, e->value());
}


template<typename T>
typename std::enable_if<is_binary<T>(), void>::type
print(Printer& p, T const* e)
{
  print_subexpr(p, e, e->left());
  print_space(p);
  print_operator(p, e);
  print_space(p);
  print_subexpr(p, e, e->right());
}


template<typename T>
typename std::enable_if<is_unary<T>(), void>::type
print(Printer& p, T const* e)
{
  print_operator(p, e);
  print_subexpr(p, e, e->arg());
}



struct Print_fn
{
  Print_fn(Printer& p)
    : p(p)
  { }

  void operator()(Int const* e) const { print(p, e); }
  void operator()(Add const* e) const { print(p, e); }
  void operator()(Sub const* e) const { print(p, e); }
  void operator()(Mul const* e) const { print(p, e); }
  void operator()(Div const* e) const { print(p, e); }
  void operator()(Mod const* e) const { print(p, e); }
  void operator()(Neg const* e) const { print(p, e); }
  void operator()(Pos const* e) const { print(p, e); }

  Printer& p;
};


} // namespace


// Pretty print the given expression.
void
print(Printer& p, Expr const* e)
{
  lingo_assert(is_valid_node(e));
  apply(e, Print_fn(p));
}


// -------------------------------------------------------------------------- //
// Debug printing


namespace
{

struct Debug_fn
{
  Debug_fn(Printer& p)
    : p(p)
  { }

  void operator()(Int const* e) const { debug(p, e); }
  void operator()(Add const* e) const { debug(p, e); }
  void operator()(Sub const* e) const { debug(p, e); }
  void operator()(Mul const* e) const { debug(p, e); }
  void operator()(Div const* e) const { debug(p, e); }
  void operator()(Mod const* e) const { debug(p, e); }
  void operator()(Neg const* e) const { debug(p, e); }
  void operator()(Pos const* e) const { debug(p, e); }

  Printer& p;
};


} // namespace


// Emit a debug representation of the given expression.
void
debug(Printer& p, Expr const* e)
{
  if (!e) {
    debug(p, "<null>");
    return;
  }

  if (is_error_node(e)) {
    debug(p, "<error>");
    return;
  }

  apply(e, Debug_fn(p));
}


std::ostream&
operator<<(std::ostream& os, Expr const* e)
{
  return os << to_string(e);
}


} // namespace calc

