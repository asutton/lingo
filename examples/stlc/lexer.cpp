// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "lexer.hpp"

#include <lingo/error.hpp>

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
    case backslash_tok: return "\\";
    case dot_tok: return ".";
    case equal_tok: return "=";
    case colon_tok: return ":";
    case semicolon_tok: return ";";
    case arrow_tok: return "->";
    case identifier_tok: return "<identifier>";
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
    case '(': return symbol();
    case ')': return symbol();
    case '\\': return symbol();
    case '.': return symbol();
    case '=': return symbol();
    case ':': return symbol();
    case ';': return symbol();

    case '-':
      save();
      if (cs_.peek() == '>')
        return symbol();
      else
        error();

    default:
      if (is_alpha(cs_.peek()))
        return identifier();
      if (is_decimal_digit(cs_.peek()))
        return integer();
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


// Save the last character of a symbol and process it.
Token
Lexer::symbol()
{
  save();
  return on_symbol();
}


// letter ::= [a-z][A-Z]
void
Lexer::letter()
{
  save();
}


// digit ::= [0-9]
void
Lexer::digit()
{
  save();
}


Token
Lexer::identifier()
{
  letter();
  while (is_alpha(cs_.peek()))
    letter();
  return on_identifier();
}


Token
Lexer::integer()
{
  digit();
  while (is_decimal_digit(cs_.peek()))
    digit();
  return on_identifier();
}


Token
Lexer::on_symbol()
{
  Symbol const* sym = symbols.get(str_.take());
  return Token(loc_, sym);
}


Token
Lexer::on_identifier()
{
  String str = str_.take();
  Symbol* sym = symbols.put_identifier(identifier_tok, str);
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
