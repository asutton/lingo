// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#ifndef LINGO_SYMBOL_HPP
#define LINGO_SYMBOL_HPP

// The symbol module defines a symbol table and related
// facilities. A symbol is a view of a string in a source
// file and affiliated data (e.g., bindings).

#include "lingo/string.hpp"
#include "lingo/integer.hpp"

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
// An implementation shuld derive from this class to provide
// name bindings.
struct Binding 
{
  Binding* prev = nullptr;
};


// -------------------------------------------------------------------------- //
//                           Symbol data

// Information associated with a symbol. This includes the kind
// of token, and token-specific data.
union Symbol_data
{
  Symbol_data() = default;

  Symbol_data(Binding* b)
    : bind(b) 
  { }

  Symbol_data(Integer const& z)
    : zval(new Integer(z))
  { }
  
  Binding* bind; // An name binding
  Integer* zval; // An integer interpretation
};


// -------------------------------------------------------------------------- //
//                                  Symbols

// Determines the kinds of attributes associated with the symbol.
//
// TODO: Add extra attributes for other stuff?
using Symbol_kind = int;
constexpr int unspecified_sym = 0;
constexpr int language_sym    = 1; // Defined by the language (no attributes)
constexpr int integer_sym     = 2; // Has an integer attribute
constexpr int real_sym        = 3; // Has a real-valued attribute
constexpr int identifier_sym  = 4; // Has a binding attribute


// The symbol descriptor contains information about a symbol
// in the symbol table.
//
// NOTE: The bitfields can be rebalanced at need.
struct Symbol_descriptor
{
  int kind  : 8;  // The kind of keyword (see above)
  int token : 24; // An associated token kind
};


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
  Symbol(String const& s, Symbol_descriptor d)
    : str(s), desc(d)
  { }

  String_view view() const { return make_view(str); }

  String            str;  // The string view
  Symbol_descriptor desc; // The kind of token
  Symbol_data       data; // Supplemental data
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


// Returns true if the symbol is one defined by the core
// language. Note that core language symbols have only
// one possible spelling (although multiple spellings may
// be associated with the same token kind).
inline bool
is_language_symbol(Symbol const& s)
{
  return s.desc.kind == language_sym;
}


void push_binding(Symbol&, Binding*);
Binding* pop_binding(Symbol&);
Binding* get_binding(Symbol&);


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
  Symbol& insert(String_view, Symbol_descriptor);
  Symbol& insert(char const*, Symbol_descriptor);
  Symbol& insert(char const*, char const*, Symbol_descriptor);

  Symbol* lookup(String_view) const;
  Symbol* lookup(char const*) const;
  Symbol* lookup(char const*, char const*) const;

  void clear();

private:
  List syms_;
  Map  map_;
};


inline Symbol&
Symbol_table::insert(char const* s, Symbol_descriptor k)
{
  return insert(String_view(s), k);
}


inline Symbol& 
Symbol_table::insert(char const* f, char const* l, Symbol_descriptor k)
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
get_symbol(char const* str)
{
  return symbols().insert(str, {});
}


// Returns the symbol correspondng to the string in `[first, last)`.
// Insert the symbol if it does not exist.
inline Symbol&
get_symbol(char const* first, char const* last)
{
  return symbols().insert(first, last, {});
}


// Returns the symbol corresponding to the stirng `s`. Insert
// the symbol if it does not exist.
inline Symbol&
get_symbol(String_view s)
{
  return symbols().insert(s, {});
}


// Returns the symbol corresponding to the stirng `s`. Insert
// the symbol if it does not exist.
inline Symbol&
get_symbol(String const& s)
{
  return symbols().insert(make_view(s), {});
}


// Return a new string, interned in the symbol table and
// return its string representation.
inline String const&
get_symbol_string(char const* str)
{
  return get_symbol(str).str;
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
