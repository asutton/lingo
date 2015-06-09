// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "lingo/symbol.hpp"
#include "lingo/print.hpp"

#include <cassert>
#include <iostream>

namespace lingo
{

// -------------------------------------------------------------------------- //
//                             Symbols
//
// TODO: Check that the symbol has the appropriate
// kind before accessing its bindings or other
// (eventual) attributes.

// Push a new binding onto the symbol.
void
push_binding(Symbol& s, void* p)
{
  Binding* bind = new Binding(p);
  Binding*& orig = s.data.bind;
  if (orig)
    bind->prev = orig;
  orig = bind;
}


// Pop a binding from the symbol.
void
pop_binding(Symbol& s)
{
  assert(s.data.bind);
  Binding*& orig = s.data.bind;
  Binding* bind = orig;
  orig = orig->prev;
  delete bind;
}


// Get the binding associated with the symbol. If there
// is no binding for this symbol, this returns nullptr.
void*
get_binding(Symbol const& s)
{
  if (s.data.bind)
    return s.data.bind->info;
  else
    return nullptr;
}


// -------------------------------------------------------------------------- //
//                           Symbol table

// Insert a symbol as having the given kind. If the symbol already
// exists, do nothing and return its index.
int
Symbol_table::insert(String_view s, Symbol_kind k)
{
  auto iter = map_.find(s);
  if (iter == map_.end()) {
    int n = syms_.size();
    syms_.emplace_back(s, k);
    map_.insert({s, n});
    return n;
  } else {
    return iter->second;
  }
}


int 
Symbol_table::insert(char const* s, Symbol_kind k)
{
  return insert(String_view(s), k);
}


int 
Symbol_table::insert(char const* f, char const* l, Symbol_kind k)
{
  return insert(String_view(f, l), k);
}


int
Symbol_table::lookup(String_view s) const
{
  auto iter = map_.find(s);
  if (iter != map_.end())
    return iter->second;
  else
    return -1;
}


// Return the entry at the given position.
Symbol&
Symbol_table::entry(int n)
{
  assert(0 <= n && n <= (int)syms_.size());
  return syms_[n];
}


const Symbol&
Symbol_table::entry(int n) const
{
  assert(0 <= n && n <= (int)syms_.size());
  return syms_[n];
}

// Returns the symbol corresponding to the given
// string. Note that the string must be in the
// symbol table.
Symbol&
Symbol_table::entry(String_view s)
{
  return entry(lookup(s));
}


Symbol const& 
Symbol_table::entry(String_view s) const
{
  return entry(lookup(s));
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


// Returns the entry of the symbol in [str, str + n) or inserts it if
// not already present.
int
get_symbol_entry(char const* str, int n)
{
  return symbols().insert(str, str + n, 0);
}


// Returns the entry of the symbol in [first, last) or inserts it if
// not already present.
int
get_symbol_entry(char const* first, char const* last)
{
  return symbols().insert(first, last, 0);
}


// Returns the symbol correspondng to `str`, inserting a new
// symbol if it is not already present.
Symbol*
get_symbol(char const* str)
{
  Symbol_table& syms = symbols();
  return &syms.entry(syms.insert(str, 0));
}


// Returns the symbol correspondng to the string in `[first, last)`.
// Insert the symbol if it does not exist.
Symbol*
get_symbol(char const* first, char const* last)
{
  String_view str(first, last);
  Symbol_table& syms = symbols();
  return &syms.entry(syms.insert(str, 0));
}


} // namespace lingo
