// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "lingo/token.hpp"
#include "lingo/format.hpp"
#include "lingo/symbol.hpp"
#include "lingo/error.hpp"

#include <stdexcept>
#include <iostream>
#include <sstream>
#include <iomanip>

namespace lingo
{

// -------------------------------------------------------------------------- //
//                              Token sets

namespace
{

// The extended token set.
Token_set* toks_;

}

// Install the token set. Currently, only one token set may
// be installed.
void
install_token_set(Token_set& toks)
{
  lingo_assert(!toks_);
  toks_ = &toks;
}


// Uninstall the extended token set.
void
uninstall_token_set(Token_set& toks)
{
  lingo_assert(toks_ == &toks);
  toks_ = nullptr;
}


// -------------------------------------------------------------------------- //
//                              Token kinds

char const*
get_token_name(Token_kind k)
{
  switch (k)
  {
  case error_tok: return "error_tok";
  
  case lparen_tok: return "lparen_tok";
  case rparen_tok: return "rparen_tok";
  case lbrace_tok: return "lbrace_tok";
  case rbrace_tok: return "rbrace_tok";
  case rbrack_tok: return "rbrack_tok";
  case lbrack_tok: return "lbrack_tok";
  case comma_tok: return "comma_tok";
  case semicolon_tok: return "semicolon_tok";
  case colon_tok: return "colon_tok";
  
  case identifier_tok: return "identifier_tok";
  case boolean_tok: return "boolean_tok";
  case binary_integer_tok: return "binary_integer_tok";
  case decimal_integer_tok: return "decimal_integer_tok";
  case octal_integer_tok: return "octal_integer_tok";
  case hexadecimal_integer_tok: return "hexadecimal_integer_tok";

  default:
    if (toks_)
      return toks_->token_name(k);
    else
      lingo_unreachable("unknown token kind ({})", k);
  }
}


// Returns the spelling associated with a token kind. If
// the token defines a class of spellings, then we return
// the token name instead.
char const*
get_token_spelling(Token_kind k)
{
  switch (k)
  {
  // Spellable tokens.
  case error_tok: return "<error>";
  case lparen_tok: return "(";
  case rparen_tok: return ")";
  case lbrace_tok: return "{";
  case rbrace_tok: return "}";
  case rbrack_tok: return "[";
  case lbrack_tok: return "]";
  case comma_tok: return ",";
  case semicolon_tok: return ";";
  case colon_tok: return ":";
  
  // Non-spellable tokens.
  case identifier_tok: return "identifier";
  case binary_integer_tok: return "binary integer";
  case decimal_integer_tok: return "decimal integer";
  case octal_integer_tok: return "octal integer";
  case hexadecimal_integer_tok: return "hexadecimal integer";

  default:
    if (toks_)
      return toks_->token_spelling(k);
    else
      lingo_unreachable("unknown token kind ({})", k);
  }
}


std::ostream& 
operator<<(std::ostream& os, Token_kind k)
{
  return os << get_token_spelling(k);
}


// Initialize a token with the given kind and having the text
// given by the characters in [str, str + len).
Token::Token(Location loc, Token_kind k, char const* str, int len)
  : loc_(loc), kind_(k), sym_(&get_symbol(str, str + len))
{ }


// Initialize a token with the given kind and having the
// text given by the characters in [first, last).
Token::Token(Location loc, Token_kind k, char const* first, char const* last)
  : loc_(loc), kind_(k), sym_(&get_symbol(first, last))
{ 
}


// Returns a string view of the token's original spelling.
String_view
Token::rep() const
{
  return symbol().str;
}


// Return a string representation of the token.
std::string
Token::str() const
{
  return rep().str();
}


// -------------------------------------------------------------------------- //
//                           Elaboration

// Returns the boolean value of the token. By default, we assume
// that truth is spelled `true` and falsity is spelled `false`.
//
// TODO: Allow for alternative spellings of `true` and `false`.
bool
as_boolean(Token tok)
{
  assert(tok.kind() == boolean_tok);
  if (tok.symbol() == get_symbol("true"))
    return true;
  if (tok.symbol() == get_symbol("false"))
    return false;
  lingo_unreachable("unknown spelling of boolean token '{}'", tok);
}


namespace
{

// Returns true if `k` is an intger token kind.
inline bool
is_integer(Token_kind k)
{
  return binary_integer_tok <= k && k <= hexadecimal_integer_tok;
}


// Retursn the base of an integer token.
inline int
get_integer_base(Token tok)
{
  lingo_assert(is_integer(tok.kind()));
  switch (tok.kind()) {
  case binary_integer_tok: return 2;
  case octal_integer_tok: return 8;
  case decimal_integer_tok: return 10;
  case hexadecimal_integer_tok: return 16;

  default:
    lingo_unreachable("unknown integer base for '{}'", tok.token_name());
  }
}

} // namespace


// Return the integer value of the token. 
//
// TODO: Cache the elaborated result in the symbol table?
Integer 
as_integer(Token tok)
{
  return Integer(tok.str(), get_integer_base(tok));
}

// -------------------------------------------------------------------------- //
//                           Pretty printing


// Pretty print the given token.
void 
print(Printer& p, Token tok)
{
  print(p, tok.rep());
}


// Print a debug representation of the token.
void
debug(Printer& p, Token tok)
{
  print(p, '<');
  print(p, tok.rep());
  print(p, '>');
}


// Streaming
std::ostream&
operator<<(std::ostream& os, Token tok)
{
  Printer p(os);
  print(p, tok.rep());
  return os;
}


// -------------------------------------------------------------------------- //
//                              Token stream


Token const& 
Token_stream::peek() const
{
  assert(!eof());
  return *first_;
}


Token 
Token_stream::peek(int n) const
{
  assert(n <= (last_ - first_));
  return *(first_ + n);
}


Token const& 
Token_stream::get()
{
  assert(!eof());
  Token const& tok = *first_;  
  ++first_;
  return tok;
}


} // namespace lingo
