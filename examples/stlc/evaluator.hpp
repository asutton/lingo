// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#ifndef CALC_EVALUATOR_HPP
#define CALC_EVALUATOR_HPP

#include "ast.hpp"

#include <lingo/environment.hpp>


namespace calc
{


using namespace lingo;


// Maintains value bindings for definitions.
using Value_map = Environment<Var const*, Expr const*>;
using Value_binding = Value_map::Binding;


struct Expr;


// The evaluator...
struct Evaluator
{
  Expr const* operator()(Expr const*);

  Expr const* eval(Expr const*);
  Expr const* eval(Var const*);
  Expr const* eval(Ref const*);
  Expr const* eval(Def const*);
  Expr const* eval(Decl const*);
  Expr const* eval(Abs const*);
  Expr const* eval(App const*);
  Expr const* eval(Seq const*);

  Value_map defs_;
};



} // namespace calc

#endif
