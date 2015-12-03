// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "lingo/symbol.hpp"

#include <iostream>


namespace lingo
{


std::ostream&
operator<<(std::ostream& os, Symbol const& sym)
{
  return os << sym.spelling();
}


} // namespace lingo
