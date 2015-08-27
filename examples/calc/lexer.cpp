
#include "lexer.hpp"

#include "lingo/lexing.hpp"
#include "lingo/symbol.hpp"
#include "lingo/character.hpp"
#include "lingo/error.hpp"

#include <cassert>
#include <cctype>
#include <string>
#include <iostream>

namespace calc
{

// -------------------------------------------------------------------------- //
//                                Tokens


// Initialize the token set used by the language.
void
init_tokens()
{
  install_token(lparen_tok,  "lparen_tok", "(");
  install_token(rparen_tok,  "rparen_tok", ")");
  install_token(plus_tok,    "plus_tok",   "+");
  install_token(minus_tok,   "minus_tok",  "-");
  install_token(star_tok,    "star_tok",   "*");
  install_token(slash_tok,   "slash_tok",   "/");
  install_token(percent_tok, "percent_tok", "%");
}


// -------------------------------------------------------------------------- //
//                                Lexing

namespace
{

// A helper function to consume whitespace.
inline void
whitespace(Character_stream& cs)
{
  using namespace lingo;
  auto pred = [](Character_stream& s) { return next_element_if(s, is_space); };
  discard_if(cs, pred);
}


// Lexically analyze a single token.
Token
token(Lexer& lex, Character_stream& cs)
{
  whitespace(cs);

  while (!cs.eof()) {
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
      // TODO: Unfold into cases.
      if (is_decimal_digit(cs.peek()))
        return lex_decimal_integer(lex, cs, loc);

      // Diagnose the unrecognized character and consume it.
      error("unrecognized character '{}'", cs.get());
    }
  }
  return {};
}


} // namespace


Token
Lexer::on_lparen(Location loc, char const* str)
{
  return Token(loc, str, 1);
}


Token
Lexer::on_rparen(Location loc, char const* str)
{
  return Token(loc, str, 1);
}


Token
Lexer::on_plus(Location loc, char const* str)
{
  Token tok = Token(loc, str, 1);
  return tok;
}


Token
Lexer::on_minus(Location loc, char const* str)
{
  return Token(loc, str, 1);
}


Token
Lexer::on_star(Location loc, char const* str)
{
  return Token(loc, str, 1);
}


Token
Lexer::on_slash(Location loc, char const* str)
{
  return Token(loc, str, 1);
}


Token
Lexer::on_percent(Location loc, char const* str)
{
  return Token(loc, str, 1);
}


Token
Lexer::on_decimal_integer(Location loc, char const* first, char const* last)
{
  return Token(loc, integer_tok, first, last);
}


// Lex all tokens in the character stream.
Token_list 
lex(Character_stream& cs)
{
  Lexer lexer;
  Token_list toks;
  while (Token tok = token(lexer, cs))
    toks.push_back(tok);
  return toks;
}



// -------------------------------------------------------------------------- //
//                           Elaboration


// Return the integer value of the token. 
Integer
as_integer(Token const& tok)
{
  return Integer(*tok.str(), 10);
}


} // namespace calc
