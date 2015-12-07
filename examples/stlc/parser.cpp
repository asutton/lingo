// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "parser.hpp"
#include "lexer.hpp"
#include "ast.hpp"

#include <lingo/error.hpp>

#include <iostream>


namespace calc
{


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
  throw Parse_error("match");
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
// Parsing


Type const*
Parser::type()
{
  Token tok = match(identifier_tok);
  return on_type(tok);
}


Var const*
Parser::var()
{
  Token tok = require(identifier_tok);
  return on_var(tok);
}


Expr const*
Parser::id()
{
  Token tok = require(identifier_tok);
  return on_id(tok);
}


Expr const*
Parser::def()
{
  Var const* v = var();
  require(equal_tok);
  Expr const* e = expr();
  return on_def(v, e);
}


Expr const*
Parser::abs()
{
  Environment env(*this);
  require(backslash_tok);
  Var const* v = var();
  match(colon_tok);
  Type const* t = type();
  match(dot_tok);
  Expr const* e = expr();
  return on_abs(v, t, e);
}


Expr const*
Parser::paren()
{
  this->require(lparen_tok);
  Expr const* e = expr();
  match(rparen_tok);
  return e;
}


// primary: identifier
//          identifier '=' expr
//          identifier ':' expr
//          '\' identifier '.' expr
//          '(' expr ')'
Expr const*
Parser::primary()
{
  if (lookahead() == identifier_tok) {
    if (lookahead(1) == equal_tok)
      return def();
    else
      return id();
  }
  if (lookahead() == backslash_tok)
    return abs();
  if (lookahead() == lparen_tok)
    return paren();
  error(ts_.location(), "expected primary-expression");
  throw Parse_error("primary");
}


Expr const*
Parser::postfix()
{
  Expr const* e = primary();
  while (true) {
    // We have an application only when the lookahead
    // indicates the start of a new primary expression.
    if (lookahead() == identifier_tok ||
        lookahead() == backslash_tok ||
        lookahead() == lparen_tok)
      e = on_app(e, primary());
    else
      break;
  }
  return e;
}



Expr const*
Parser::expr()
{
  return postfix();
}


Expr const*
Parser::seq()
{
  Expr const* e = postfix();
  while (true) {
    if (match_if(semicolon_tok)) {
      if (ts_.eof())
        return e;
      e = on_seq(e, expr());
    }
    else
      break;
  }
  return e;
}


Expr const*
Parser::operator()()
{
  Environment env(*this);
  if (ts_.eof())
    return nullptr;
  return seq();
}



// -------------------------------------------------------------------------- //
//                            Parser function


Type const*
Parser::on_type(Token tok)
{
  return get_type(tok.symbol());
}


Var const*
Parser::on_var(Token tok)
{
  Symbol const* sym = tok.symbol();
  Var* v = new Var(sym);
  names_.bind(sym, v);
  return v;
}


// Return a reference to the bound variable (if bound)
// or simply the symbol (if unbound).
Expr const*
Parser::on_id(Token tok)
{
  Symbol const* sym = tok.symbol();
  if (Name_binding const* bind = names_.lookup(sym))
    return new Ref(sym, bind->second);
  else
    return new Ref(sym);
}


Expr const*
Parser::on_def(Var const* v, Expr const* e)
{
  return new Def(v, e);
}


Expr const*
Parser::on_abs(Var const* v, Type const* t, Expr const* e)
{
  return new Abs(v, t, e);
}


Expr const*
Parser::on_app(Expr const* e1, Expr const* e2)
{
  return new App(e1, e2);
}


Expr const*
Parser::on_seq(Expr const* e1, Expr const* e2)
{
  return new Seq(e1, e2);
}


// Parse the given buffer.
Expr const* 
parse(String const& str)
{
  Buffer buf(str);  
  Character_stream cs(buf);
  Token_stream ts(buf);
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
