// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "lingo/integer.hpp"
#include "lingo/debug.hpp"

#include <memory>

namespace lingo
{


// Streaming
std::ostream&
operator<<(std::ostream& os, const Integer& n)
{
  return os << n.impl().toString(10, true);
}


} // namespace lingo
