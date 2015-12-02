// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "parser.hpp"
#include "lexer.hpp"
#include "ast.hpp"

#include <lingo/error.hpp>

#include <iostream>


namespace calc
{


// Denotes a parse error.
struct Parse_error : std::runtime_error
{
  Parse_error()
    : std::runtime_error("")
  { }
};


namespace
{

// Returns a spelling for the current token. If the token
// stream is at the end of input, then the spelling will
// reflect that state.
String const&
token_spelling(Token_stream& ts)
{
  static String end = "end-of-file";
  if (ts.eof())
    return end;
  else
    return ts.peek().spelling();
}

} // namespace


// Returns the first token of lookahead.
Token_kind
Parser::lookahead() const
{
  return Token_kind(ts_.peek().kind());
}


// Returns the nth token of lookahead.
Token_kind
Parser::lookahead(int n) const
{
  return Token_kind(ts_.peek(n).kind());
}


Token
Parser::match(Token_kind k)
{
  if (lookahead() == k)
    return ts_.get();

  String msg = format("expected '{}' but got '{}'", 
                      get_spelling(k), 
                      token_spelling(ts_));
  error(ts_.location(), msg);
  throw Parse_error();
}


// If the current token matches k, return the token
// and advance the stream. Otherwise, return an
// invalid token.
//
// Note that invalid tokens evaluate to false.
Token
Parser::match_if(Token_kind k)
{
  if (lookahead() == k)
    return ts_.get();
  else
    return Token();
}


// Require a token of the given kind. Behavior is
// udefined if the token does not match.
Token
Parser::require(Token_kind k)
{
  assert(lookahead() == k);
  return ts_.get();
}


// Returns the current token and advances the
// underlying token stream.
Token
Parser::accept()
{
  return ts_.get();
}


// -------------------------------------------------------------------------- //
//                          Primary expressions


// Parse a paren-enclosed expression.
//
// FIXME: Improve diagnostics for matched parens.
Expr const*
Parser::paren()
{
  this->require(lparen_tok);
  Expr const* e = expr();
  match(rparen_tok);
  return e;
}


// Parse a primary expression.
//
//    primary-expression ::=
//        integer-literal
//      | '(' expression ')'
Expr const*
Parser::primary()
{
  if (Token tok = match_if(integer_tok))
    return on_int(tok);
  if (lookahead() == lparen_tok)
    return paren();
  error(ts_.location(), "expected primary-expression");
  throw Parse_error();
}


// Parse a unary epxression. A unary expressions is one
// that begins with an operator and is followed by a
// unary expression.
//
//    unary-expression ::=
//        primary-expression
//      | unary-operator unary-expression.
Expr const*
Parser::unary()
{
  if (Token tok = match_if(plus_tok))
    return on_unary(tok, unary());
  if (Token tok = match_if(minus_tok))
    return on_unary(tok, unary());
  return primary();
}


// Parse a multiplicative expression.
//
//    multiplicative-expression ::=
//        unary-expression
//      | multiplicative-expression multiplicative-operator unary-expression
Expr const*
Parser::multiplicative()
{
  Expr const* e = unary();
  while (true) {
    if (Token tok = match_if(star_tok))
      e = on_binary(tok, e, unary());
    else if (Token tok = match_if(slash_tok))
      e = on_binary(tok, e, unary());
    else if (Token tok = match_if(percent_tok))
      e = on_binary(tok, e, unary());
    else
      break;
  }
  return e;
}


// Parse an additive expression.
//
//    additive-expression ::=
//        multiplicative-expression
//      | additive-expression additive-operator multiplicative-expression
Expr const*
Parser::additive()
{
  Expr const* e = multiplicative();
  while (true) {
    if (Token tok = match_if(plus_tok))
      e = on_binary(tok, e, multiplicative());
    else if (Token tok = match_if(minus_tok))
      e = on_binary(tok, e, multiplicative());
    else
      break;
  }
  return e;
}


// Parse a binary expression. This is the top-level entry point 
// for the binary precedence parser.
inline Expr const*
Parser::binary()
{
  return additive();
}


// Parse an expression. 
Expr const*
Parser::expr()
{
  return binary();
}


// -------------------------------------------------------------------------- //
//                            Parser function

Expr const*
Parser::on_int(Token tok)
{
  return new Int(tok.location(), tok.integer_symbol()->value());
}


Expr const*
Parser::on_unary(Token tok, Expr const* e)
{
  Location loc = tok.location();
  switch (tok.kind()) {
    case plus_tok: return new Pos(loc, e);
    case minus_tok: return new Neg(loc, e);
    default: break;
  }
  lingo_unreachable("invalid unary operator", tok.spelling());
}


Expr const*
Parser::on_binary(Token tok, Expr const* e1, Expr const* e2)
{
  Location loc = tok.location();
  switch (tok.kind()) {
    case plus_tok: return new Add(loc, e1, e2);
    case minus_tok: return new Sub(loc, e1, e2);
    case star_tok: return new Mul(loc, e1, e2);
    case slash_tok: return new Div(loc, e1, e2);
    case percent_tok: return new Mod(loc, e1, e2);
    default: break;
  }
  lingo_unreachable("invalid binary operator '{}'", tok.spelling());
}


Expr const*
Parser::operator()()
{
  if (ts_.eof())
    return nullptr;
  return expr();
}


// Parse the given buffer.
Expr const* 
parse(String const& str)
{
  Buffer buf(str);  
  Token_stream ts;
  Character_stream cs(buf);
  Lexer lex(cs, ts);
  Parser parse(ts);

  // Lex.
  lex();
  if (error_count()) {
    reset_diagnostics();
    return make_error_node<Expr>();
  }

  // Parse.
  Expr const* expr = parse();
  if (error_count()) {
    reset_diagnostics();
    return make_error_node<Expr>();
  }

  return expr;
}

} // namespace calc
