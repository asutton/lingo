// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "step.hpp"
#include "ast.hpp"
#include "parser.hpp"

namespace calc
{

Expr const* step(Expr const*);
Expr const* next(Expr const*);


// Reduce each operand in turn. Evaluation proceeds left-to-right.
template<typename T>
Expr const* 
step_binary(T const* e)
{
  Location loc = e->location();
  Expr const* e1 = e->left();
  Expr const* e2 = e->right();
  if (!is<Int>(e1))
    return new T(loc, step(e1), e2);
  if (!is<Int>(e2))
    return new T(loc, e1, step(e2));
  return new Int(loc, evaluate(e));
}


// Reduce the operand.
template<typename T>
Expr const*
step_unary(T const* e)
{
  Location loc = e->location();
  Expr const* e0 = e->arg();
  if (!is<Int>(e0))
    return new T(loc, step(e0));
  return new Int(loc, evaluate(e));
}


// Dispatch table for evaluation.
struct Step_fn
{
  Expr const* operator()(Int const* e) const { return e; }
  Expr const* operator()(Add const* e) const { return step_binary(e); }
  Expr const* operator()(Sub const* e) const { return step_binary(e); }
  Expr const* operator()(Mul const* e) const { return step_binary(e); }
  Expr const* operator()(Div const* e) const { return step_binary(e); }
  Expr const* operator()(Mod const* e) const { return step_binary(e); }
  Expr const* operator()(Neg const* e) const { return step_unary(e); }
  Expr const* operator()(Pos const* e) const { return step_unary(e); }
};


// Compute the integer evaluation of the expression.
Expr const*
step(Expr const* e)
{
  return apply(e, Step_fn());
}


// Search for the next operand to be reduced.
template<typename T>
Expr const* 
next_binary(T const* e)
{
  Expr const* e1 = e->left();
  Expr const* e2 = e->right();
  if (!is<Int>(e1))
    return next(e1);
  if (!is<Int>(e2))
    return next(e2);
  return e;
}


// Search for the next opeand to be reduced.
template<typename T>
Expr const*
next_unary(T const* e)
{
  Expr const* e0 = e->arg();
  if (!is<Int>(e0))
    return next(e0);
  return e;
}


// Dispatch table for evaluation.
struct Next_fn
{
  Expr const* operator()(Int const* e) const { return nullptr; }
  Expr const* operator()(Add const* e) const { return next_binary(e); }
  Expr const* operator()(Sub const* e) const { return next_binary(e); }
  Expr const* operator()(Mul const* e) const { return next_binary(e); }
  Expr const* operator()(Div const* e) const { return next_binary(e); }
  Expr const* operator()(Mod const* e) const { return next_binary(e); }
  Expr const* operator()(Neg const* e) const { return next_unary(e); }
  Expr const* operator()(Pos const* e) const { return next_unary(e); }
};


// Returns the next subexpression to be evaluated. Returns
// nullptr if `e` is fully reduced.
Expr const*
next(Expr const* e)
{
  return apply(e, Next_fn());
}


// Iterate through the evaluation of the expression, showing
// which expressions are being evaluated.
Expr const*
step_eval(Expr const* e)
{
  do {
    // Rebuild the input context around the current
    // expression by rendering and re-parsing it. This
    // guarantees that output will be emitted using the
    // correct buffer.
    Buffer buf = to_string(e);
    Input_context cxt(buf);
    e = parse(buf);

    // Select the sub-expression being evaluated.
    note(next(e)->span(), "evaluating");

    // Perform that evaluation.
    e = step(e);

  } while (!is<Int>(e));
  print(e);
  return e;
}


} // namespace
