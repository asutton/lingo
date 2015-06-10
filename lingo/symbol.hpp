// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#ifndef LINGO_SYMBOL_HPP
#define LINGO_SYMBOL_HPP

// The symbol module defines a symbol table and related
// facilities. A symbol is a view of a string in a source
// file and affiliated data (e.g., bindings).

#include "lingo/string.hpp"

#include <cstring>
#include <list>
#include <string>
#include <unordered_map>

namespace lingo
{

// -------------------------------------------------------------------------- //
//                         Identifier bindings

// A binding associates the use of a name with different
// kinds of information: type, value, etc. This information
// is represented as a pointer within the binding object.
//
// An identifier maybe bound to different types or values
// in different scopes. Each new binding results in a
// list with a pointer to the previous entry.
struct Binding 
{
  Binding(void* p)
    : info(p), prev(nullptr)
  { }

  void*    info;
  Binding* prev;
};


// -------------------------------------------------------------------------- //
//                           Symbol data

// Information associated with a symbol. This includes the kind
// of token, and token-specific data.
//
// TODO: The binding must a union. Improve this data structure.
struct Symbol_data
{
  Symbol_data()
    : bind(nullptr) { }
  
  int      kw;    // The keyword token kind
  Binding* bind;  // An identifier binding
};


// -------------------------------------------------------------------------- //
//                                  Symbols


// The symbol kind is a classifier for the symbol. 
// Specific values are typically defind by the lexer.
//
// FIXME: We need stronger guarantees about the
// kind of symbol so we can actually check the kind
// when referring through symbol data.
using Symbol_kind = int;

constexpr Symbol_kind unspcified_sym = 0;
constexpr Symbol_kind keyword_sym    = 1;
constexpr Symbol_kind identifier_sym = 2;


// A Symbol represents a lexeme saved in the symbol table and
// its associated attributes. The string representation of symbols 
// are represented as a pair of pointers into a character array.
// Additional attributes include the kind of token and the token
// specific data.
//
// Note that the symbol owns the pointer to its string view and is
// responsible for de-allocating that memory.
//
// TODO: What kinds of information can we add to this entry
// to simplify functioning.
//
// TODO: When we start using a reasonable allocator for the symbol
// table, revisit the destructor.
struct Symbol
{
  ~Symbol();

  Symbol(String_view s, Symbol_kind k)
    : str(s), kind(k)
  { }

  Symbol(char const* s, Symbol_kind k)
    : str(s), kind(k)
  { }

  Symbol(char const* f, char const* l, Symbol_kind k)
    : str(f, l), kind(k)
  { }

  String_view str;   // The string view
  Symbol_kind kind;  // The kind of token
  Symbol_data data;  // Supplemental data
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


void push_binding(Symbol&, void*);
void pop_binding(Symbol&);
void* get_binding(Symbol const&);


// -------------------------------------------------------------------------- //
//                           Symbol table

// A symbol table stores unique representations of strings in
// a program and their affiliated information (e.g., token kind, 
// etc.). The symbol table also supports efficient insertion and 
// lookup of those strings.
//
// The symbol table is implemented as a linked list of symbols 
// with a side table to support efficient lookup.
class Symbol_table
{
  using Hash = String_view_hash;
  using Eq = String_view_eq;
  using List = std::list<Symbol>;
  using Map = std::unordered_map<String_view, Symbol*, Hash, Eq>;

public:
  Symbol& insert(String_view, Symbol_kind);
  Symbol& insert(char const*, Symbol_kind);
  Symbol& insert(char const*, char const*, Symbol_kind);

  Symbol* lookup(String_view) const;

  void clear();

private:
  List syms_;
  Map  map_;
};


Symbol_table& symbols();
Symbol&       get_symbol(char const*);
Symbol&       get_symbol(char const*, char const*);


// Return a new string, interned in the symbol table. 
// Note that `str` must have a lifetime longer than the 
// symbol table.
inline String_view
get_symbol_string(char const* str)
{
  return get_symbol(str).str;
}

} // namespace lingo

#endif
