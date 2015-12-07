// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "ast.hpp"

#include "lingo/memory.hpp"
#include "lingo/error.hpp"

#include <iostream>
#include <set>


namespace calc
{


// -------------------------------------------------------------------------- //
// Types


struct Type_less
{
  bool operator()(Type const& a, Type const& b) const
  {
    std::less<Symbol const*> cmp;
    return cmp(a.name(), b.name());
  }
};


Type const*
get_type(Symbol const* s)
{
  std::set<Type, Type_less> types_;
  auto iter = types_.emplace(s);
  return &*iter.first;
}


// -------------------------------------------------------------------------- //
// Printing

void
print(std::ostream& os, Type const* t)
{
  os << *t->name() << '\n';
}


void
print(std::ostream& os, Var const* e)
{
  os << *e->name();
}


void
print(std::ostream& os, Ref const* e)
{
  os << *e->name();
}


void
print(std::ostream& os, Def const* e)
{
   os << *e->var() << " = " << *e->expr();
}


void
print(std::ostream& os, Decl const* e)
{
   os << *e->var() << " : " << *e->type();
}


void
print(std::ostream& os, Abs const* e)
{
  os << '\\' << *e->var() << '.' << *e->expr();
}


void
print(std::ostream& os, App const* e)
{
  os << '(' << *e->fn() << ' ' << *e->arg() << ')';
}


void
print(std::ostream& os, Seq const* e)
{
  os << *e->left() << ";\n" << *e->right();
}


void
print(std::ostream& os, Expr const* e)
{
  struct Fn
  {
    std::ostream& os;
    void operator()(Var const* e) { print(os, e); }
    void operator()(Ref const* e) { print(os, e); }
    void operator()(Def const* e) { print(os, e); }
    void operator()(Decl const* e) { print(os, e); }
    void operator()(Abs const* e) { print(os, e); }
    void operator()(App const* e) { print(os, e); }
    void operator()(Seq const* e) { print(os, e); }
  };
  return apply(e, Fn{os});
}


std::ostream&
operator<<(std::ostream& os, Expr const& e)
{
  print(os, &e);
  return os;
}


std::ostream&
operator<<(std::ostream& os, Type const& t)
{
  print(os, &t);
  return os;
}


} // namespace calc

