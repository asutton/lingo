
#ifndef CALC_PARSER_HPP
#define CALC_PARSER_HPP

#include "lingo/parsing.hpp"
#include "lingo/buffer.hpp"

namespace calc
{

using namespace lingo;

struct Expr;
struct Error;

// The parser is responsible for transforming a stream of tokens
// into nodes. The parser owns a reference to the buffer for its
// tokens. This supports the resolution of source code locations.
struct Parser
{
  using argument_type = Token;
  using result_type = Expr*;

  Parser(Buffer& b)
    : buf(b)
  { }

  Expr* operator()(Token_stream&);

  Expr* on_error();
  Expr* on_int_expr(Token const*);
  Expr* on_unary_term(Token const*, Expr*);
  Expr* on_binary_term(Token const*, Expr*, Expr*);

  Expr* on_enclosure(Token const*, Token const*);
  Expr* on_enclosure(Token const*, Token const*, Expr*);

  Error* on_expected(char const*);
  Error* on_expected(Location, char const*);
  Error* on_expected(Location, char const*, Token const&);

  Buffer const& buffer() const { return buf; }

  Buffer& buf;
};


Expr* parse(Buffer&, Token_stream&);

} // nammespace calc

#endif
