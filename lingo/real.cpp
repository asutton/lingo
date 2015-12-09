// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "lingo/real.hpp"
#include "lingo/debug.hpp"

#include <memory>

namespace lingo
{


// Streaming
std::ostream&
operator<<(std::ostream& os, Real const& n)
{
  return os;
}


} // namespace lingo
