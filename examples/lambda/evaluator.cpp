// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "evaluator.hpp"
#include "substitution.hpp"

#include <iostream>
#include <stdexcept>

namespace calc
{

Expr const*
Evaluator::operator()(Expr const* e)
{
  return eval(e);
}


Expr const*
Evaluator::eval(Expr const* e)
{
  struct Fn
  {
    Evaluator& eval;
    Expr const* operator()(Var const* e) { return eval.eval(e); }
    Expr const* operator()(Ref const* e) { return eval.eval(e); }
    Expr const* operator()(Def const* e) { return eval.eval(e); }
    Expr const* operator()(Abs const* e) { return eval.eval(e); }
    Expr const* operator()(App const* e) { return eval.eval(e); }
    Expr const* operator()(Seq const* e) { return eval.eval(e); }
  };
  return apply(e, Fn{*this});
}


Expr const*
Evaluator::eval(Var const* e)
{
  return e;
}


// Evaluate a reference to a variable. If the referenced
// variable is a definition, then substitute the bound
// expression.
//
//     x = e in S
//    ------------ E-ref-1
//    S |- x ->* e
//
//    x not in S
//    ------------ E-ref-2
//    S |- x ->* x
Expr const*
Evaluator::eval(Ref const* e)
{
  if (Var const* v = e->var())
    if (Value_binding const* b = defs_.lookup(v))
      return b->second;
  return e;
}


// Evaluating a definition does not produce a value.
//
// FIXME: This should produce result \x.x, which could
// be interpreted as the unit value.
Expr const*
Evaluator::eval(Def const* e)
{
  defs_.bind(e->var(), e->expr());
  return nullptr;
}


// An abstraction is a value.
Expr const*
Evaluator::eval(Abs const* e)
{
  return e;
}


// Evaluate an application.
//
//        t1 ->* \x:T.t
//    --------------------- E-app-1
//    t1 t2 ->* (\x:T.t) t2
//
//          t2 ->* v
//    --------------------- E-app-2
//    \x:T.t t2 ->* [x->v]t
Expr const*
Evaluator::eval(App const* e)
{
  Abs const* fn = as<Abs>(eval(e->fn()));
  if (!fn) {
    String msg = format("application of non-abstraction '{}'", *e->fn());
    throw std::runtime_error(msg);
  }
  Expr const* arg = eval(e->arg());

  // Sbustitute the argument into the abstraction.
  Substitution subst {
    {fn->var(), arg}
  };
  Expr const* result = subst(fn->expr());

  // And recursively evaluate.
  return eval(result);
}


// Evaluate a sequence.
//
//       S |- e1 ->* v
//    ------------------- E-seq-1
//    e1 ; e2 ->* v ; e2
//
//       S |- e2 ->* v
//    ------------------- E-seq-2
//    v ; e2 ->* v
//
// Note that the result of the left operand
// is discarded.
Expr const*
Evaluator::eval(Seq const* e)
{
  // Print the result of the left operand (if it's not null).
  if (Expr const* v = eval(e->left()))
    std::cout << *v << '\n';
  return eval(e->right());
}


} // namespace calc
