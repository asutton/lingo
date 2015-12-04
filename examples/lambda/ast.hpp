// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#ifndef CALC_AST_HPP
#define CALC_AST_HPP

#include "lingo/integer.hpp"
#include "lingo/node.hpp"
#include "lingo/token.hpp"
#include "lingo/print.hpp"


namespace calc
{

using namespace lingo;

struct Var;
struct Ref;
struct Def;
struct Abs;
struct App;
struct Seq;
struct Visitor;

// -------------------------------------------------------------------------- //
//                                   Nodes


// The base class of all terms in the language. The set
// of expressions is:
//
//    e ::= x       -- variables/references
//          x = e;  -- definitions
//          \x.e    -- abstractions
//          e1 e2   -- applications
//          e1 ; e2 -- sequences
struct Expr
{
  Expr()
    : loc_()
  { }

  Expr(Location l)
    : loc_(l)
  { }

  virtual ~Expr()
  { }

  virtual void accept(Visitor&) const = 0;

  Location     location() const { return loc_; }
  virtual Span span() const     { return {loc_, loc_}; }

  Location loc_;
};


// The expression visitor.
struct Visitor
{
  virtual void visit(Var const*) = 0;
  virtual void visit(Ref const*) = 0;
  virtual void visit(Def const*) = 0;
  virtual void visit(Abs const*) = 0;
  virtual void visit(App const*) = 0;
  virtual void visit(Seq const*) = 0;
};


// A varaible.
struct Var : Expr
{
  Var(Symbol const* n)
    : name_(n)
  { }

  void accept(Visitor& v) const { return v.visit(this); }

  Symbol const* name() const { return name_; }

  Symbol const* name_;
};


// A reference to a variable.
struct Ref : Expr
{
  Ref(Symbol const* s)
    : name_(s), var_(nullptr)
  { }

  Ref(Symbol const* s, Var const* v)
    : name_(s), var_(v)
  { }

  void accept(Visitor& v) const { return v.visit(this); }

  Symbol const* name() const { return name_; }
  Var const*    var() const  { return var_; }

  Symbol const* name_;
  Var const*    var_;
};



// The definition of a variable.
struct Def : Expr
{
  Def(Var const* v, Expr const* e)
    : first(v), second(e)
  { }

  void accept(Visitor& v) const { return v.visit(this); }

  Var const*  var() const  { return first; }
  Expr const* expr() const { return second; }

  Var const* first;
  Expr const* second;
};


// An expression abstracted over a variable.
struct Abs : Expr
{
  Abs(Var const* v, Expr const* e)
    : first(v), second(e)
  { }

  void accept(Visitor& v) const { return v.visit(this); }

  Var const*  var() const  { return first; }
  Expr const* expr() const { return second; }

  Var const*  first;
  Expr const* second;
};


// The application of an argument to a variable.
struct App : Expr
{
  App(Expr const* e1, Expr const* e2)
    : first(e1), second(e2)
  { }

  void accept(Visitor& v) const { return v.visit(this); }

  Expr const* fn() const  { return first; }
  Expr const* arg() const { return second; }

  Expr const* first;
  Expr const* second;
};


// A sequencing of expressions.
struct Seq : Expr
{
  Seq(Expr const* e1, Expr const* e2)
    : first(e1), second(e2)
  { }

  void accept(Visitor& v) const { return v.visit(this); }

  Expr const* left() const  { return first; }
  Expr const* right() const { return second; }

  Expr const* first;
  Expr const* second;
};


// -------------------------------------------------------------------------- //
//                                Generic visitor

// A parameterized visitor that dispatches to a function 
// object. F is the type of the function and T is its
// return type.
//
// This class is never used directly. It is used only in
// the apply function below.
template<typename F, typename T>
struct Basic_visitor : Visitor, Generic_visitor<F, T>
{
  Basic_visitor(F f)
    : Generic_visitor<F, T>(f)
  { }

  void visit(Var const* e) { this->invoke(e); }
  void visit(Ref const* e) { this->invoke(e); }
  void visit(Def const* e) { this->invoke(e); }
  void visit(Abs const* e) { this->invoke(e); }
  void visit(App const* e) { this->invoke(e); }
  void visit(Seq const* e) { this->invoke(e); }
};


// Apply the function f to the type t.
// The return type is that of the function object F.
template<typename F, typename T = typename std::result_of<F(Var const*)>::type>
inline T
apply(Expr const* e, F fn)
{
  Basic_visitor<F, T> v(fn);
  return accept(e, v);
}


// -------------------------------------------------------------------------- //
// Facilities

void print(std::ostream&, Expr const*);
void print(std::ostream&, Def const*);
void print(std::ostream&, Var const*);
void print(std::ostream&, Ref const*);
void print(std::ostream&, Abs const*);
void print(std::ostream&, App const*);
void print(std::ostream&, Seq const*);

std::ostream& operator<<(std::ostream&, Expr const&);

} // namespace calc

#endif
