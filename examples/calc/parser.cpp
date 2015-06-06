
#include "parser.hpp"
#include "lexer.hpp"
#include "ast.hpp"

#include "lingo/memory.hpp"

#include <iostream>

namespace calc
{

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
    return p.on_int_expr(tok);
  
  if (next_token_is(toks, lparen_tok))
    return parse_paren_enclosed(p, toks, parse_expression, "expression");

  // If the expression was none of the above, the program is ill-formed.
  Location loc = toks.location();
  if (toks.eof())
    error(loc, "exected primary-expression, got end-of-file");
  else
    error(loc, "expected primary-expression, got '{}'", toks.peek());
  return p.on_error();
}


// -------------------------------------------------------------------------- //
//                            Prefix expressions


// Parse a prefix operator.
//
//    prefix-operator ::=
//      '+' | '-'
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
  return parse_prefix_term(p, toks, op, sub, "prefix-expression");
}


// -------------------------------------------------------------------------- //
//                            Precedence parser


// Parse a multiplicative operator.
//
//    multiplicative-operator ::=
//      '*' | '/' | '%'
//
// TODO: How do we generalize this. We really need the algorithm
// header to project the token kind so we can make these kinds
// of determinations. Same for additive operator and unary operator
// above.
Token const*
parse_multiplicative_operator(Parser& p, Token_stream& toks)
{
  // FIXME: I need a matching function for these kinds of things.
  if (toks.eof())
    return nullptr;
  
  Token_kind k = toks.peek().kind();
  if (star_tok <= k && k <= percent_tok)
    return &toks.get();
  return {};
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
  return parse_left_binary_term(p, toks, op, sub, "prefix-expression");
}


// Parse an additive operator.
//
//    additive-operator ::=
//      '+' | '-'
Token const*
parse_additive_operator(Parser& p, Token_stream& toks)
{
  // FIXME: This should be in a matching function.
  if (toks.eof())
    return nullptr;

  Token_kind k = toks.peek().kind();
  if (plus_tok <= k && k <= minus_tok)
    return &toks.get();
  return {};
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
  return parse_left_binary_term(p, toks, op, sub, "multiplicative-expression");
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


// Execute the parsing function.
Expr*
Parser::operator()(Token_stream& toks)
{
  if (toks.eof())
    return nullptr;
  return parse_expression(*this, toks);
}


Expr*
Parser::on_error()
{
  return get_error();
}


Expr*
Parser::on_int_expr(Token const* tok)
{
  return gc().make<Int>(tok->location(), as_integer(*tok));
}


Expr*
Parser::on_unary_term(Token const* tok, Expr* e)
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
Parser::on_binary_term(Token const* tok, Expr* e1, Expr* e2)
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


void
Parser::on_expected_got(Location loc, Token const& tok, char const* str)
{
  error(loc, "expected '{}' but got '{}'", str, tok);
}


void
Parser::on_expected_eof(Location loc, char const* str)
{
  error(loc, "expected '{}' but got eof", str);
}


Expr* 
parse(Token_stream& ts)
{
  Parser p;
  return p(ts);
}


} // namespace calc
