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

Symbol_table::~Symbol_table()
{
  for (auto p : map_)
    delete p.second;
}


// Insert a symbol as having the given kind. If the symbol already
// exists, do nothing and return its index.
//
// Note that a symbol's kind cannot be changed by re-insertion.
// This means that inserting a symbol as an identifier when it has 
// been previously installed as a keyword, will simply return the
// keyword.
//
// A symbol's kind can be modified after initialization if needed.
Symbol&
Symbol_table::insert(String_view s, int k)
{
  auto ins = map_.insert({s, nullptr});
  if (ins.second) {
    // Create a new symbol for the given string.
    Symbol* sym = new Symbol(s.str(), k);
    ins.first->second = sym;
    return *sym;
  } else {
    // Return the existing symbol.
    return *ins.first->second;
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
