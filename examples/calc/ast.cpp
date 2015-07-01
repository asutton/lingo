
#include "ast.hpp"

#include "lingo/memory.hpp"
#include "lingo/error.hpp"

#include <iostream>

namespace calc
{

// Returns a textual representation of the node's name.
char const*
get_node_name(Kind k)
{
  switch (k) {
  case int_expr: return "int_expr";
  case add_expr: return "add_expr";
  case sub_expr: return "sub_expr";
  case mul_expr: return "mul_expr";
  case div_expr: return "div_expr";
  case mod_expr: return "mod_expr";
  case neg_expr: return "neg_expr";
  case pos_expr: return "pos_expr";

  default:
    lingo_unreachable("unhandled node kind ({})", (int)k);
  }
}


// -------------------------------------------------------------------------- //
//                               Node accessors

// Returs an error expression.
Expr* 
get_error()
{
  return make_error_node<Expr>();
}


// -------------------------------------------------------------------------- //
//                                  Evaluations

inline Integer
eval_int(Int const* e)
{
  return e->value();
}


inline Integer
eval_add(Add const* e) 
{
  return evaluate(e->left()) + evaluate(e->right());
}


inline Integer
eval_sub(Sub const* e)
{
  return evaluate(e->left()) - evaluate(e->right());
}


inline Integer
eval_mul(Mul const* e)
{
  return evaluate(e->left()) * evaluate(e->right());
}


inline Integer
eval_div(Div const* e)
{
  return evaluate(e->left()) / evaluate(e->right());
}


inline Integer
eval_mod(Mod const* e)
{
  return evaluate(e->left()) % evaluate(e->right());
}


inline Integer
eval_neg(Neg const* e)
{
  return -evaluate(e->arg());
}


inline Integer
eval_pos(Pos const* e)
{
  return evaluate(e->arg());
}


// Compute the integer evaluation of the expression.
Integer
evaluate(Expr const* e)
{
  lingo_assert(e);
  switch (e->kind()) {
  case int_expr: return eval_int(cast<Int>(e));
  case add_expr: return eval_add(cast<Add>(e));
  case sub_expr: return eval_sub(cast<Sub>(e));
  case mul_expr: return eval_mul(cast<Mul>(e));
  case div_expr: return eval_div(cast<Div>(e));
  case mod_expr: return eval_mod(cast<Mod>(e));
  case neg_expr: return eval_neg(cast<Neg>(e));
  case pos_expr: return eval_pos(cast<Pos>(e));
  
  default:
    lingo_unreachable("unevaluated node '{}'", e->node_name());
  }
}


// -------------------------------------------------------------------------- //
//                               Garbage collection

void
mark(Expr* e)
{
  if (!e)
    return;
  if (is_error_node(e))
    return;
  switch (e->kind()) {
  case int_expr: return mark_unary(cast<Int>(e));
  case add_expr: return mark_binary(cast<Add>(e));
  case sub_expr: return mark_binary(cast<Sub>(e));
  case mul_expr: return mark_binary(cast<Mul>(e));
  case div_expr: return mark_binary(cast<Div>(e));
  case mod_expr: return mark_binary(cast<Mod>(e));
  case neg_expr: return mark_unary(cast<Neg>(e));
  case pos_expr: return mark_unary(cast<Pos>(e));
  
  default:
    lingo_unreachable("unevaluated node '{}'", e->node_name());
  }
}

// -------------------------------------------------------------------------- //
//                                  Printing

namespace
{

inline void
print_int(Printer& p, Int const* e)
{
  print(p, e->value());
}


inline void
print_operator(Printer& p, Expr const* e)
{
  switch (e->kind()) {
  case add_expr:
  case pos_expr:
    print(p, '+'); break;

  case sub_expr: 
  case neg_expr:
    print(p, '-'); break;
  
  case mul_expr:
    print(p, '*'); break;
  
  case div_expr:
    print(p, '/'); break;
  
  case mod_expr: 
    print(p, '%'); break;

  default:
    lingo_unreachable("unhandled node '{}'", e->node_name());
  }
}


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
  switch (e->kind()) {
  case int_expr:
    return 0;

  case add_expr:
  case sub_expr:
    return 3;

  case mul_expr:
  case div_expr:
  case mod_expr:
    return 2;

  case neg_expr:
  case pos_expr:
    return 1;

  default:
    lingo_unreachable("precedence undefined for '{}'", e->node_name());
  }
}

// A subexpression needs parens only when its prcedence is
// greater than that of the subexpression. Note that we could
// easily implement the following policies:
//
//    1. Also use parens for equal precence expressions
//    2. Only use parens for primary expressions
//    3. Others?
//
// This currently implements extended policy #1.
inline bool
needs_parens(Expr const* expr, Expr const* sub)
{
  return precedence(expr) <= precedence(sub);
}


// Print a subexpression of an outer expression. This will automatically
// add parentheses if they are needed (and omit them if they are not).
inline void
print_subexpr(Printer& p, Expr const* expr, Expr* sub)
{
  if (needs_parens(expr, sub))
    print_paren_enclosed(p, sub);
  else
    print(p, sub);
}


template<typename T>
void
print_binary(Printer& p, T const* e)
{
  print_subexpr(p, e, e->left());
  print_space(p);
  print_operator(p, e);
  print_space(p);
  print_subexpr(p, e, e->right());
}


template<typename T>
void
print_unary(Printer& p, T const* e)
{
  print_operator(p, e);
  print_subexpr(p, e, e->arg());
}

} // namespace


// Pretty print the given expression.
void 
print(Printer& p, Expr const* e)
{
  lingo_assert(is_valid_node(e));
  switch (e->kind()) {
  case int_expr: return print_int(p, cast<Int>(e));
  case add_expr: return print_binary(p, cast<Add>(e));
  case sub_expr: return print_binary(p, cast<Sub>(e));
  case mul_expr: return print_binary(p, cast<Mul>(e));
  case div_expr: return print_binary(p, cast<Div>(e));
  case mod_expr: return print_binary(p, cast<Mod>(e));
  case neg_expr: return print_unary(p, cast<Neg>(e));
  case pos_expr: return print_unary(p, cast<Pos>(e));

  default:
    lingo_unreachable("unhandled node '{}'", e->node_name());
  }
}


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
  
  switch (e->kind()) {
  case int_expr: return debug_unary(p, cast<Int>(e));
  case add_expr: return debug_binary(p, cast<Add>(e));
  case sub_expr: return debug_binary(p, cast<Sub>(e));
  case mul_expr: return debug_binary(p, cast<Mul>(e));
  case div_expr: return debug_binary(p, cast<Div>(e));
  case mod_expr: return debug_binary(p, cast<Mod>(e));
  case neg_expr: return debug_unary(p, cast<Neg>(e));
  case pos_expr: return debug_unary(p, cast<Pos>(e));

  default:
    lingo_unreachable("unhandled node '{}'", e->node_name());
  }
}


std::ostream&
operator<<(std::ostream& os, Expr const* e)
{
  return os << to_string(e);
}


} // namespace calc

