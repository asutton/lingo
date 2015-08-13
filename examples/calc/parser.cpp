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

Expr const* parse_expression(Parser&, Token_stream&);


// -------------------------------------------------------------------------- //
//                          Primary expressions

using Nested_expr = Enclosed_term<Expr>;


// Parse an integer literal.
Expr const*
parse_integer_literal(Parser& p, Token_stream& toks)
{
  // Match integers.
  if (Token const* tok = match_if(toks, is_token(decimal_integer_tok)))
    return p.on_int_expression(tok);
  return nullptr;
}


// Parse a paren-enclosed expression.
Expr const*
parse_paren_expression(Parser& p, Token_stream& toks)
{
  // Match a nested sub-exprssion.
  if (Token const* open = match_token(toks, lparen_tok)) {

    // Check for empty parens so we can diagnos an error
    // more appropriately.
    if (next_token_is(toks, rparen_tok)) {
        error(open->location(), "expected expression after '('");
        return make_error_node<Expr>();
    }

    // Match the nested expression.
    if (Required<Expr> e = parse_expression(p, toks)) {
      if (expect_token(p, toks, rparen_tok)) {
        return *e;
      } else {
        // TODO: Show the location of the first brace.
        note(open->location(), "unmatched paren is here");
        return make_error_node<Expr>();
      }
    } else {
      return *e; // Propagate the error.
    }
  }
  return nullptr;
}


// Parse a primary expression.
//
//    primary-expression ::=
//        integer-literal
//      | '(' expression ')'
Expr const*
parse_primary_expression(Parser& p, Token_stream& toks)
{
  if (Nonempty<Expr> e = parse_integer_literal(p, toks))
    return *e;

  if (Nonempty<Expr> e = parse_paren_expression(p, toks))
    return *e;

  // If the expression was none of the above, the program 
  // is ill-formed. Emit a resaonable diagnostic.
  //
  // TODO: Maybe let fail gracefully so that the error gets
  // picked up at the lowest precedence parse?
  return nullptr;
}


// -------------------------------------------------------------------------- //
//                            Prefix expressions

// Parse a unary operator.
//
//    unary-operator ::= '+' | '-'
//
// Note that this is the same as additive-operator.
Token const*
parse_unary_operator(Parser& p, Token_stream& toks)
{
  extern Token const* parse_additive_operator(Parser&, Token_stream&);
  return parse_additive_operator(p, toks);
}


// Parse a unary epxression. A unary expressions is one
// that begins with an operator and is followed by a
// unary expression.
//
//    unary-expression ::=
//        primary-expression
//      | unary-operator unary-expression.
Expr const*
parse_unary_expression(Parser& p, Token_stream& toks)
{
  auto op = parse_unary_operator;
  auto sub = parse_primary_expression;
  auto act = [&](Token const* tok, Expr const* e) {
    return p.on_unary_expression(tok, e);
  };
  return parse_prefix_term(p, toks, op, sub, act);
}


// -------------------------------------------------------------------------- //
//                        Binary precedence parser

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
inline Token const*
parse_multiplicative_operator(Parser& p, Token_stream& toks)
{
  return match_if(toks, is_multiplicative_operator);
}


// Parse a multiplicative expression.
//
//    multiplicative-expression ::=
//        unary-expression
//      | multiplicative-expression multiplicative-operator unary-expression
Expr const*
parse_multiplicative_expression(Parser& p, Token_stream& toks)
{
  auto op = parse_multiplicative_operator;
  auto sub = parse_unary_expression;
  auto act = [&](Token const* tok, Expr const* e1, Expr const* e2) {
    return p.on_binary_expression(tok, e1, e2);
  };
  return parse_left_infix_term(p, toks, op, sub, act);
}


// Parse an additive operator.
//
//    additive-operator ::= '+' | '-'
inline Token const*
parse_additive_operator(Parser& p, Token_stream& toks)
{
  return match_if(toks, is_additive_operator);
}


// Parse an additive expression.
//
//    additive-expression ::=
//        multiplicative-expression
//      | additive-expression additive-operator multiplicative-expression
Expr const*
parse_additive_expression(Parser& p, Token_stream& toks)
{
  auto op = parse_additive_operator;
  auto sub = parse_multiplicative_expression;
  auto act = [&](Token const* tok, Expr const* e1, Expr const* e2) {
    return p.on_binary_expression(tok, e1, e2);
  };
  return parse_left_infix_term(p, toks, op, sub, act);
}


// Parse a binary expression. This is the top-level entry point 
// for the binary precedence parser.
inline Expr const*
parse_binary_expression(Parser& p, Token_stream& toks)
{
  return parse_additive_expression(p, toks);
}


// -------------------------------------------------------------------------- //
//                           Expression parser

// Parse an expression. 
Expr const*
parse_expression(Parser& p, Token_stream& toks)
{
  return parse_additive_expression(p, toks);
}


} // namespace


// -------------------------------------------------------------------------- //
//                            Parser function

Expr const*
Parser::on_int_expression(Token const* tok)
{
  return gc().make<Int>(tok->location(), as_integer(*tok));
}


Expr const*
Parser::on_unary_expression(Token const* tok, Expr const* e)
{
  Location loc = tok->location();
  switch (tok->kind()) {
  case plus_tok: return gc().make<Pos>(loc, e);
  case minus_tok: return gc().make<Neg>(loc, e);
  default:
    lingo_unreachable("invalid unary operator '{}'", tok->token_name());
  }
}


Expr const*
Parser::on_binary_expression(Token const* tok, Expr const* e1, Expr const* e2)
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


Expr const* 
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
  install_grammar(parse_unary_expression, "unary-expression");
  install_grammar(parse_multiplicative_expression, "multiplicative-expression");
  install_grammar(parse_additive_expression, "additive-expression");
  install_grammar(parse_expression, "expression");
}


} // namespace calc
