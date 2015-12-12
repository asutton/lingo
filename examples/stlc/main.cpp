// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "lexer.hpp"
#include "parser.hpp"
#include "evaluator.hpp"

#include <lingo/file.hpp>
#include <lingo/io.hpp>
#include <lingo/error.hpp>

#include <iostream>


using namespace lingo;
using namespace calc;


// Initialize the token set used by the language.
void
init_tokens()
{
  symbols.put_symbol(lparen_tok, "(");
  symbols.put_symbol(rparen_tok, ")");
  symbols.put_symbol(backslash_tok, "\\");
  symbols.put_symbol(dot_tok, ".");
  symbols.put_symbol(equal_tok, "=");
  symbols.put_symbol(colon_tok, ":");
  symbols.put_symbol(semicolon_tok, ";");
  symbols.put_symbol(arrow_tok, "->");
}


int
main(int argc, char* argv[])
{
  init_colors();
  init_tokens();

  if (argc != 2) {
    std::cerr << "usage: lambda <input-file>\n";
    return -1;
  }

  File input(argv[1]);
  Character_stream cs(input);
  Token_stream ts(input);
  Lexer lex(cs, ts);
  Parser parse(ts);

  // Transform characters into tokens.
  lex();
  if (error_count())
    return -1;

  try {
    // Transform tokens into abstract syntax.
    Expr const* expr = parse();
    if (error_count())
      return 1;
    // std::cout << "Parsed:\n" << *expr << '\n';

    Evaluator eval;
    Expr const* result = eval(expr);
    if (result)
      std::cout << *result << '\n';
  } catch (Translation_error&) {
    return 1;
  }
  return 0;
}
