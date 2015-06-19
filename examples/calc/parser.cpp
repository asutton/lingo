// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "parser.hpp"
#include "lexer.hpp"
#include "ast.hpp"

#include "lingo/token.hpp"
#include "lingo/memory.hpp"

#include <iostream>

namespace calc
{

using lingo::print;
using lingo::debug;

namespace
{


Expr* parse_expression(Parser&, Token_stream&);


// -------------------------------------------------------------------------- //
//                          Primary expressions


// Parse a primary expression.
//
//    primary-expression ::=
//        integer-literal
//      | '(' expression ')'
Expr*
parse_primary_expression(Parser& p, Token_stream& toks)
{
  if (Token const* tok = match_if(toks, is_token(decimal_integer_tok)))
    return p.on_int_expression(tok);

  if (next_token_is(toks, lparen_tok))
    return parse_paren_enclosed(p, toks, parse_expression);

  // If the expression was none of the above, the program is ill-formed.
  if (toks.eof())
    error(Location::none, "exected primary-expression, got end-of-file");
  else
    error("expected primary-expression, got '{}'", toks.peek());
  return p.on_error();
}


// -------------------------------------------------------------------------- //
//                            Prefix expressions


// Parse a prefix operator.
//
//    prefix-operator ::= '+' | '-'
//
// Note that this is the same as additive-operator.
Token const*
parse_prefix_operator(Parser& p, Token_stream& toks)
{
  extern Token const* parse_additive_operator(Parser&, Token_stream&);
  return parse_additive_operator(p, toks);
}


// Parse a prefix epxression. A prefix expressions is one
// that begins with an operator and is followed by a
// prefix expression.
//
//    prefix-expression ::=
//        primary-expression
//      | prefix-operator prefix-expression.
Expr*
parse_prefix_expression(Parser& p, Token_stream& toks)
{
  auto op = parse_prefix_operator;
  auto sub = parse_primary_expression;
  return parse_prefix_expression(p, toks, op, sub);
}


// -------------------------------------------------------------------------- //
//                            Precedence parser

// Returns true iff tok is one of the multiplicative operators.
inline bool
is_multiplicative_operator(Token const& tok)
{
  return tok.kind() == star_tok 
      || tok.kind() == slash_tok
      || tok.kind() == percent_tok;
}


// Returns true iff tok is one of the additive operators.
inline bool
is_additive_operator(Token const& tok)
{
  return tok.kind() == plus_tok 
      || tok.kind() == minus_tok;
}


// Parse a multiplicative operator.
//
//    multiplicative-operator ::= '*' | '/' | '%'
//
// TODO: How do we generalize this. We really need the algorithm
// header to project the token kind so we can make these kinds
// of determinations. Same for additive operator and unary operator
// above.
Token const*
parse_multiplicative_operator(Parser& p, Token_stream& toks)
{
  if (Token const* tok = match_if(toks, is_multiplicative_operator))
    return tok;
  return nullptr;
}


// Parse a multiplicative expression.
//
//    multiplicative-expression:
//      unary-expression
//      multiplicative-expression multiplicative-operator unary-expression
Expr*
parse_multiplicative_expression(Parser& p, Token_stream& toks)
{
  auto op = parse_multiplicative_operator;
  auto sub = parse_prefix_expression;
  return parse_left_infix_expression(p, toks, op, sub);
}


// Parse an additive operator.
//
//    additive-operator ::= '+' | '-'
Token const*
parse_additive_operator(Parser& p, Token_stream& toks)
{
  if (Token const* tok = match_if(toks, is_additive_operator))
    return tok;
  return nullptr;
}


// Parse an additive expression.
//
//    additive-expression ::=
//        multiplicative-expression
//      | additive-expression additive-operator multiplicative-expression
inline Expr*
parse_additive_expression(Parser& p, Token_stream& toks)
{
  auto op = parse_additive_operator;
  auto sub = parse_multiplicative_expression;
  return parse_left_infix_expression(p, toks, op, sub);
}


// -------------------------------------------------------------------------- //
//                           Expression parser

// Parse an expression. 
Expr*
parse_expression(Parser& p, Token_stream& toks)
{
  return parse_additive_expression(p, toks);
}

} // namespace


// -------------------------------------------------------------------------- //
//                            Parser function


Expr*
Parser::on_error()
{
  return get_error();
}


Expr*
Parser::on_int_expression(Token const* tok)
{
  return gc().make<Int>(tok->location(), as_integer(*tok));
}


Expr*
Parser::on_prefix_expression(Token const* tok, Expr* e)
{
  Location loc = tok->location();
  switch (tok->kind()) {
  case plus_tok: return gc().make<Pos>(loc, e);
  case minus_tok: return gc().make<Neg>(loc, e);
  default:
    lingo_unreachable("invalid unary operator '{}'", tok->token_name());
  }
}


Expr*
Parser::on_infix_expression(Token const* tok, Expr* e1, Expr* e2)
{
  Location loc = tok->location();
  switch (tok->kind()) {
  case plus_tok: return gc().make<Add>(loc, e1, e2);
  case minus_tok: return gc().make<Sub>(loc, e1, e2);
  case star_tok: return gc().make<Mul>(loc, e1, e2);
  case slash_tok: return gc().make<Div>(loc, e1, e2);
  case percent_tok: return gc().make<Mod>(loc, e1, e2);
  default:
    lingo_unreachable("invalid binary operator '{}'", tok->token_name());
  }
}


// Do not allow empty parens.
Expr*
Parser::on_enclosure(Token const* left, Token const* right)
{
  error(left->location(), "empty nested expression");
  return get_error();
}


// Just return the enclosed expression.
Expr*
Parser::on_enclosure(Token const* left, Token const* right, Expr* mid)
{
  return mid;
}


Error*
Parser::on_expected(char const* str)
{
  error(Location::none, "expected '{}' but got end-of-file", str);
  return get_error();
}


Error*
Parser::on_expected(Location loc, char const* str, Token const& tok)
{
  error(loc, "expected '{}' but got '{}'", str, tok);
  return get_error();
}


Error*
Parser::on_expected(Location loc, char const* str)
{
  error(loc, "expected '{}'", str);
  return get_error();
}


Expr* 
parse(Token_stream& ts)
{
  Parser p;
  if (ts.eof())
    return nullptr;
  else
    return parse_expression(p, ts);
}


// Initialize the grammar production rules.
void
init_grammar()
{
  install_grammar(parse_primary_expression, "primary-expression");
  install_grammar(parse_prefix_expression, "prefix-expression");
  install_grammar(parse_multiplicative_expression, "multiplicative-expression");
  install_grammar(parse_additive_expression, "additive-expression");
  install_grammar(parse_expression, "expression");
}


} // namespace calc
