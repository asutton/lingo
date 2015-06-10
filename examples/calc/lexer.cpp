
#include "lexer.hpp"

#include "lingo/lexing.hpp"
#include "lingo/symbol.hpp"
#include "lingo/error.hpp"

#include <cassert>
#include <cctype>
#include <iostream>
#include <string>

namespace calc
{

// -------------------------------------------------------------------------- //
//                                Tokens

// The extended token set.
Extended_token_set toks_;


// Initialize the token set used by the language.
void
init_tokens()
{
  install_token_set(toks_);
}


// Returns a string representation of the token name.
char const*
get_token_name(Token_kind k)
{
  switch (k)
  {
  case plus_tok: return "plus_tok";
  case minus_tok: return "minus_tok";
  case star_tok: return "star_tok";
  case slash_tok: return "slash_tok";
  case percent_tok: return "percent_tok";
  default: lingo_unreachable("unknown token kind ({})", k);
  }
}


// Returns the spelling of the given token kind.
char const*
get_token_spelling(Token_kind k)
{
  switch (k)
  {
  case plus_tok: return "+";
  case minus_tok: return "-";
  case star_tok: return "*";
  case slash_tok: return "/";
  case percent_tok: return "%";
  default: lingo_unreachable("unknown token kind ({})", k);
  }
}


// -------------------------------------------------------------------------- //
//                                Lexing

namespace
{

// A helper function to consume whitespace.
inline void
discard_whitespace(Character_stream& cs)
{
  using namespace lingo;
  auto pred = [](Character_stream& s) { return next_element_if(s, is_space); };
  discard_if(cs, pred);
}


// Lexically analyze a single token.
Token
token(Lexer& lex, Character_stream& cs)
{
  // Consume all whitespace before a token.
  discard_whitespace(cs);
  if (cs.eof())
    return {};

  // Match the current token.
  Location loc = cs.location();
  switch (cs.peek()) {
  case '(': return lex.on_lparen(loc, &cs.get());
  case ')': return lex.on_rparen(loc, &cs.get());

  case '+': return lex.on_plus(loc, &cs.get());
  case '-': return lex.on_minus(loc, &cs.get());
  case '*': return lex.on_star(loc, &cs.get());
  case '/': return lex.on_slash(loc, &cs.get());
  case '%': return lex.on_percent(loc, &cs.get());

  default:
    if (is_decimal_digit(cs.peek()))
      return lex_decimal_integer(lex, cs, loc);
    
    // Diagnose the unrecognized character and consume it.
    error(loc, "unrecognized character '{}'", cs.get());
  }
  return {};
}

} // namespace


// Returns the next token in the stream.
Token
Lexer::operator()(Character_stream& cs)
{
  if (Token tok = token(*this, cs)) {
    tokens.push_back(tok);
    return tok;
  }
  return {};
}


Token
Lexer::on_lparen(Location loc, char const* str)
{
  return Token(loc, lparen_tok, str, 1);
}


Token
Lexer::on_rparen(Location loc, char const* str)
{
  return Token(loc, rparen_tok, str, 1);
}


Token
Lexer::on_plus(Location loc, char const* str)
{
  return Token(loc, plus_tok, str, 1);
}


Token
Lexer::on_minus(Location loc, char const* str)
{
  return Token(loc, minus_tok, str, 1);
}


Token
Lexer::on_star(Location loc, char const* str)
{
  return Token(loc, star_tok, str, 1);
}


Token
Lexer::on_slash(Location loc, char const* str)
{
  return Token(loc, slash_tok, str, 1);
}


Token
Lexer::on_percent(Location loc, char const* str)
{
  return Token(loc, percent_tok, str, 1);
}


Token
Lexer::on_decimal_integer(Location loc, char const* first, char const* last)
{
  return Token(loc, decimal_integer_tok, first, last);
}


Token_list 
lex(Character_stream& cs)
{
  Lexer lfn;
  while (!cs.eof()) 
    lfn(cs);
  return lfn.tokens;
}


} // namespace calc
