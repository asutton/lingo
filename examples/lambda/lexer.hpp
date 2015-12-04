// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#ifndef CALC_LEXER_HPP
#define CALC_LEXER_HPP

// The calc lexer defines the additional tokens used by the 
// language and the machine used to recognize those tokens 
// in input source.

#include <lingo/symbol.hpp>
#include <lingo/token.hpp>
#include <lingo/character.hpp>


namespace calc
{

using namespace lingo;


// -------------------------------------------------------------------------- //
// Symbols


// The symbol table is a global resource.
//
// TODO: Consider protecting this with a
// a set of accessor functions.
extern Symbol_table symbols;


// -------------------------------------------------------------------------- //
// Tokens

enum Token_kind
{
  error_tok = -1,
  lparen_tok,
  rparen_tok,
  backslash_tok,
  dot_tok,
  equal_tok,
  semicolon_tok,
  identifier_tok,
};


char const* get_spelling(Token_kind);


// -------------------------------------------------------------------------- //
// Lexing


// The Lexer is a facility that translates sequences of
// characters into tokens. This is primarily a callback
// interface for the lexing function for the language.
struct Lexer
{
  using argument_type = char;
  using result_type = Token;

  Lexer(Character_stream& cs, Token_stream& ts)
    : cs_(cs), ts_(ts)
  { }

  void operator()();

  // Scanners
  Token scan();
  Token eof();
  Token symbol1();
  Token identifier();

  // Consumers
  void error();
  void space();
  void letter();

  // Semantic actions.
  Token on_symbol();
  Token on_identifier();

  void save();

  Character_stream& cs_;
  Token_stream&     ts_;
  String_builder    str_;
  Location          loc_;
};


} // namespace calc


#endif
