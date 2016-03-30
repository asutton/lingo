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
  static String end = "end-of-input";
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
// Type parsing


Type const*
Parser::paren_type()
{
  require(lparen_tok);
  Type const* t = type();
  match(rparen_tok);
  return t;
}

// primary-type: base-type
//               '(' type ')'
Type const*
Parser::primary_type()
{
  if (Token tok = match_if(identifier_tok))
    return on_base_type(tok);
  if (lookahead() == lparen_tok)
    return paren_type();
  error(ts_.location(), "expected primary-type");
  throw Parse_error();
}


// arrow-type: base-type '->' arrow_type
//             type
Type const*
Parser::arrow_type()
{
  Type const* t = primary_type();
  if (match_if(arrow_tok))
    t = on_arrow_type(t, arrow_type());
  return t;
}


Type const*
Parser::type()
{
  return arrow_type();
}


// -------------------------------------------------------------------------- //
// Expression parsing

// var: id ':' type
Var const*
Parser::var()
{
  Token n = require(identifier_tok);
  match(colon_tok);
  Type const* t = type();
  return on_var(n, t);
}


Expr const*
Parser::id()
{
  Token tok = require(identifier_tok);
  return on_id(tok);
}


// def: id '=' expr
//
// TODO: Allow explicit specification of types?
Expr const*
Parser::def()
{
  Token n = require(identifier_tok);
  match(equal_tok);
  Expr const* e = expr();
  return on_def(n, e);
}


// decl: id ':' expr
Expr const*
Parser::decl()
{
  Var const* v = var();
  return on_decl(v);
}


Expr const*
Parser::abs()
{
  Environment env(*this);
  require(backslash_tok);
  Var const* v = var();
  match(dot_tok);
  Expr const* e = expr();
  return on_abs(v, e);
}


Expr const*
Parser::paren()
{
  require(lparen_tok);
  Expr const* e = expr();
  match(rparen_tok);
  return e;
}


// primary: identifier
//          identifier '=' expr
//          identifier ':' expr
//          '\' identifier '.' expr
//          '(' expr ')'
//
// TODO: Factor definitions and declarations into a
// top-level parse. They should most definitely not
// be primaries.
Expr const*
Parser::primary()
{
  if (lookahead() == identifier_tok) {
    if (lookahead(1) == equal_tok)
      return def();
    else if (lookahead(1) == colon_tok)
      return decl();
    else
      return id();
  }
  if (lookahead() == backslash_tok)
    return abs();
  if (lookahead() == lparen_tok)
    return paren();
  error(ts_.location(), "expected primary-expression");
  throw Parse_error();
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


// seq: seq ';' expr
//      expr [;]
//
// We allow a trailing semicolon for convenience.
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
  Expr const* e = seq();
  if (!ts_.eof()) {
    String msg = format("expected end-of-input but got '{}'", ts_.peek());
    error(ts_.location(), msg);
    throw Parse_error();
  }
  return e;
}


// -------------------------------------------------------------------------- //
//                            Parser function


Type const*
Parser::on_base_type(Token tok)
{
  return get_base_type(tok.symbol());
}


Type const*
Parser::on_arrow_type(Type const* t1, Type const* t2)
{
  return get_arrow_type(t1, t2);
}


Var const*
Parser::on_var(Token tok, Type const* t)
{
  Symbol const* sym = tok.symbol();
  Var* v = new Var(sym, t);
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
  error(ts_.location(), "no matching variable for '{}'", *sym);
  throw Name_error();
}


// Generate a definition. Note that this produces a
// variable whose type is that of `e`. Also note that
// definitions cannot be recursive since the type is
// deduced.
//
// TODO: Make this valid:
//
//    f : T->T;
//    f = \x.x;
//
// This would allow me to assign a type to the identifier
// before parsing and typing its definition.
Expr const*
Parser::on_def(Token tok, Expr const* e)
{
  Type const* t = e->type();
  Var const* v = on_var(tok, t);
  return new Def(v, e);
}


// The binding is created when the variable is parsed.
Expr const*
Parser::on_decl(Var const* v)
{
  return new Decl(v);
}


// G |- v:T1 ; G, v:T1 |- e : T2
// ----------------------------
//    G |- \v.e : T1 -> T2
Expr const*
Parser::on_abs(Var const* v, Expr const* e)
{
  Type const* t = get_arrow_type(v->type(), e->type());
  return new Abs(t, v, e);
}


// G |- e1 : T1 -> T2 ; G |- e2 : T1
// ---------------------------------
//        G |- e1 e2 : T2
//
// TODO: Improve diagnostics.
Expr const*
Parser::on_app(Expr const* e1, Expr const* e2)
{
  // e1 shall have arrow type.
  Arrow_type const* a = as<Arrow_type>(e1->type());
  if (!a) {
    error(ts_.location(), "expression does not have arrow type");
    throw Type_error();
  }
  Type const* t1 = a->in();
  Type const* t2 = a->out();

  // The type of e2 shall match t1.
  if (e2->type() != t1) {
    error(ts_.location(), "type mismatch in application");
    throw Type_error();
  }

  // The type of the expression shall be t2.
  return new App(t2, e1, e2);
}


// Types are ignored.
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
