// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#ifndef LINGO_JSON_HPP
#define LINGO_JSON_HPP

// The JSON module provides facilities for working with JSON
// values, including parsing and pretty printing. JSON is used
// internally in lingo as a dynamic value system for e.g.,
// application configuration.
//
// This is actually an extension of traditional JSON. In particular,
// we differentiate between ints and reals, and allow unquoted
// identifiers as strings.
//
// TODO: Move this into examples.

#include "lingo/node.hpp"
#include "lingo/integer.hpp"
#include "lingo/string.hpp"

#include <unordered_map>
#include <vector>

namespace lingo
{

namespace json
{

// -------------------------------------------------------------------------- //
//                                  Types

enum Kind
{
  null_value,   // null
  bool_value,   // true/false
  int_value,    // integers
  real_value,   // reals
  string_value, // strings
  array_value,  // arrays
  object_value  // objects
};


// Declarations
struct Value;
struct Undefined;
struct Null;
struct Bool;
struct Int;
struct Real;
struct String;
struct Array;
struct Object;


// Hash function for JSON strings.
struct String_hash
{
  std::size_t operator()(String const*) const;
};


// Equality function for JSON strings.
struct String_eq
{
  bool operator()(String const*, String const*) const;
};


// Implementation types.
using Array_impl = std::vector<Value*>;
using Object_impl = std::unordered_map<String const*, Value*, String_hash, String_eq>;


// The type of Key/value pairs in JSON.
using Pair = Object_impl::value_type;


// -------------------------------------------------------------------------- //
//                                JSON Values


// The value class is the base class of all json values. 
//
// Note that this is the internal representation of those values;
// we don't keep source locations for these.
struct Value
{
  virtual ~Value() { }
};


// A helper class for the definition of literal-valued terms.
// This includes booleans, integers, reals, and strings.
template<typename T>
struct Literal : Value
{
  template<typename... Args>
  Literal(Args&&... args)
    : first(std::forward<Args>(args)...)
  { }

  T const& value() const { return first; }

  T first;
};


// The literal value null.
struct Null : Value
{
};



// Represents the literal values true and false.
struct Bool : Literal<bool> 
{
  using Literal<bool>::Literal;
};


// Represents integer-valued literals.
struct Int : Literal<Integer> 
{
  using Literal<Integer>::Literal;
};


// Represents real-valued literals.
//
// FIXME: Use a more general purpose real number for the value.
struct Real : Literal<double> 
{
  using Literal<double>::Literal;
};


// Represents string-valued literals.
//
// TODO: Strings are immutable. Perhaps we could use a shared
// representation to minimize allocations? 
struct String : Literal<std::string> 
{
  using Literal<std::string>::Literal;
};


// An array of values.
struct Array : Value, Array_impl
{
  Array() = default;

  Array(std::initializer_list<Value*> list)
    : Array_impl(list)
  { }

  Array(Array_impl&& arr)
    : Array_impl(std::move(arr))
  { }
};


// A mapping of strings to values, or a set of 
// name/value pairs.
struct Object : Value, Object_impl
{
  Object() = default;

  Object(std::initializer_list<Pair> list)
    : Object_impl(list)
  { }

  Object(Object_impl&& map)
    : Object_impl(std::move(map))
  { }

  Value*& operator[](String const*);
  Value*  operator[](String const*) const;

  Value*& operator[](char const*);
  Value*  operator[](char const*) const;

  Value*& operator[](std::string const&);
  Value*  operator[](std::string const&) const;
};


// -------------------------------------------------------------------------- //
//                            Value creation

Null* make_null();
Bool* make_true();
Bool* make_false();
Int* make_integer(std::intmax_t);
Int* make_integer(Integer);
Real* make_real(double);
String* make_string(char const*);
String* make_string(char const*, char const*);
String* make_string(std::string const&);
String* make_string(String_view);
Array* make_array();
Array* make_array(Array_impl&&);
Object* make_object();
Object* make_object(Object_impl&&);

void destroy(Value*);


// -------------------------------------------------------------------------- //
//                            Pretty printing

void print(json::Value const*);
void print(Printer& p, json::Value const*);


// -------------------------------------------------------------------------- //
//                            Parsing

Value* parse(char const*, char const*);
Value* parse(char const*);
Value* parse(std::string const&);


} // namespace json

} // namespace lingo

#endif
