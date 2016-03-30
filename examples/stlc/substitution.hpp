// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#ifndef CALC_SUBSTITUTION_HPP
#define CALC_SUBSTITUTION_HPP

#include "ast.hpp"

#include <unordered_map>

namespace calc
{


// This class represents the lexical substitution of an
// an expression for an argument.

// Note that the actual substitution rules are defined as
// an application of this object as a function.
struct Substitution : std::unordered_map<Var const*, Expr const*>
{
  using std::unordered_map<Var const*, Expr const*>::unordered_map;

  Expr const* operator()(Expr const*) const;

  Expr const* subst(Expr const*) const;
  Expr const* subst(Var const*) const;
  Expr const* subst(Ref const*) const;
  Expr const* subst(Def const*) const;
  Expr const* subst(Decl const*) const;
  Expr const* subst(Abs const*) const;
  Expr const* subst(App const*) const;
  Expr const* subst(Seq const*) const;
};


} // namespace calc

#endif
