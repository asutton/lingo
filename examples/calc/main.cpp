
#include <iostream>

#include "lexer.hpp"
#include "parser.hpp"
#include "ast.hpp"

#include "lingo/error.hpp"
#include "lingo/memory.hpp"

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
  init_diagnostics();
  init_tokens();

  std::string line;
  while (prompt(line)) {
    if (line.empty())
      continue;

    Character_stream cs = line;
    Token_list toks = lex(cs);
    if (error_count()) {
      reset_diagnostics();
      continue;
    }

    // for (Token const& tok : toks)
    //   debug(tok);
    
    Token_stream ts = toks;
    Expr* expr = parse(ts);
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

  gc().collect();
}
