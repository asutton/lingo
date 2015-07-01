// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#ifndef CALC_PARSER_HPP
#define CALC_PARSER_HPP

#include "lingo/parsing.hpp"
#include "lingo/buffer.hpp"

namespace calc
{

using namespace lingo;

struct Expr;


// The parser is responsible for transforming a stream of tokens
// into nodes. The parser owns a reference to the buffer for its
// tokens. This supports the resolution of source code locations.
struct Parser
{
  using argument_type = Token;
  using result_type = Expr*;

  Expr* on_int_expression(Token const*);
  Expr* on_prefix_expression(Token const*, Expr*);
  Expr* on_infix_expression(Token const*, Expr*, Expr*);

  Expr* on_enclosure(Token const*, Token const*);
  Expr* on_enclosure(Token const*, Token const*, Expr*);

  Expr* on_error();
  Expr* on_expected(char const*);
  Expr* on_expected(Location, char const*);
  Expr* on_expected(Location, char const*, Token const&);
};

void init_grammar();

Expr* parse(Token_stream&);

} // nammespace calc

#endif
