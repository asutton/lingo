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
  Expr const* on_int_expression(Token const*);
  Expr const* on_unary_expression(Token const*, Expr const*);
  Expr const* on_binary_expression(Token const*, Expr const*, Expr const*);
};


Expr const* parse(Token_stream&);
Expr const* parse(Buffer&);
Expr const* parse(std::string const&);


void init_grammar();


} // nammespace calc


#endif
