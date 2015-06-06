// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "parsing.hpp"

#include <unordered_map>

namespace lingo
{

Token const& 
Token_stream::peek() const
{
  assert(!eof());
  return *first;
}


Token 
Token_stream::peek(int n) const
{
  assert(n <= (last - first));
  return *(first + n);
}


Token const& 
Token_stream::get()
{
  assert(!eof());
  Token const& tok = *first;  
  ++first;
  return tok;
}


} // namespace lingo
