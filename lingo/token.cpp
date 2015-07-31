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



namespace
{


// Returns the symbol kind associated with the token kind.
Symbol_kind
get_symbol_kind(Token_kind k)
{
  switch (k) {
  case identifier_tok: 
    return identifier_sym;

  case binary_integer_tok:
  case octal_integer_tok:
  case decimal_integer_tok:
  case hexadecimal_integer_tok:
    return integer_sym;

  default: 
    return language_sym;
  }
}


// A helper function for creating symbols.
Symbol*
make_symbol(char const* str, Token_kind k)
{
  Symbol& sym = get_symbol(str);
  sym.desc = {get_symbol_kind(k), k};
  return &sym;
}


// A helper function for creating symbols.
Symbol*
make_symbol(char const* first, char const* last, Token_kind k)
{
  Symbol& sym = get_symbol(first, last);
  sym.desc = {get_symbol_kind(k), k};
  return &sym;
}


} // namespace


// Install a token. This associates the given spelling with its
// name in the symbol table. Every token with a fixed set of values
// must be installed prior to lookup.
void
install_token(char const* str, Token_kind tk)
{
  make_symbol(str, tk);
}


// Install a sequence of tokens. For example:
//
//    install_tokens({
//      { "(", lparen_tok},
//      { "}", rparen_tok}
//    });
void
install_tokens(std::initializer_list<std::pair<char const*, Token_kind>> list)
{
  for (auto p : list)
    install_token(p.first, p.second);
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
  
  case dot_tok: return "dot_tok";
  case comma_tok: return "comma_tok";
  case semicolon_tok: return "semicolon_tok";
  case colon_tok: return "colon_tok";
  case equal_tok: return "equal_tok";
  case plus_tok: return "plus_tok";
  case minus_tok: return "minus_tok";
  case star_tok: return "star_tok";
  case slash_tok: return "slash_tok";
  case percent_tok: return "percent_tok";
  case amp_tok: return "amp_tok";
  case bar_tok: return "bar_tok";
  case caret_tok: return "caret_tok";
  case tilde_tok: return "tilde_tok";
  case bang_tok: return "bang_tok";
  case lt_tok: return "lt_tok";
  case gt_tok: return "gt_tok";

  case minus_gt_tok: return "minus_gt_tok";
  case eq_gt_tok: return "eq_gt_tok";
  case lt_lt_tok: return "lt_lt_tok";
  case gt_gt_tok: return "gt_gt_tok";
  case eq_eq_tok: return "eq_eq_tok";
  case bang_eq_tok: return "bang_eq_tok";
  case lt_eq_tok: return "lt_eq_tok";
  case gt_eq_tok: return "gt_eq_tok";
  case amp_amp_tok: return "amp_amp_tok";
  case bar_bar_tok: return "bar_bar_tok";
  case dot_dot_tok: return "dot_dot_tok";

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
  case error_tok: return "<error>";

  case lparen_tok: return "(";
  case rparen_tok: return ")";
  case lbrace_tok: return "{";
  case rbrace_tok: return "}";
  case rbrack_tok: return "[";
  case lbrack_tok: return "]";
  
  case dot_tok: return ".";
  case comma_tok: return ",";
  case semicolon_tok: return ";";
  case colon_tok: return ":";
  case equal_tok: return "=";
  case plus_tok: return "+";
  case minus_tok: return "-";
  case star_tok: return "*";
  case slash_tok: return "/";
  case percent_tok: return "%";
  case amp_tok: return "&";
  case bar_tok: return "|";
  case caret_tok: return "^";
  case tilde_tok: return "~";
  case bang_tok: return "!";
  case lt_tok: return "<";
  case gt_tok: return ">";

  case minus_gt_tok: return "->";
  case eq_gt_tok: return "=>";
  case lt_lt_tok: return "<<";
  case gt_gt_tok: return ">>";
  case eq_eq_tok: return "==";
  case bang_eq_tok: return "!=";
  case lt_eq_tok: return "<=";
  case gt_eq_tok: return ">=";
  case amp_amp_tok: return "&&";
  case bar_bar_tok: return "||";
  case dot_dot_tok: return "..";

  case identifier_tok: return "identifier";
  case boolean_tok: return "boolean-value";
  case binary_integer_tok: return "binary-integer";
  case decimal_integer_tok: return "decimal-integer";
  case octal_integer_tok: return "octal-integer";
  case hexadecimal_integer_tok: return "hexadecimal-integer";

  default:
    if (toks_)
      return toks_->token_spelling(k);
    else
      lingo_unreachable("unknown token kind ({})", k);
  }
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
  : loc_(loc)
{
  lingo_alert(lookup_symbol(first, last), 
              "uninstalled token '{}'", String_view(first, last));
  sym_ = lookup_symbol(first, last);
  kind_ = reinterpret_cast<Token_kind>(sym_->desc.token);
}


// Initialize a token with the given kind and having the text
// given by the characters in [str, str + len).
Token::Token(Location loc, Token_kind k, char const* str, int len)
  : Token(loc, k, str, str + len)
{ }


// Initialize a token with the given kind and having the
// text given by the characters in [first, last).
Token::Token(Location loc, Token_kind k, char const* first, char const* last)
  : loc_(loc), kind_(k), sym_(make_symbol(first, last, k))
{ }


// Initialize a token with the properties of the given symbol.
Token::Token(Location loc, Symbol& sym)
  : loc_(loc), kind_(sym.desc.token), sym_(&sym)
{
}


Token
Token::make_identifier(Location loc, Symbol& sym)
{
  sym.desc.kind = identifier_sym;
  sym.desc.token = identifier_tok;
  return Token(loc, sym);
}


// TODO: Assign the integer interpretation of the given token.
Token
Token::make_integer(Location loc, Symbol& sym, Token_kind k)
{
  lingo_assert(is_integer(k));
  sym.desc.kind = integer_sym;
  sym.desc.token = k;
  return Token(loc, sym);
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
  return Integer(*tok.str(), get_integer_base(tok));
}


// Return the string representation of the token.
String const&
as_string(Token tok)
{
  return *tok.str();
}


// -------------------------------------------------------------------------- //
//                           Pretty printing


// Pretty print the given token.
void 
print(Printer& p, Token tok)
{
  print(p, tok.str());
}


// Print a debug representation of the token.
void
debug(Printer& p, Token tok)
{
  print(p, '<');
  print(p, tok.str());
  print(p, '>');
}


// Streaming
std::ostream&
operator<<(std::ostream& os, Token tok)
{
  return os << tok.str();
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

  // Save the token's location as the input location.
  set_input_location(location());

  // Advance the token, return the previous value.
  Token const& tok = *first_;  
  ++first_;
  return tok;
}


void
debug(Printer& p, Token_stream const& toks)
{
  for (auto iter = toks.begin(); iter != toks.end(); ++iter) {
    debug(p, *iter);
    print(p, " ");
  }

}


} // namespace lingo
