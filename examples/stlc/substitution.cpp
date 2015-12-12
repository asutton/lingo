// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "substitution.hpp"
#include "ast.hpp"


namespace calc
{

Expr const*
Substitution::operator()(Expr const* e) const
{
  return subst(e);
}


Expr const*
Substitution::subst(Expr const* e) const
{
  struct Fn
  {
    Substitution const& subst;
    Expr const* operator()(Var const* e) { return subst.subst(e); }
    Expr const* operator()(Ref const* e) { return subst.subst(e); }
    Expr const* operator()(Def const* e) { return subst.subst(e); }
    Expr const* operator()(Decl const* e) { return subst.subst(e); }
    Expr const* operator()(Abs const* e) { return subst.subst(e); }
    Expr const* operator()(App const* e) { return subst.subst(e); }
    Expr const* operator()(Seq const* e) { return subst.subst(e); }
  };
  return apply(e, Fn{*this});
}


// Substitute through a variable definition.
//
//    [x->s]y = y
//
// Variables are not substituted into.
Expr const*
Substitution::subst(Var const* e) const
{
  return e;
}


// Substitute a reference to a variable with its
// corresponding mapping.
//
//    [x->s]x = s
//    [x->s]y = y for all y != x
Expr const*
Substitution::subst(Ref const* e) const
{
  if (e->var()) {
    auto iter = find(e->var());
    if (iter != end())
      return iter->second;
  }
  return e;
}


// Substitute through a definition.
//
//    [x->s](def y = e) = def y = [x->s]e
//
// FIXME: I don't think this ever actually happens.
Expr const*
Substitution::subst(Def const* e) const
{
  lingo_unreachable();
}


Expr const*
Substitution::subst(Decl const* e) const
{
  lingo_unreachable();
}


Expr const*
Substitution::subst(Abs const* e) const
{
  Var const* v = cast<Var>(subst(e->var()));
  Expr const* d = subst(e->expr());
  return new Abs(e->type(), v, d);
}


Expr const*
Substitution::subst(App const* e) const
{
  Expr const* e1 = subst(e->fn());
  Expr const* e2 = subst(e->arg());
  return new App(e->type(), e1, e2);
}


Expr const*
Substitution::subst(Seq const* e) const
{
  Expr const* e1 = subst(e->left());
  Expr const* e2 = subst(e->right());
  return new App(e->type(), e1, e2);
}


} // namespace calc
