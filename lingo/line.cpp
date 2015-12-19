// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "config.hpp"

#include "lingo/line.hpp"
#include "lingo/assert.hpp"


namespace lingo
{

// Returns the line containing the character offset.
Line const&
Line_map::line(int n) const
{
  lingo_assert(!empty());
  auto iter = --upper_bound(n);
  return iter->second;
}


// Returns the locus of the given the character offset.
Locus
Line_map::locus(int n) const
{
  Line const& l = line(n);
  return {l.number(), n - l.offset() + 1};
}

} // namespace lingo
