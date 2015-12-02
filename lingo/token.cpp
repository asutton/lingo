// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "lingo/token.hpp"


namespace lingo
{


std::ostream& 
operator<<(std::ostream& os, Token tok)
{
  if (tok) {
    os << '<' << tok.kind() << ':' << tok.spelling() << '>';
  } else {
    os << "<invalid>";
  }
  return os;
}


} // namespace lingo
