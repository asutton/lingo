
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


// -------------------------------------------------------------------------- //
//                                 Node kinds

// Different classes of expressions.
enum Kind
{
  int_expr,   // n, integer literals
  add_expr,   // n1 + n2
  sub_expr,   // n1 - n2
  mul_expr,   // n1 * n2
  div_expr,   // n1 / n2
  mod_expr,   // n1 % n2
  neg_expr,   // -n
  pos_expr,   // +n
};


char const* get_node_name(Kind);


// Adapt the generic node-kinding template to this node type.
template<Kind K>
using Node_kind = Kind_base<Kind, K>;


// -------------------------------------------------------------------------- //
//                                   Nodes

// The base class of all terms in the core.
struct Expr
{
  Expr(Kind k)
    : Expr(k, {})
  { }

  Expr(Kind k, Location l)
    : kind_(k), loc_(l)
  { }

  virtual ~Expr()
  { }

  char const* node_name() const { return get_node_name(kind_); }
  Kind        kind() const      { return kind_; }
  Location    location() const  { return loc_; }

  static Expr const* empty() { return nullptr; }
  static Expr const* error() { return make_error_node<Expr>(); }

  Kind     kind_;
  Location loc_;
};


// A helper class for constructing unary expressions. A unary
// expression contains the operator token and its operand.
template<Kind K>
struct Unary : Expr, Node_kind<K>
{
  Unary(Location loc, Expr const* e)
    : Expr(K, loc), first(e)
  { }

  Expr const* arg() const { return first; }

  Expr const* first;
};


// A helper class for constructing binary expressions. A binary
// expression contains the operator token and its two operands.
template<Kind K>
struct Binary : Expr, Node_kind<K>
{
  Binary(Location loc, Expr const* l, Expr const* r)
    : Expr(K, loc), first(l), second(r)
  { }

  Expr const* left() const { return first; }
  Expr const* right() const { return second; }

  Expr const* first;
  Expr const* second;
};


// An integer literal.
struct Int : Expr, Node_kind<int_expr>
{
  Int(Location loc, Integer n)
    : Expr(node_kind, loc), first(n)
  { }

  Integer value() const { return first; }

  Integer first;
};


// Addition of numbers.
struct Add : Binary<add_expr>
{
  using Binary<add_expr>::Binary;
};


// Subtraction of numbers.
struct Sub : Binary<sub_expr>
{
  using Binary<sub_expr>::Binary;
};


// Multiplication of numbers.
struct Mul : Binary<mul_expr>
{
  using Binary<mul_expr>::Binary;
};


// Quotient of division of numbers.
struct Div : Binary<div_expr>
{
  using Binary<div_expr>::Binary;
};


// Remainder of division numubers.
struct Mod : Binary<mod_expr>
{
  using Binary<mod_expr>::Binary;
};


// Negation of numbers.
struct Neg : Unary<neg_expr>
{
  using Unary<neg_expr>::Unary;
};


// Identity of numbers.
struct Pos : Unary<pos_expr>
{
  using Unary<pos_expr>::Unary;
};


// -------------------------------------------------------------------------- //
//                               Node accessors

Expr const* get_error();


// -------------------------------------------------------------------------- //
//                                Operations

Integer evaluate(Expr const*);

// -------------------------------------------------------------------------- //
//                                  Facilities

// Garbage collection
void mark(Expr const*);

// Pretty printing
void print(Printer&, Expr const*);
void debug(Printer&, Expr const*);

std::ostream& operator<<(std::ostream&, Expr const*);

} // namespace calc

#endif
