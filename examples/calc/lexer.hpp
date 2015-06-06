
#ifndef CALC_LEXER_HPP
#define CALC_LEXER_HPP

// The calc lexer defines the additional tokens used by the 
// language and the machine used to recognize those tokens 
// in input source.

#include "lingo/lexing.hpp"

#include <string>

namespace calc
{

using namespace lingo;

// -------------------------------------------------------------------------- //
//                              Tokens

constexpr Token_kind plus_tok    = 100;
constexpr Token_kind minus_tok   = 101;
constexpr Token_kind star_tok    = 102;
constexpr Token_kind slash_tok   = 103;
constexpr Token_kind percent_tok = 104;

constexpr Token_kind minus_minus_tok = 200;

char const* get_token_name(Token_kind);
char const* get_token_spelling(Token_kind);


// The extended token set for the calc language.
struct Extended_token_set : Token_set
{
  char const* token_name(Token_kind k) const override
  { 
    return get_token_name(k); 
  }

  char const* token_spelling(Token_kind k) const override
  { 
    return get_token_spelling(k); 
  }
};


void init_tokens();


// -------------------------------------------------------------------------- //
//                              Lexing


// The Lexer is a function that maps a character stream into
// tokens. Each call to an object of this type returns the
// next token in the stream in the underlying character stream.
//
// Each successive token is cached by the lexer.
struct Lexer
{
  using argument_type = char;
  using result_type = Token;

  // Lexical analysis.
  Token operator()(Character_stream&);

  // Semantic actions.
  Token on_lparen(Location, char const*);
  Token on_rparen(Location, char const*);

  Token on_plus(Location, char const*);
  Token on_minus(Location, char const*);
  Token on_star(Location, char const*);
  Token on_slash(Location, char const*);
  Token on_percent(Location, char const*);

  Token on_decimal_integer(Location, char const*, char const*);

  Token_list tokens;
};


Token_list lex(Character_stream&);

} // namespace calc

#endif
