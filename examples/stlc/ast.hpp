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

struct Type;
struct Base_type;
struct Arrow_type;

struct Expr;
struct Var;
struct Ref;
struct Def;
struct Decl;
struct Abs;
struct App;
struct Seq;
struct Visitor;


// -------------------------------------------------------------------------- //
// Types

// Represents an uninterpreted base type. Each type with
// a different name is unique. The set of types is:
//
//    t ::= x        -- uninterpreted base type
//          t1 -> t2 -- arrow types
struct Type
{
  struct Visitor;

  virtual ~Type()
  { }
};


struct Type::Visitor
{
  virtual void visit(Base_type const*) = 0;
  virtual void visit(Arrow_type const*) = 0;
};


// Uninterpreted base types.
struct Base_type : Type
{
  Base_type(Symbol const* n)
    : name_(n)
  { }

  Symbol const* name() const { return name_; }

  Symbol const* name_;
};


// Arrow types t1 -> t2.
struct Arrow_type : Type
{
  Arrow_type(Type const* t1, Type const* t2)
    : first(t1), second(t2)
  { }

  Type const* in() const { return first; }
  Type const* out() const { return first; }

  Type const* first;
  Type const* second;
};


Type const* get_base_type(Symbol const* n);
Type const* get_arrow_type(Type const*, Type const*);


// -------------------------------------------------------------------------- //
// Expressions

// The base class of all terms in the language. The set
// of expressions is:
//
//    e ::= x       -- variables/references
//          x = e;  -- definitions
//          x : t;  -- declarations
//          \x.e    -- abstractions
//          e1 e2   -- applications
//          e1 ; e2 -- sequences
struct Expr
{
  struct Visitor;

  Expr()
    : loc_(), type_()
  { }

  Expr(Location l)
    : loc_(l), type_()
  { }

  Expr(Location l, Type const* t)
    : loc_(l), type_(t)
  { }

  virtual ~Expr()
  { }

  virtual void accept(Visitor&) const = 0;

  Location     location() const { return loc_; }
  virtual Span span() const     { return {loc_, loc_}; }

  Type const* type() const              { return type_; }
  void        type(Type const* t) const { type_ = t; }

  Location            loc_;
  mutable Type const* type_;
};


// The expression visitor.
struct Expr::Visitor
{
  virtual void visit(Var const*) = 0;
  virtual void visit(Ref const*) = 0;
  virtual void visit(Def const*) = 0;
  virtual void visit(Decl const*) = 0;
  virtual void visit(Abs const*) = 0;
  virtual void visit(App const*) = 0;
  virtual void visit(Seq const*) = 0;
};


// A varaible.
struct Var : Expr
{
  Var(Symbol const* n, Type const* t)
    : Expr({}, t), name_(n)
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
    : Expr({}, v->type()), name_(s), var_(v)
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
    : Expr({}, v->type()), first(v), second(e)
  { }

  void accept(Visitor& v) const { return v.visit(this); }

  Var const*  var() const  { return first; }
  Expr const* expr() const { return second; }

  Var const* first;
  Expr const* second;
};


// The declaration of a named constant.
struct Decl : Expr
{
  Decl(Var const* v, Type const* t)
    : Expr({}, t), first(v)
  { }

  void accept(Visitor& v) const { return v.visit(this); }

  Var const*  var() const  { return first; }

  Var const* first;
};



// An expression abstracted over a variable.
struct Abs : Expr
{
  Abs(Var const* v, Expr const* e)
    : first(v), third(e)
  { }

  void accept(Visitor& v) const { return v.visit(this); }

  Var const*  var() const  { return first; }
  Type const* type() const { return second; }
  Expr const* expr() const { return third; }

  Var const*  first;
  Type const*  second;
  Expr const* third;
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
// Visitors

template<typename F, typename T>
struct Basic_type_visitor : Type::Visitor, Generic_visitor<F, T>
{
  Basic_type_visitor(F f)
    : Generic_visitor<F, T>(f)
  { }

  void visit(Base_type const* t) { this->invoke(t); }
  void visit(Arrow_type const* t) { this->invoke(t); }
};


template<typename F, typename T = typename std::result_of<F(Base_type const*)>::type>
inline T
apply(Type const* t, F fn)
{
  Basic_type_visitor<F, T> v(fn);
  return accept(t, v);
}


template<typename F, typename T>
struct Basic_expr_visitor : Expr::Visitor, Generic_visitor<F, T>
{
  Basic_expr_visitor(F f)
    : Generic_visitor<F, T>(f)
  { }

  void visit(Var const* e) { this->invoke(e); }
  void visit(Ref const* e) { this->invoke(e); }
  void visit(Def const* e) { this->invoke(e); }
  void visit(Decl const* e) { this->invoke(e); }
  void visit(Abs const* e) { this->invoke(e); }
  void visit(App const* e) { this->invoke(e); }
  void visit(Seq const* e) { this->invoke(e); }
};


template<typename F, typename T = typename std::result_of<F(Var const*)>::type>
inline T
apply(Expr const* e, F fn)
{
  Basic_expr_visitor<F, T> v(fn);
  return accept(e, v);
}


// -------------------------------------------------------------------------- //
// Facilities

void print(std::ostream&, Type const*);
void print(std::ostream&, Base_type const*);
void print(std::ostream&, Arrow_type const*);

void print(std::ostream&, Expr const*);
void print(std::ostream&, Var const*);
void print(std::ostream&, Ref const*);
void print(std::ostream&, Def const*);
void print(std::ostream&, Decl const*);
void print(std::ostream&, Abs const*);
void print(std::ostream&, App const*);
void print(std::ostream&, Seq const*);

std::ostream& operator<<(std::ostream&, Type const&);
std::ostream& operator<<(std::ostream&, Expr const&);

} // namespace calc

#endif
