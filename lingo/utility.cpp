// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "lingo/utility.hpp"

#include <cstdlib>
#include <cxxabi.h>
#include <iostream>

namespace lingo
{

String
type_str(std::type_info const& t)
{
  std::size_t n = 0;
  char* buf = abi::__cxa_demangle(t.name(), nullptr, &n, 0);
  String result(buf);
  std::free(buf);
  return result;
}


} // namespace lingo
