// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#ifndef LINGO_SYMBOL_HPP
#define LINGO_SYMBOL_HPP

// The symbol module defines a symbol table and related
// facilities. A symbol is a view of a string in a source
// file and affiliated data (e.g., bindings).

#include <lingo/string.hpp>
#include <lingo/integer.hpp>

#include <cstring>
#include <list>
#include <string>
#include <unordered_map>


namespace lingo
{

// -------------------------------------------------------------------------- //
//                                  Symbols

// The unknown token kind. No token must ever have this kind.
constexpr int unknown_tok = -1;


// A Symbol represents a lexeme saved in the symbol table and
// its associated attributes. The string representation of symbols
// are represented as a pair of pointers into a character array.
// Additional attributes include the kind of token and the token
// specific data.
//
// Note that the symbol owns the pointer to its string view and is
// responsible for de-allocating that memory.
//
// The symbol also associates a token kind, allowing for efficient
// token construction. Most symbols are inserted with the token
// kind already known. However, for tokens like identifiers, integers,
// and real values, the token kind must be assigned later.
//
struct Symbol
{
  Symbol(String const& s, int k)
    : str(s), kind(k)
  { }

  String_view view() const { return make_view(str); }

  String str;  // The string view
  int    kind; // The kind of token
};


inline bool
operator==(Symbol const& a, Symbol const& b)
{
  return a.str == b.str;
}


inline bool
operator!=(Symbol const& a, Symbol const& b)
{
  return a.str != b.str;
}


// -------------------------------------------------------------------------- //
//                           Symbol table

// A symbol table stores unique representations of strings in
// a program and their affiliated information (e.g., token kind,
// etc.). The symbol table also supports efficient insertion and
// lookup of those strings.
//
// The symbol table is implemented as a linked list of symbols
// with a side table to support efficient lookup.
//
// TODO: Use a bump alloctor for the hash table and the strings.
// Because the table is only ever cleared on program exit, it
// would be far more effecient to simply blow away all pages
// allocated to the symbol table.
class Symbol_table
{
  using Hash = String_view_hash;
  using Eq = String_view_eq;
  using Map = std::unordered_map<String_view, Symbol*, Hash, Eq>;

public:
  ~Symbol_table();

  Symbol& insert(String_view, int);
  Symbol& insert(char const*, int);
  Symbol& insert(char const*, char const*, int);

  Symbol* lookup(String_view) const;
  Symbol* lookup(char const*) const;
  Symbol* lookup(char const*, char const*) const;

private:
  Map  map_;
};


inline Symbol&
Symbol_table::insert(char const* s, int k)
{
  return insert(String_view(s), k);
}


inline Symbol&
Symbol_table::insert(char const* f, char const* l, int k)
{
  return insert(String_view(f, l), k);
}


inline Symbol*
Symbol_table::lookup(char const* s) const
{
  return lookup(String_view(s));
}


inline Symbol*
Symbol_table::lookup(char const* f, char const* l) const
{
  return lookup(String_view(f, l));
}


Symbol_table& symbols();


// Returns the symbol correspondng to `str`, inserting a new
// symbol if it is not already present.
inline Symbol&
get_symbol(char const* str, int k = unknown_tok)
{
  return symbols().insert(str, k);
}


// Returns the symbol correspondng to the string in `[first, last)`.
// Insert the symbol if it does not exist.
inline Symbol&
get_symbol(char const* first, char const* last, int k = unknown_tok)
{
  return symbols().insert(first, last, k);
}


// Returns the symbol corresponding to the stirng `s`. Insert
// the symbol if it does not exist.
inline Symbol&
get_symbol(String_view s, int k = unknown_tok)
{
  return symbols().insert(s, k);
}


// Returns the symbol corresponding to the stirng `s`. Insert
// the symbol if it does not exist.
inline Symbol&
get_symbol(String const& s, int k = unknown_tok)
{
  return symbols().insert(make_view(s), k);
}


// Return a pointer to a unique representation of the
// given string. Inserts a symbol into the symbol table
// where appropreate.
inline String const*
get_string(char const* str)
{
  return &get_symbol(str).str;
}


// Return a the symbol for the given string or nullptr if the
// symbol is not in the table.
inline Symbol*
lookup_symbol(char const* s)
{
  return symbols().lookup(s);
}


// Return a the symbol for the given string or nullptr if the
// symbol is not in the table.
inline Symbol*
lookup_symbol(char const* first, char const* last)
{
  return symbols().lookup(first, last);
}


// Return a the symbol for the given string or nullptr if the
// symbol is not in the table.
inline Symbol*
lookup_symbol(String_view s)
{
  return symbols().lookup(s);
}


// Return a the symbol for the given string or nullptr if the
// symbol is not in the table.
inline Symbol*
lookup_symbol(String const& s)
{
  return symbols().lookup(make_view(s));
}


} // namespace lingo

#endif
