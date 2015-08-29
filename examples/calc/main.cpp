// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "lexer.hpp"
#include "parser.hpp"
#include "ast.hpp"

#include "lingo/error.hpp"
#include "lingo/memory.hpp"

#include <iostream>

using namespace lingo;
using namespace calc;


std::istream&
prompt(std::string& line)
{
  std::cout << "> ";
  return getline(std::cin, line);
}


int main()
{
  init_tokens();
  init_grammar();

  std::string line;
  while (prompt(line)) {
    if (line.empty())
      continue;

    // Construct a buffer for the line.
    Buffer buf(line);
    
    // Establish the input context.
    Use_buffer cxt(buf);

    // Transform character input into tokens.
    Character_stream cs(buf);
    Token_list toks = lex(cs);
    if (error_count()) {
      reset_diagnostics();
      continue;
    }

    // debug(toks);

    // Transform tokens into abstract syntax.
    Token_stream ts(toks);
    Expr const* expr = parse(ts);
    if (error_count()) {
      reset_diagnostics();
      continue;
    }

    // FIXME: This is an error. Why didn't we fail before.
    if (!expr) {
      std::cout << "internal error: parsing failed\n";
      continue;
    }

    std::cout << expr << " == " << evaluate(expr) << '\n';
  }
}
