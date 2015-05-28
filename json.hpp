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


char const* get_node_name(Kind);


// Partial specialization of the kinding facility.
template<Kind K>
using Kind_of = lingo::Kind_of<Kind, K>;


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
using Object_impl = std::unordered_map<String*, Value*, String_hash, String_eq>;


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
  Value(Kind k)
    : kind_(k)
  { }

  virtual ~Value() { }

  char const* node_name() const { return get_node_name(kind_); }
  Kind kind() const { return kind_; }

  Kind kind_;
};


// A helper class for the definition of literal-valued terms.
// This includes booleans, integers, reals, and strings.
template<Kind K, typename T>
struct Literal : Value, Kind_of<K>
{
  Literal(T const& x)
    : Value(K), first(x)
  { }

  T const& value() const { return first; }

  static bool is(Kind k) { return k == K; }

  T first;
};


// The literal value null.
struct Null : Value, Kind_of<null_value>
{
  Null()
    : Value(node_kind)
  { }
};



// Represents the literal values true and false.
struct Bool : Literal<bool_value, bool> 
{
  using Literal<bool_value, bool>::Literal;
};


// Represents integer-valued literals.
struct Int : Literal<int_value, Integer> 
{
  using Literal<int_value, Integer>::Literal;
};


// Represents real-valued literals.
//
// FIXME: Use a more general purpose real number for the value.
struct Real : Literal<real_value, double> 
{
  using Literal<real_value, double>::Literal;
};


// Represents string-valued literals.
//
// TODO: Strings are immutable. Perhaps we could use a shared
// representation to minimize allocations? 
struct String : Literal<string_value, std::string> 
{
  using Literal<string_value, std::string>::Literal;
};


// An array of values.
struct Array : Value, Array_impl, Kind_of<array_value>
{
  Array()
    : Value(node_kind)
  { }

  Array(std::initializer_list<Value*> list)
    : Value(node_kind), Array_impl(list)
  { }

  Array(Array_impl&& arr)
    : Value(node_kind), Array_impl(std::move(arr))
  { }
};


// A mapping of strings to values, or a set of 
// name/value pairs.
struct Object : Value, Object_impl, Kind_of<object_value>
{
  Object()
    : Value(node_kind)
  { }

  Object(std::initializer_list<Pair> list)
    : Value(node_kind), Object_impl(list)
  { }

  Object(Object_impl&& map)
    : Value(node_kind), Object_impl(std::move(map))
  { }
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
