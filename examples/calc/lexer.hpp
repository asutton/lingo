// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#ifndef CALC_LEXER_HPP
#define CALC_LEXER_HPP

// The calc lexer defines the additional tokens used by the 
// language and the machine used to recognize those tokens 
// in input source.

#include <lingo/character.hpp>
#include <lingo/token.hpp>


namespace calc
{

using namespace lingo;

// -------------------------------------------------------------------------- //
//                              Tokens
enum Token_kind
{
  error_tok = -1,
  lparen_tok,
  rparen_tok,
  plus_tok,
  minus_tok,
  star_tok,
  slash_tok,
  percent_tok,
  integer_tok,
};


char const* get_spelling(Token_kind);


// -------------------------------------------------------------------------- //
//                              Lexing


// The Lexer is a facility that translates sequences of
// characters into tokens. This is primarily a callback
// interface for the lexing function for the language.
struct Lexer
{
  using argument_type = char;
  using result_type = Token;

  Lexer(Symbol_table& s, Character_stream& cs, Token_stream& ts)
    : syms_(s), cs_(cs), ts_(ts)
  { }

  void operator()();

  // Scanners
  Token scan();
  Token eof();
  Token symbol1();
  Token integer();

  // Consumers
  void error();
  void space();
  void digit();

  // Semantic actions.
  Token on_symbol();
  Token on_integer();

  void save();

  Symbol_table&     syms_;
  Character_stream& cs_;
  Token_stream&     ts_;
  String_builder    str_;
  Location          loc_;
};


} // namespace calc


#endif
