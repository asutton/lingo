// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#ifndef LINGO_TOKEN_HPP
#define LINGO_TOKEN_HPP

#include "lingo/location.hpp"
#include "lingo/string.hpp"
#include "lingo/integer.hpp"
#include "lingo/print.hpp"
#include "lingo/debug.hpp"

#include <cassert>
#include <cstdint>
#include <vector>

namespace lingo
{

struct Symbol;

// -------------------------------------------------------------------------- //
//                              Token kinds
//
// Note that the token spellings below are suggestions. The 
// actual spelling of tokens is determined by the lexer.

// The Token_kind enumerates the different kinds of lexemes 
// that can appear in the source language. 
using Token_kind = int;

constexpr Token_kind error_tok      = 0; // not a valid token

// Common punctuation tokens
constexpr Token_kind lparen_tok     = 1; // (
constexpr Token_kind rparen_tok     = 2; // )
constexpr Token_kind lbrace_tok     = 3; // {
constexpr Token_kind rbrace_tok     = 4; // }
constexpr Token_kind lbrack_tok     = 5; // ]
constexpr Token_kind rbrack_tok     = 6; // [
constexpr Token_kind comma_tok      = 7; // ,
constexpr Token_kind semicolon_tok  = 8; // ;
constexpr Token_kind colon_tok      = 9; // :

// Common value classes
constexpr Token_kind identifier_tok          = 10; // identifiers (like C)
constexpr Token_kind boolean_tok             = 11; // true | false
constexpr Token_kind binary_integer_tok      = 12; // 0b[:binary-digit:]*
constexpr Token_kind decimal_integer_tok     = 13; // [:decimal-digit:]*
constexpr Token_kind octal_integer_tok       = 14; // 0o[:octal-digit:]*
constexpr Token_kind hexadecimal_integer_tok = 15; // 0x[:hexadecimal-digit]*

// Last token value. All application specific tokens
// must start with this value.
constexpr Token_kind max_tok = 15;


char const* get_token_name(Token_kind);
char const* get_token_spelling(Token_kind);


std::ostream& operator<<(std::ostream&, Token_kind);


// -------------------------------------------------------------------------- //
//                            Token class

// The Token class represents the occurrence of a lexeme  within a 
// source file. It associates the class of the the lexeme with its 
// associated value (if any) and its location in the source file.
//
// Each token indexes an entry in the symbol table, which stores 
// additional attributes associated with the token  (e.g. scope 
// bindings, numeric interpretation of values, etc.).
//
// A token whose kind is error_tok is not a valid token. This can be 
// used as a method for determining when a matching  algorithm fails 
// to match a sequence of characters.
class Token
{
public:
  // Construct an error token.
  Token()
    : loc_(), kind_(error_tok)
  { }

  Token(Location, Token_kind, char const*, int);
  Token(Location, Token_kind, char const*, char const*);

  // Observers
  char const*   token_name() const     { return get_token_name(kind_); }
  char const*   token_spelling() const { return get_token_spelling(kind_); }
  Location      location() const       { return loc_; }
  Token_kind    kind() const           { return kind_; }
  Symbol const& symbol() const         { return *sym_; }
  String_view   rep() const;
  std::string   str() const;

  // Contextually convertible to bool.
  // True when this is not an error token.
  explicit operator bool() const { return kind_ != error_tok; }

private:
  Location   loc_;
  Token_kind kind_;
  Symbol*    sym_;
};


// A sequence of tokens.
using Token_list = std::vector<Token>;


// -------------------------------------------------------------------------- //
//                              Elaboration

bool    as_boolean(Token);
Integer as_integer(Token);

// -------------------------------------------------------------------------- //
//                              Printing

void print(Printer&, Token);
void debug(Printer&, Token);

std::ostream& operator<<(std::ostream&, Token);


// -------------------------------------------------------------------------- //
//                              Token stream


// A token stream provides a sequence of tokens and has a very 
// simple streaming interface consisting of only 5 functions:
// peek(), get(), and eof(), begin(), and end(). Character streams
// are the input to lexical analyzers.
class Token_stream
{
public:
  using value_type = Token;

  Token_stream(Token const* f, Token const* l)
    : first_(f), last_(l)
  { }

  Token_stream(Token_list const& toks)
    : Token_stream(toks.data(), toks.data() + toks.size())
  { }

  // Stream control
  bool eof() const { return first_ == last_; }
  Token const& peek() const;
  Token        peek(int) const;
  Token const& get();

  // Iterators
  Token const* begin()       { return first_; }
  Token const* begin() const { return first_; }
  Token const* end()       { return last_; }
  Token const* end() const { return last_; }

  // Returns the source location of the the current token.
  Location location() { return eof() ? Location{} : peek().location(); }

  Token const* first_; // Current character pointer
  Token const* last_;  // Past the end of the character buffer
};


// -------------------------------------------------------------------------- //
//                              Token sets

// A token set defines hooks between the general token
// processing faclities in lingo and those in client
// languages.
//
// TODO: Should we allow multiple token sets? That would
// effectively imply that we partition token values values
// among applications and that there could be NO overlapping
// values.
struct Token_set
{
  virtual ~Token_set() { }

  virtual char const* token_name(Token_kind) const = 0;
  virtual char const* token_spelling(Token_kind) const = 0;
};


void install_token_set(Token_set&);
void uninstall_token_set(Token_set&);


} // namespace lingo


#endif
