// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#ifndef LINGO_SYMBOL_HPP
#define LINGO_SYMBOL_HPP

#include <lingo/utility.hpp>
#include <lingo/string.hpp>

#include <unordered_map>
#include <typeinfo>


namespace lingo
{


// -------------------------------------------------------------------------- //
//                            Symbols

// The base class of all symbols of a language. By itself, this 
// class is capable of representing symbols that have no other 
// attributes. Examples include punctuators and operators.
class Symbol
{
  friend struct Symbol_table;

public:
  Symbol(int k)
    : str_(nullptr), tok_(k)
  { }

  virtual ~Symbol() { }

  String const& spelling() const { return *str_; }
  int           token() const    { return tok_; }

private:
  String const* str_; // The textual representation
  int           tok_; // The associated token kind
};


// Represents all identifiers.
//
// TODO: Track the innermost binding of the identifier?
struct Identifier_sym : Symbol
{
  Identifier_sym(int k)
    : Symbol(k)
  { }
};


// Represents the integer symbols true and false.
struct Boolean_sym : Symbol
{
  Boolean_sym(int k, bool b)
    : Symbol(k), value_(b)
  { }

  bool value() const { return value_; }

  bool value_;
};


// Represents all integer symbols.
//
// TODO: Develop and use a good arbitrary precision
// integer for this representation.
//
// TOOD: Track the integer base? Technically, that
// can be inferred by the spelling, but it might be
// useful to keep cached.
struct Integer_sym : Symbol
{
  Integer_sym(int k, int n)
    : Symbol(k), value_(n)
  { }

  int value() const { return value_; }

  int value_;
};


// Character symbols are represented by their integer
// encoding in the execution character set. That
// defaults to extended ASCII. Note that this internall
// represnted as a system integer for simplicity.
//
// TODO: Support wide character encoding in ISO 10646
// (Unicode).
//
// TODO: Support configuration of the execution character
// set.
struct Character_sym : Symbol
{
  Character_sym(int k, int n)
    : Symbol(k), value_(n)
  { }

  int value() const { return value_; }

  int value_;
};


// A string symbol contains the representation of the
// string literal in the execution character set.
//
// TODO: Supporting wide string literals would effectively
// require this to be a union of narrow and wide string
// representations.
struct String_sym : Symbol
{
  String_sym(int k, String const& s)
    : Symbol(k), value_(s)
  { }

  String const& value() const { return value_; }

  String value_;
};


// Streaming
std::ostream& operator<<(std::ostream&, Symbol const&);


// -------------------------------------------------------------------------- //
//                           Symbol table


// The symbol table maintains a mapping of
// unique string values to their corresponding
// symbols.
struct Symbol_table : std::unordered_map<std::string, Symbol*>
{
  ~Symbol_table();

  template<typename T, typename... Args>
  Symbol* put(int, String const&, Args&&...);

  template<typename T, typename... Args>
  Symbol* put(int, char const*, char const*, Args&&...);

  Symbol* put_symbol(int, String const&);
  Symbol* put_identifier(int, String const&);
  Symbol* put_boolean(int, String const&, bool);
  Symbol* put_integer(int, String const&, int);
  Symbol* put_character(int, String const&, int);
  Symbol* put_string(int, String const&, String const&);

  Symbol const* get(String const&) const;
  Symbol const* get(char const*) const;
};


// Delete allocated resources.
inline
Symbol_table::~Symbol_table()
{
  for (auto const& x : *this)
    delete x.second;
}


// Insert a new symbol into the table. The spelling of the symbol 
// is given by the string s and the attributes are given in args.
//
// Note that the type of the symbol must be given explicitly, and 
// it must derive from the Symbol class.
//
// If the symbol already exists, no insertion is performed. Note 
// that this must not attempt to re-define the symbol as one of
// a different kind. That is, the dynamic type of the existing
// symbol shall be the same as T.
template<typename T, typename... Args>
Symbol*
Symbol_table::put(int k, String const& s, Args&&... args)
{
  auto x = emplace(s, nullptr);
  auto iter = x.first;
  Symbol*& sym = iter->second;
  if (x.second) {
    sym = new T(k, std::forward<Args>(args)...);
    sym->str_ = &iter->first;
  } else {
    lingo_assert(is<T>(sym));
  }
  return iter->second;
}


// Insert a symbol with the spelling [first, last) and the 
// properties in args...
template<typename T, typename... Args>
inline Symbol*
Symbol_table::put(int k, char const* first, char const* last, Args&&... args)
{
  String s(first, last);
  return this->template put<T>(k, s, std::forward<Args>(args)...);
}


inline Symbol*
Symbol_table::put_symbol(int k, String const& s)
{
  return put<Symbol>(k, s);
}


inline Symbol*
Symbol_table::put_identifier(int k, String const& s)
{
  return put<Identifier_sym>(k, s);
}


inline Symbol*
Symbol_table::put_boolean(int k, String const& s, bool b)
{
  return put<Boolean_sym>(k, s, b);
}


inline Symbol*
Symbol_table::put_integer(int k, String const& s, int n)
{
  return put<Integer_sym>(k, s, n);
}


inline Symbol*
Symbol_table::put_character(int k, String const& s, int c)
{
  return put<Integer_sym>(k, s, c);
}


// Insert a string symbol whose spelling in the basic
// character set is given in s1, and whose spelling in the
// execution character set is given in s2.
inline Symbol*
Symbol_table::put_string(int k, String const& s1, String const& s2)
{
  return put<String_sym>(k, s1, s2);
}


// Returns the symbol with the given spelling or
// nullptr if no such symbol exists.
inline Symbol const*
Symbol_table::get(String const& s) const
{
  auto iter = find(s);
  if (iter != end())
    return iter->second;
  else
    return nullptr;
}


// Returns the symbol with the given spelling or
// nullptr if no such symbol exists.
inline Symbol const*
Symbol_table::get(char const* s) const
{
  auto iter = find(s);
  if (iter != end())
    return iter->second;
  else
    return nullptr;
}


} // namespace lingo

#endif
