// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "lexer.hpp"

#include "lingo/error.hpp"

#include <cassert>
#include <cctype>
#include <string>
#include <iostream>

namespace calc
{

// -------------------------------------------------------------------------- //
// Symbols

Symbol_table symbols;


// -------------------------------------------------------------------------- //
// Tokens

char const*
get_spelling(Token_kind k)
{
  switch (k) {
    case error_tok: return "<error>";
    case lparen_tok: return "(";
    case rparen_tok: return ")";
    case plus_tok: return "+";
    case minus_tok: return "-";
    case star_tok: return "*";
    case slash_tok: return "/";
    case percent_tok: return "%";
    case integer_tok: return "<integer>";
  }
  lingo_unreachable();
}


// -------------------------------------------------------------------------- //
// Lexing


void
Lexer::save()
{
  str_.put(cs_.get());
}


// Lexically analyze a single token.
Token
Lexer::scan()
{
  while (!cs_.eof()) {
    space();

    loc_ = cs_.location();
    switch (cs_.peek()) {
    case '\0': return eof();
    case '(': return symbol1();
    case ')': return symbol1();
    case '+': return symbol1();
    case '-': return symbol1();
    case '*': return symbol1();
    case '/': return symbol1();
    case '%': return symbol1();

    default:
      // TODO: Unfold into cases.
      if (is_decimal_digit(cs_.peek()))
        return integer();
      else
        error();
    }
  }
  return {};
}


void
Lexer::error()
{
  lingo::error(loc_, "unrecognized character '{}'", cs_.get());
}


void
Lexer::space()
{
  while (is_space(cs_.peek()))
    cs_.ignore();
}


Token
Lexer::eof()
{
  return Token{};
}


Token
Lexer::symbol1()
{
  save();
  return on_symbol();
}


// digit ::= [0-9]
void
Lexer::digit()
{
  save();
}


Token
Lexer::integer()
{
  assert(is_decimal_digit(cs_.peek()));
  digit();
  while (is_decimal_digit(cs_.peek()))
    digit();
  return on_integer();
}


Token
Lexer::on_symbol()
{
  Symbol const* sym = symbols.get(str_.take());
  return Token(loc_, sym);
}


Token
Lexer::on_integer()
{
  String str = str_.take();
  int n = string_to_int<int>(str, 10);
  Symbol* sym = symbols.put_integer(integer_tok, str, n);
  return Token(loc_, sym);
}


void
Lexer::operator()()
{
  while (Token tok = scan())
    ts_.put(tok);
}


} // namespace calc
