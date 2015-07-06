// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "lingo/symbol.hpp"
#include "lingo/print.hpp"
#include "lingo/error.hpp"

#include <cassert>
#include <cstring>
#include <iostream>

namespace lingo
{


// -------------------------------------------------------------------------- //
//                           Symbol table

// Insert a symbol as having the given kind. If the symbol already
// exists, do nothing and return its index.
Symbol&
Symbol_table::insert(String_view s, Symbol_descriptor k)
{
  auto iter = map_.find(s);
  if (iter == map_.end()) {
    // Create/insert a new symbol for the given string.
    syms_.emplace_back(s.str(), k);
    Symbol* sym = &syms_.back();

    // Insert the string into the lookup table.
    map_.insert({sym->view(), sym});
    return *sym;
  } else {
    return *iter->second;
  }
}


Symbol*
Symbol_table::lookup(String_view s) const
{
  auto iter = map_.find(s);
  if (iter != map_.end())
    return iter->second;
  else
    return nullptr;
}


namespace
{

// The global symbol table.
Symbol_table sym_;

} // namespace


// Returns the global symbol table.
Symbol_table& 
symbols()
{
  return sym_;
}


} // namespace lingo
