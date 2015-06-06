
#ifndef CALC_PARSER_HPP
#define CALC_PARSER_HPP

#include "lingo/parsing.hpp"

namespace calc
{

using namespace lingo;

struct Expr;

// The parser is responsible for transforming a stream of tokens
// into nodes.
struct Parser
{
  using argument_type = Token;
  using result_type = Expr*;

  Expr* operator()(Token_stream&);

  Expr* on_error();
  Expr* on_int_expr(Token const*);
  Expr* on_unary_term(Token const*, Expr*);
  Expr* on_binary_term(Token const*, Expr*, Expr*);

  void on_expected_got(Location, Token const&, char const*);
  void on_expected_eof(Location, char const*);
};


Expr* parse(Token_stream&);

} // nammespace calc

#endif
