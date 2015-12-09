// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "ast.hpp"

#include "lingo/memory.hpp"
#include "lingo/error.hpp"

#include <iostream>
#include <set>
#include <typeindex>


namespace calc
{

// -------------------------------------------------------------------------- //
// Ordering

bool is_less(Type const*, Type const*);


bool
is_less(Base_type const* a, Base_type const* b)
{
  std::less<Symbol const*> cmp;
  return cmp(a->name(), b->name());
}


bool
is_less(Arrow_type const* a, Arrow_type const* b)
{
  if (is_less(a->first, b->first))
    return true;
  if (is_less(b->first, a->first))
    return false;
  return is_less(a->second, b->second);
}


bool
is_less(Type const* a, Type const* b)
{
  struct Fn
  {
    Type const* b;
    bool operator()(Base_type const* a) const 
    { 
      return is_less(a, as<Base_type>(b)); 
    }
    bool operator()(Arrow_type const* a) const 
    { 
      return is_less(a, as<Arrow_type>(b)); 
    }
  };

  std::type_index t1 = typeid(*a);
  std::type_index t2 = typeid(*b);
  if (t1 < t2)
    return true;
  if (t2 < t1)
    return false;
  lingo_assert(t1 == t2);
  return apply(a, Fn{b});
}


// -------------------------------------------------------------------------- //
// Types


struct Type_less
{
  template<typename T>
  bool operator()(T const& a, T const& b) const
  {
    return is_less(&a, &b);
  }
};


Type const*
get_base_type(Symbol const* sym)
{
  static std::set<Base_type, Type_less> s;
  auto iter = s.emplace(sym);
  return &*iter.first;
}


Type const*
get_arrow_type(Type const* t1, Type const* t2)
{
  static std::set<Arrow_type, Type_less> s;
  auto iter = s.emplace(t1, t2);
  return &*iter.first;
}


// -------------------------------------------------------------------------- //
// Printing

void
print(std::ostream& os, Base_type const* t)
{
  os << *t->name();
}


void
print(std::ostream& os, Arrow_type const* t)
{
  os << *t->in() << " -> " << *t->out();
}


void
print(std::ostream& os, Type const* t)
{
  struct Fn
  {
    std::ostream& os;
    void operator()(Base_type const* t) { print(os, t); }
    void operator()(Arrow_type const* t) { print(os, t); }
  };
  apply(t, Fn{os});
}


void
print(std::ostream& os, Var const* e)
{
  os << *e->name() << ':' << *e->type();
  // os << *e->name() << '\n';
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
   os << *e->var();
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
