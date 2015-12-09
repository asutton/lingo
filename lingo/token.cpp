// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "lingo/token.hpp"


namespace lingo
{


// TODO: Add debugging support.
std::ostream& 
operator<<(std::ostream& os, Token tok)
{
  if (tok) {
    os << tok.spelling();
    // os << '<' << tok.kind() << ':' << tok.spelling() << '>';
  } else {
    os << "<invalid>";
  }
  return os;
}


} // namespace lingo
