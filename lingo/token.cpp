// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "lingo/token.hpp"
#include "lingo/format.hpp"
#include "lingo/symbol.hpp"
#include "lingo/error.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace lingo
{

// -------------------------------------------------------------------------- //
//                              Token name

namespace
{

std::unordered_map<int, char const*> name_;
std::unordered_map<int, Symbol*> spelling_;

} // namespace


// Install the name of a grammar, mapping `rule` to `name`. Note
// that `name` must be a C-string literal.
//
// TODO: Should we strdup `name`?
void 
install_token(int kind, char const* name, char const* spelling)
{
  lingo_alert(name_.count(kind) == 0, "existing token kind '{}'", name);

  // Save the token name.
  name_.insert({kind, name});
  
  // Save the symbol for the spelling of the token.
  spelling_.insert({kind, &get_symbol(spelling, kind)});
}


// Return a name associated with the rule. If the rule does not
// have an associated name, return a placeholder string.
char const* 
get_token_name(int kind)
{
  auto iter = name_.find(kind);
  if (iter == name_.end())
    return "<unspecified-token>";
  else
    return iter->second;
}


// Return a name associated with the rule. If the rule does not
// have an associated name, return a placeholder string.
char const* 
get_token_spelling(int kind)
{
  auto iter = spelling_.find(kind);
  if (iter == spelling_.end())
    return "<unspecified-token>";
  else
    return iter->second->str.c_str();
}



// Initialize a token whose symbol is the string [str, str + len).
// The token kind is taken from the symbol table.
Token::Token(Location loc, char const* str, int len)
  : Token(loc, str, str + len)
{ }


// Initialize a token whose symbol is the string [str, str + len).
// The token kind is taken from the symbol table. If a symbol
// corresponding to the spelling of this token has not been installed,
// behavior is undefined.
Token::Token(Location loc, char const* first, char const* last)
  : loc_(loc), sym_(lookup_symbol(first, last))
{
  lingo_alert(kind() != unknown_tok, "unknown token '{}'", sym_->str);
}


// Initialize a token with the given kind and having the text
// given by the characters in [str, str + len). this will
// add a symbol to the symbol table.
Token::Token(Location loc, int k, char const* str, int len)
  : Token(loc, k, str, str + len)
{ }


// Initialize a token with the given kind and having the
// text given by the characters in [first, last). This will
// add the symbol to the symbol table.
//
// FIXME: This potentially allows an existing symbol with
// an unknown binding to 
Token::Token(Location loc, int k, char const* first, char const* last)
  : loc_(loc), sym_(&get_symbol(first, last, k))
{ }


// Initialize a token with the properties of the given symbol.
Token::Token(Location loc, Symbol& sym)
  : loc_(loc), sym_(&sym)
{ }


// -------------------------------------------------------------------------- //
//                              Token stream

Token const& 
Token_stream::peek() const
{
  assert(!eof());
  return *first_;
}


Token const&
Token_stream::peek(int n) const
{
  assert(n <= (last_ - first_));
  return *(first_ + n);
}


Token const& 
Token_stream::get()
{
  assert(!eof());

  // Save the token's location as the input location.
  set_input_location(location());

  // Advance the token, return the previous value.
  Token const& tok = *first_;  
  ++first_;
  return tok;
}


// -------------------------------------------------------------------------- //
//                           Pretty printing


// Pretty print the given token.
void 
print(Printer& p, Token const& tok)
{
  print(p, tok.str());
}


// Print a debug representation of the token.
void
debug(Printer& p, Token const& tok)
{
  print(p, '<');
  print(p, tok.str());
  print(p, '>');
}


// Streaming
std::ostream&
operator<<(std::ostream& os, Token tok)
{
  return os << *tok.str();
}


} // namespace lingo
