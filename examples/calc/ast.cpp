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
  std::stringstream ss;
  ss << val_;
  return {loc_, Location(loc_.offset() + ss.str().size())};
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
//    1. Natural parens: precedence(expr) > precedence(sub)
//       Parens are used only when needed to disambiguate
//       operators with different precedence. Example:
//
//          (3 * 2) + 4  ~>  3 * 2 + 4
//
//    2. Nested parens: precedence(expr) >= precedence(sub)
//       Like above, except that parens are also added for
//       subexpressions of like precedence.
//
//          (3 * 2) + 4  ~>  (3 * 2) + 4
//
//    3. Max parens: precedence(expr) != 0
//       Parens are used everywhere except literals.
//
//          (3 * 2) + (4 * 2)  ~>  (3 * 2) + (4 * 2)
//
//    4. Always parens: true.
//
//          3  ~>  (3)
//
// This currently implements extended policy #3.
inline bool
needs_parens(Expr const* expr, Expr const* sub)
{
  return precedence(sub) != 0;
}


// Output streaming for nested sub-expressions.
struct subexpr
{
  subexpr(Expr const* e, Expr const* s)
    : e(e), s(s)
  { }

  Expr const* e;
  Expr const* s;
};


std::ostream&
operator<<(std::ostream& os, subexpr sub)
{
  if (needs_parens(sub.e, sub.s))
    os << '(' << *sub.s << ')';
  else
    os << *sub.s;
  return os;
}


// Output streaming for the operator name.
struct opname
{
  opname(Expr const* e)
    : e(e)
  { }

  Expr const* e;
};


std::ostream&
operator<<(std::ostream& os, opname op)
{
  struct Fn
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
  return os << apply(op.e, Fn{});
}


} // namespace


void
print(std::ostream& os, Int const* e)
{
  os << e->value();
}


void
print(std::ostream& os, Binary const* e)
{
  os << subexpr(e, e->left())
     << ' ' << opname(e) << ' '
     << subexpr(e, e->right());

}


void
print(std::ostream& os, Unary const* e)
{
  os << opname(e) << subexpr(e, e->arg());
}


// Pretty print the given expression.
void
print(std::ostream& os, Expr const* e)
{
  struct Fn
  {
    void operator()(Int const* e) const { print(os, e); }
    void operator()(Binary const* e) const { print(os, e); }
    void operator()(Unary const* e) const { print(os, e); }

    std::ostream& os;
  };
  apply(e, Fn{os});
}


std::ostream&
operator<<(std::ostream& os, Expr const& e)
{
  print(os, &e);
  return os;
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


} // namespace calc
