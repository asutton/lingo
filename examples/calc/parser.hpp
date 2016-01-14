// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#ifndef CALC_PARSER_HPP
#define CALC_PARSER_HPP

#include "lexer.hpp"

namespace calc
{

using namespace lingo;


struct Expr;


// Denotes a parse error.
struct Parse_error : std::runtime_error
{
  using std::runtime_error::runtime_error;
};


// The parser is responsible for transforming a stream of tokens
// into nodes. The parser owns a reference to the buffer for its
// tokens. This supports the resolution of source code locations.
struct Parser
{
  Parser(Token_stream& ts)
    : ts_(ts)
  { }

  Expr const* operator()();

  Expr const* paren();
  Expr const* primary();
  Expr const* unary();
  Expr const* multiplicative();
  Expr const* additive();
  Expr const* binary();
  Expr const* expr();

  Expr const* on_int(Token);
  Expr const* on_unary(Token, Expr const*);
  Expr const* on_binary(Token, Expr const*, Expr const*);

  Token_kind lookahead() const;
  Token_kind lookahead(int) const;
  Token      match(Token_kind);
  Token      match_if(Token_kind);
  Token      require(Token_kind);
  Token      accept();

  Token_stream& ts_;
};


Expr const* parse(String const&);


} // namespace calc

#endif
