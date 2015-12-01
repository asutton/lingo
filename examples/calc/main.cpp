// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "lexer.hpp"
#include "parser.hpp"
#include "ast.hpp"
#include "directive.hpp"
#include "step.hpp"

#include "lingo/error.hpp"
#include "lingo/memory.hpp"

#include <iostream>



namespace calc
{

Symbol_table syms;

} 

using namespace lingo;
using namespace calc;


// Initialize the token set used by the language.
void
init_tokens(Symbol_table& syms)
{
  syms.put_symbol(lparen_tok, "(");
  syms.put_symbol(rparen_tok, ")");
  syms.put_symbol(plus_tok, "+");
  syms.put_symbol(minus_tok, "-");
  syms.put_symbol(star_tok, "*");
  syms.put_symbol(slash_tok, "/");
}


std::istream&
prompt(std::string& line)
{
  std::cout << "> ";
  return getline(std::cin, line);
}


int 
main()
{
  init_tokens(syms);
  evaluation_mode(eval_mode);


  std::string line;
  while (prompt(line)) {
    if (line.empty())
      continue;

    // Construct a buffer for the line.
    Buffer buf(line);

    // If the input contains a directive, then process
    // that and continue.
    if (contains_directive(buf)) {
      process_directive(buf);
      continue;
    }

    Token_stream ts;
    Character_stream cs(buf);
    Lexer lex(syms, cs, ts);
    Parser parse(ts);
    
    // Transform characters into tokens.
    lex();
    if (error_count()) {
      reset_diagnostics();
      continue;
    }

    // Transform tokens into abstract syntax.
    Expr const* expr = parse();
    if (error_count()) {
      reset_diagnostics();
      continue;
    }

    // FIXME: This is an error. Why didn't we fail before.
    if (!expr) {
      std::cout << "internal error: parsing failed\n";
      continue;
    }

    if (is_step_mode())
      step_eval(expr);
    else      
      std::cout << expr << " == " << evaluate(expr) << '\n';
  }
}
