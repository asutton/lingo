// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "lingo/format.hpp"

namespace lingo {

// Returns the integer base of the current stream.
int
stream_base(const std::ios_base& s) 
{
  std::ios_base::fmtflags f = s.flags() & std::ios_base::basefield;
  if (f == std::ios_base::hex)
    return 16;
  else if (f == std::ios_base::oct)
    return 8;
  else
    return 10;
}

} // namespace lingo
