// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#ifndef LINGO_TOKEN_HPP
#define LINGO_TOKEN_HPP

#include "lingo/location.hpp"
#include "lingo/buffer.hpp"
#include "lingo/error.hpp"
#include "lingo/symbol.hpp"
#include "lingo/string.hpp"
#include "lingo/integer.hpp"
#include "lingo/print.hpp"
#include "lingo/debug.hpp"

#include <cstdint>
#include <vector>
#include <tuple>


namespace lingo
{

struct Symbol;



// -------------------------------------------------------------------------- //
//                              Management

void install_token(int, char const*, char const*);

char const* get_token_name(int);
char const* get_token_spelling(int);


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
// Note that -1 is reserved as a special token kind, indicating an
// error.
class Token
{
public:
  // Construct an error token.
  Token()
    : loc_(), sym_(nullptr)
  { }

  Token(Location, char const*, int);
  Token(Location, char const*, char const*);
  Token(Location, int, char const*, int);
  Token(Location, int, char const*, char const*);
  Token(Location, Symbol&);

  explicit operator bool() const { return sym_ && kind() != unknown_tok; }

  // Observers
  char const* token_name() const { return get_token_name(kind()); }
  
  // Source location and span.
  Location  location() const { return loc_; }
  Span      span() const;

  int kind() const { return sym_->kind; }
  
  // Symbol/text representation
  Symbol const& symbol() const { return *sym_; }
  String const* str() const    { return &sym_->str; }

private:
  Location   loc_;
  Symbol*    sym_;
};


// Returns the span of the token.
inline Span
Token::span() const
{
  Location start = location();
  Location end(start.offset() + str()->size());
  return {start, end};
}


// A list of tokens.
using Token_list = std::vector<Token>;


// -------------------------------------------------------------------------- //
//                              Printing

void print(Printer&, Token const&);
void debug(Printer&, Token const&);

std::ostream& operator<<(std::ostream&, Token const&);



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

  // Construct a token stream over a non-empty range of
  // token pointers. Behavior is undefind if f == l.
  Token_stream(Token const* f, Token const* l)
    : first_(f), last_(l)
  { 
    lingo_assert(first_ != last_);
  }

  Token_stream(Token_list const& toks)
    : Token_stream(toks.data(), toks.data() + toks.size())
  { }

  // Stream control
  bool eof() const { return first_ == last_; }
  Token const& peek() const;
  Token const& peek(int) const;
  Token const& get();
  Token const& last() { return *(last_ - 1); }
  Token const& last() const { return *(last_ - 1); }

  // Iterators
  Token const* begin()       { return first_; }
  Token const* begin() const { return first_; }
  Token const* end()       { return last_; }
  Token const* end() const { return last_; }

  // Returns the source location of the the current token.
  Location location() const { return eof() ? Location{} : peek().location(); }

  // Returns the last source location for a token in the buffer.
  Location last_location() const { return last().span().end(); }

  Token const* first_; // Current character pointer
  Token const* last_;  // Past the end of the character buffer
};


// Debug print a token string.
inline void 
debug(Printer& p, Token_stream const& toks)
{
  for (const Token& tok : toks) {
    debug(p, tok);
    print(p, " ");
  }
}


} // namespace lingo


#endif
