// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#ifndef CALC_HPP
#define CALC_HPP

#include "lingo/integer.hpp"
#include "lingo/node.hpp"
#include "lingo/token.hpp"
#include "lingo/print.hpp"
#include "lingo/debug.hpp"

namespace calc
{

using namespace lingo;
using lingo::print;
using lingo::debug;

struct Expr;
struct Int;
struct Add;
struct Sub;
struct Mul;
struct Div;
struct Mod;
struct Neg;
struct Pos;
struct Visitor;

// -------------------------------------------------------------------------- //
//                                   Nodes


// The base class of all terms in the core.
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

  Location location() const  { return loc_; }

  Location loc_;
};


// The expression visitor.
struct Visitor
{
  virtual void visit(Int const*) = 0;
  virtual void visit(Add const*) = 0;
  virtual void visit(Sub const*) = 0;
  virtual void visit(Mul const*) = 0;
  virtual void visit(Div const*) = 0;
  virtual void visit(Mod const*) = 0;
  virtual void visit(Neg const*) = 0;
  virtual void visit(Pos const*) = 0;
};


// A helper class for constructing unary expressions. A unary
// expression contains the operator token and its operand.
struct Unary : Expr
{
  Unary(Location loc, Expr const* e)
    : Expr(loc), first(e)
  { }

  Expr const* arg() const { return first; }

  Expr const* first;
};


// A helper class for constructing binary expressions. A binary
// expression contains the operator token and its two operands.
struct Binary : Expr
{
  Binary(Location loc, Expr const* l, Expr const* r)
    : Expr(loc), first(l), second(r)
  { }

  Expr const* left() const { return first; }
  Expr const* right() const { return second; }

  Expr const* first;
  Expr const* second;
};


// An integer literal.
struct Int : Expr
{
  Int(Location loc, Integer n)
    : Expr(loc), first(n)
  { }

  void accept(Visitor& v) const { v.visit(this); }

  Integer value() const { return first; }

  Integer first;
};


// Addition of numbers.
struct Add : Binary
{
  using Binary::Binary;

  void accept(Visitor& v) const { v.visit(this); }
};


// Subtraction of numbers.
struct Sub : Binary
{
  using Binary::Binary;

  void accept(Visitor& v) const { v.visit(this); }
};


// Multiplication of numbers.
struct Mul : Binary
{
  using Binary::Binary;

  void accept(Visitor& v) const { v.visit(this); }
};


// Quotient of division of numbers.
struct Div : Binary
{
  using Binary::Binary;

  void accept(Visitor& v) const { v.visit(this); }
};


// Remainder of division numubers.
struct Mod : Binary
{
  using Binary::Binary;

  void accept(Visitor& v) const { v.visit(this); }
};


// Negation of numbers.
struct Neg : Unary
{
  using Unary::Unary;

  void accept(Visitor& v) const { v.visit(this); }
};


// Identity of numbers.
struct Pos : Unary
{
  using Unary::Unary;

  void accept(Visitor& v) const { v.visit(this); }
};


// -------------------------------------------------------------------------- //
//                                Term classifies

// Returns true if T is a unary expression.
template<typename T>
constexpr bool is_unary()
{
  return std::is_base_of<Unary, T>::value;
}


// Returns true if T is a binary expression.
template<typename T>
constexpr bool is_binary()
{
  return std::is_base_of<Binary, T>::value;
}


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

  void visit(Int const* e) { this->invoke(e); }
  void visit(Add const* e) { this->invoke(e); }
  void visit(Sub const* e) { this->invoke(e); }
  void visit(Mul const* e) { this->invoke(e); }
  void visit(Div const* e) { this->invoke(e); }
  void visit(Mod const* e) { this->invoke(e); }
  void visit(Neg const* e) { this->invoke(e); }
  void visit(Pos const* e) { this->invoke(e); }
};


// Apply the function f to the type t.
// The return type is that of the function object F.
template<typename F, typename T = typename std::result_of<F(Int*)>::type>
inline T
apply(Expr const* e, F fn)
{
  Basic_visitor<F, T> v(fn);
  return accept(e, v);
}


// -------------------------------------------------------------------------- //
//                                Operations

Integer evaluate(Expr const*);


// -------------------------------------------------------------------------- //
//                                  Facilities

// Pretty printing
void print(Printer&, Expr const*);


// Debug printing
void debug(Printer&, Expr const*);

std::ostream& operator<<(std::ostream&, Expr const*);

} // namespace calc

#endif
