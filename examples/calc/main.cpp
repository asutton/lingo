
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

    // Construct a buffer for the line.
    Buffer buf(line);
    Input_guard guard(buf);

    Character_stream cs(buf);
    Token_list toks = lex(cs);
    if (error_count()) {
      reset_diagnostics();
      continue;
    }

    // debug(toks);

    Token_stream ts(toks);
    Expr* expr = parse(buf, ts);
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
