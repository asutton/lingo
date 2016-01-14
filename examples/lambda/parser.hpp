// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#ifndef CALC_PARSER_HPP
#define CALC_PARSER_HPP

#include "lexer.hpp"
#include "ast.hpp"

#include <lingo/environment.hpp>

namespace calc
{

using namespace lingo;


// Denotes a parse error.
struct Parse_error : std::runtime_error
{
  using std::runtime_error::runtime_error;
};


// The naming environment associates names with their
// definitions. Note that a symbol can be bound to either
// a variable (Var) or definition (Def).
using Name_map = Environment<Symbol const*, Var const*>;
using Name_binding = Name_map::Binding;
using Name_stack = Stack<Name_map>;


// The parser is responsible for transforming a stream of tokens
// into nodes. The parser owns a reference to the buffer for its
// tokens. This supports the resolution of source code locations.
struct Parser
{
  Parser(Token_stream& ts)
    : ts_(ts)
  { }

  Expr const* operator()();

  Var const* var();
  Expr const* id();
  Expr const* def();
  Expr const* abs();
  Expr const* paren();
  Expr const* primary();
  Expr const* app();
  Expr const* postfix();
  Expr const* seq();
  Expr const* binary();
  Expr const* expr();

  Var const* on_var(Token);
  Expr const* on_id(Token);
  Expr const* on_def(Var const*, Expr const*);
  Expr const* on_abs(Var const*, Expr const*);
  Expr const* on_app(Expr const*, Expr const*);
  Expr const* on_seq(Expr const*, Expr const*);


  // Parsing support.
  Token_kind lookahead() const;
  Token_kind lookahead(int) const;
  Token      match(Token_kind);
  Token      match_if(Token_kind);
  Token      require(Token_kind);
  Token      accept();

  Token_stream& ts_;
  Name_stack    names_;

  // Name binding support.
  struct Environment {
    Environment(Parser& p)
      : parser(p)
    {
      parser.names_.push();
    }

    ~Environment()
    {
      parser.names_.pop();
    }

    Parser& parser;
  };
};


Expr const* parse(String const&);


} // namespace calc

#endif
