
#ifndef CALC_LEXER_HPP
#define CALC_LEXER_HPP

// The calc lexer defines the additional tokens used by the 
// language and the machine used to recognize those tokens 
// in input source.

#include "lingo/character.hpp"
#include "lingo/lexing.hpp"

#include <string>

namespace calc
{

using namespace lingo;

// -------------------------------------------------------------------------- //
//                              Tokens

enum Token_kind
{
  lparen_tok,
  rparen_tok,
  plus_tok,
  minus_tok,
  star_tok,
  slash_tok,
  percent_tok,
  integer_tok,
};


void init_tokens();


// -------------------------------------------------------------------------- //
//                              Elaboration

Integer as_integer(Token const&);


// -------------------------------------------------------------------------- //
//                              Lexing


// The Lexer is a facility that translates sequences of
// characters into tokens. This is primarily a callback
// interface for the lexing function for the language.
struct Lexer
{
  using argument_type = char;
  using result_type = Token;

  // Semantic actions.
  Token on_lparen(Location, char const*);
  Token on_rparen(Location, char const*);

  Token on_plus(Location, char const*);
  Token on_minus(Location, char const*);
  Token on_star(Location, char const*);
  Token on_slash(Location, char const*);
  Token on_percent(Location, char const*);

  Token on_integer(Location, char const*, char const*, int);
};


Token_list lex(Character_stream&);


} // namespace calc


#endif
