// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "json.hpp"
#include "memory.hpp"
#include "lexing.hpp"
#include "parsing.hpp"

#include <iostream>

namespace lingo 
{

namespace json
{

// Return a textual description of the node name.
char const*
get_node_name(Kind k)
{
  switch(k) {
  case null_value: return "null_value";
  case bool_value: return "bool_value";
  case int_value: return "int_value";
  case real_value: return "real_value";
  case string_value: return "string_value";
  case array_value: return "array_value";
  case object_value: return "object_value";
  default:
    lingo_unreachable("unhandled node kind '{}'", k);
  }
}

// Hash a string value.
std::size_t
String_hash::operator()(String const* p) const
{
  assert(p);
  std::hash<std::string> h;
  return h(p->value());
}


// Compare two strings for equality.
bool 
String_eq::operator()(String const* a, String const* b) const
{
  assert(a && b);
  return a->value() == b->value();
}

// TODO: These should be inlined.

// Returns a reference to the object at the given position.
// If the object does not exist, a new entry is created, allowing
// assignment.
inline Value*& 
Object::operator[](String const* key)
{
  return Object_impl::operator[](key);
}


// Returns the value with the given key. If the key does not
// exist, returns nullptr.
Value*  
Object::operator[](String const* key) const
{
  auto iter = find(key);
  if (iter != end())
    return iter->second;
  else
    return nullptr;
}


Value*&
Object::operator[](char const* key)
{
  return operator[](make_string(key));
}


Value*
Object::operator[](char const* key) const
{
  String s = key;
  return operator[](&s);
}


Value*&
Object::operator[](std::string const& key)
{
  return operator[](make_string(key));
}


Value*
Object::operator[](std::string const& key) const
{
  String s = key;
  return operator[](&s);
}


// -------------------------------------------------------------------------- //
//                            Value creation
//
// This implementation uses a mixed allocation strategy to represent
// values in memory. Specific values (null, true, and false) are 
// singleton objects and must never be deleted. All other values
// (strings, ints, doubles, arrays, and objects) are always dynamically
// allocated as needed.
//
// Allocation of these objects must be done using the make_*() 
// functions and the destroy() function. Never use new and delete 
// with JSON values.

namespace
{

Singleton_factory<Null> null_;
Singleton_factory<Bool> true_;
Singleton_factory<Bool> false_;

} // namespace


// Create a null value.
Null*
make_null()
{
  return null_.make();
}


// Create the true value.
Bool*
make_true()
{
  return true_.make(true);
}


// Create the false value.
Bool*
make_false()
{
  return false_.make(false);
}


// Create an integer value.
Int*
make_int(std::intmax_t n)
{
  return new Int(n);
}


// Create an integer value.
Int*
make_int(Integer n)
{
  return new Int(n);
}


// Create a real value.
Real*
make_real(double d)
{
  return new Real(d);
}


// Create a string.
String*
make_string(char const* str)
{
  return new String(str);
}


String*
make_string(char const* first, char const* last)
{
  return new String(std::string(first, last));
}


// Create a string.
String*
make_string(std::string const& str)
{
  return new String(str);
}


// Create a string from the given view.
String*
make_string(String_view str)
{
  return new String(str.str());
}


// Create an empty array.
Array*
make_array()
{
  return new Array();
}


// Create an array by moving the implementation into the object.
Array*
make_array(Array_impl&& arr)
{
  return new Array(std::move(arr));
}


// Create an empty object.
Object*
make_object()
{
  return new Object();
}


// Create an objectd by moving its implementation into the object.
Object*
make_object(Object_impl&& map)
{
  return new Object(std::move(map));
}


namespace
{

// Delete an allocated literal value.
template<typename T>
inline void 
destroy_literal(T* p)
{
  delete p;
}


// Destroy the sequence of elements in the array.
inline void
destroy_sequence(Array_impl& arr)
{
  for (Value* v : arr)
    destroy(v);
}


// Delete an array.
inline void
destroy_array(Array* a)
{
  destroy_sequence(*a);
  delete a;
}


// Destroy all key-value pairs in the map.
inline void
destroy_mapping(Object_impl& map)
{
  for (Pair& p : map) {
    destroy(const_cast<String*>(p.first));
    destroy(p.second);
  }
}


// Delete an object.
inline void
destroy_object(Object* o)
{
  destroy_mapping(*o);
  delete o;
}


} // namespace


// Destroy the value.
void
destroy(Value* v)
{
  switch (v->kind()) {
  case null_value:
  case bool_value: 
    return;

  case int_value: 
    return destroy_literal(cast<Int>(v));
  
  case real_value: 
    return destroy_literal(cast<Real>(v));
  
  case string_value: 
    return destroy_literal(cast<String>(v));
  
  case array_value: 
    return destroy_array(cast<Array>(v));
  
  case object_value: 
    return destroy_object(cast<Object>(v));
  }
}


// -------------------------------------------------------------------------- //
//                            Pretty printing

void
print_bool(Printer& p, Bool const* value)
{
  if (value->value())
    print(p, "true");
  else
    print(p, "false");
}


template<typename T>
inline void
print_literal(Printer& p, T const* value)
{
  print(p, value->value());
}


inline void
print_string(Printer& p, String const* value)
{
  print_quoted(p, value->value());
}


void
print_array(Printer& p, Array const* value) 
{
  print(p, '[');
  print_nested(p, *value);
  print(p, ']');
}


void
print_object(Printer& p, Object const* value)
{
  print(p, '{');
  print_nested(p, *value);
  print(p, '}');
}


void
print(Printer& p, Pair const& pair)
{
  print(p, pair.first);
  print(p, " : ");
  print(p, pair.second);
}


void 
print_value(Printer& p, Value const* v) 
{
  lingo_assert(v != nullptr);

  switch (v->kind()) {
  case null_value: return print(p, "null");
  case bool_value: return print_bool(p, cast<Bool>(v));
  case int_value: return print_literal(p, cast<Int>(v));
  case real_value: return print_literal(p, cast<Real>(v));
  case string_value: return print_string(p, cast<String>(v));
  case array_value: return print_array(p, cast<Array>(v));
  case object_value: return print_object(p, cast<Object>(v));

  default:
    lingo_unreachable("unhandled node '{}'", v->node_name());
  }
}

void
print(json::Value const* v)
{
  Printer p(std::cout);
  p.needs_newline = true;
  json::print_value(p, v);
}

void
print(Printer& p, json::Value const* v)
{
  json::print_value(p, v);
}


// -------------------------------------------------------------------------- //
//                              Parsing


namespace
{

struct Parser;

Value* parse_value(Parser&, Character_stream&);

struct Parser
{
  using result_type = Value*;
  using argument_type = char;

  Value* operator()(Character_stream& s)
  {
    return parse_value(*this, s);
  }

  Int* on_decimal_integer(Location loc, char const* first, char const* last)
  {
    return make_int(std::string(first, last));
  }
};


// Consume whitespace.
void
discard_whitespace(Character_stream& s)
{
  discard_if(s, &is_whitespace<Character_stream>);
}


// Ensure that whitespace is consumed before and after values.
struct Whitespace_guard
{
  Whitespace_guard(Character_stream& s)
    : s(s)
  {
    discard_whitespace(s);
  }
  ~Whitespace_guard()
  {
    discard_whitespace(s);
  }
  Character_stream& s;
};



// TODO: Improve diagnostics. Emit the sequence of tokens up 
// to the first non-alphabetical character.
//
// TODO: Use the parser for semantic actions.
template<int N, typename Make>
Value*
parse_literal(Parser& p, Character_stream& s, char const* str, Make make)
{
  s.get(); // Accept str[0]

  // FIXME: This awful... It would be great if I could just
  // expand an array (or C-str) into a sequence of arguments.
  if (N == 4) {
    if (match_all(s, str[1], str[2], str[3]))
      return make();
  }
  else if (N == 5) {
    if (match_all(s, str[1], str[2], str[3], str[4]))
      return make();
  }

  throw std::runtime_error(format("invalid {} literal", str));  
}


// Parse a null literal.
//
//    null ::= 'null'
inline Value*
parse_null(Parser& p, Character_stream& s)
{
  return parse_literal<4>(p, s, "null", make_null);
}


// Parse a true literal.
//
//    true ::= 'true'
inline Value*
parse_true(Parser& p, Character_stream& s)
{
  return parse_literal<4>(p, s, "true", make_true);
}


// Parse a false literal.
//
//    false ::= 'false'
inline Value*
parse_false(Parser& p, Character_stream& s)
{
  return parse_literal<5>(p, s, "false", make_false);
}


// Parse a JSON number starting with a digit.
//
// FIXME: Support floating point values.
inline Value*
parse_nonnegative_number(Parser& p, Character_stream& s)
{
  return lex_decimal_integer(p, s, Location::none);
}


// Parse a JSON number starting with a '-' sign.
//
// FIXME: Support floating point values.
inline Value*
parse_negative_number(Parser& p, Character_stream& s)
{
  s.get(); // Consume the '-'
  Int* num = cast<Int>(lex_decimal_integer(p, s, Location::none));
  num->first.neg(); 
  return num;
}


// Parse a string with the usual escape characters. Note that
// the enclosing quotes are not saved with the string.
//
// TODO: Is this sufficiently common that we could lift
// it into the lexing header?
Value*
parse_string(Parser& p, Character_stream& s)
{
  char const* first = &s.get();
  while (next_character_is_not(s, '"')) {
    if (next_character_is(s, '\\'))
      s.get();
    s.get();
  }
  if (char const* last = match(s, '"'))
    return make_string(first + 1, last);
  throw std::runtime_error("unterminated string");
}


// Provides temporary storage for parsing an aray, which
// also allows for proper memory cleanup.
struct Temp_array : Array_impl
{
  ~Temp_array() 
  {
    destroy_sequence(*this);
  }
};


// A helper function for parsing empty arrays. Note that the
// opening bracket has already been matched.
Array*
parse_empty_array(Parser& p, Character_stream& s)
{
  discard_whitespace(s);
  if (match(s, ']'))
    return make_array();
  return nullptr;
}


// Parse an array of values.
//
//    array ::= '[' [value-list] ']'
//    value-list ::= value [','' value-list]
Array*
parse_array(Parser& p, Character_stream& s)
{
  s.get();

  if (Array* a = parse_empty_array(p, s))
    return a;

  // Parse the value-list.
  // FIXME: Factor this using combinators.
  Temp_array arr;
  while (true) {
    Value* v = parse_value(p, s);
    arr.push_back(v);

    if (match(s, ','))
      continue;
    if (match(s, ']'))
      break;

    throw std::runtime_error("ill-formed array");
  }
  return make_array(std::move(arr));
}


// Parse a key in a key value pair. This guarantees
// that whitespace is consumed around the string
// value.
//
//    key ::= string
String*
parse_key(Parser& p, Character_stream& s)
{
  Whitespace_guard ws(s);
  if (next_character_is(s, '"'))
    return cast<String>(parse_string(p, s));
  throw std::runtime_error("ill-formed key");
}


// Parse a key/value pair.
//
//    key-value-pair ::= key ':' value
Pair
parse_pair(Parser& p, Character_stream& s)
{
  String* k = parse_key(p, s);
  if (match(s, ':'))
    return {k, parse_value(p, s)};
  throw std::runtime_error("ill-formed key-value pair");
}


// Provides temporary storage for parsing an object, which
// also allows for proper memory cleanup.
struct Temp_object : Object_impl
{
  ~Temp_object() 
  {
    destroy_mapping(*this);
  }
};


// A helper function for parsing empty objects. Note that the
// opening brace has already been matched.
inline Object*
parse_empty_object(Parser& p, Character_stream& s)
{
  discard_whitespace(s);
  if (match(s, '}'))
    return make_object();
  return nullptr;
}


// Parse an object.
//
//    object ::= '{' [key-value-list] '}'
//    key-value-list ::= key-value-pair [, key-value-list]
//
// Note that the key-value-list may be empty.
Value*
parse_object(Parser& p, Character_stream& s)
{
  s.get();

  if (Object* o = parse_empty_object(p, s))
    return o;

  // Parse the key-value list.
  // FIXME: Factor this using combinators.
  Temp_object map;
  while (true) {
    Pair kv = parse_pair(p, s);
    map.insert(kv);

    if (match(s, ','))
      continue;
    if (match(s, '}'))
      break;
    else
      throw std::runtime_error("ill-formed object");
  }
  return make_object(std::move(map));
}


Value* 
parse_value(Parser& p, Character_stream& s)
{
  Whitespace_guard ws(s);

  if (s.eof())
    return nullptr;

  switch (s.peek()) {
  case 'n':
    return parse_null(p, s);
  
  case 't':
    return parse_true(p, s);
  
  case 'f':
    return parse_false(p, s);

  case '-':
    return parse_negative_number(p, s);
  case '0':
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9':
    return parse_nonnegative_number(p, s);

  case '"':
    return parse_string(p, s);

  case '[':
    return parse_array(p, s);
  
  case '{':
    return parse_object(p, s);

  default:
    throw std::runtime_error(format("unrecognized character '{}'", s.peek()));
  }
}


} // namespace


// Parse the value in the character range `[first, last)`. An 
// exception is thrown if [first, last) does not contain a valid 
// JSON value. 
//
// Note that this will return `nullptr` if the
// range `[first, last)` is empty or only whitespace.
Value*
parse(char const* first, char const* last)
{
  Character_stream chars(first, last);
  Parser parse;
  return parse(chars);
}


// Parse the value in the character range `str`. An exception
// is thrown if `str` does not contain a valid JSON value.
//
// Note that this will return `nullptr` if the
// range `[first, last)` is empty or only whitespace.
Value*
parse(char const* str)
{
  return parse(str, str + std::strlen(str));
}


// Parse the value in the character range `str`. An exception
// is thrown if `str` does not contain a valid JSON value.
//
// Note that this will return `nullptr` if the
// range `[first, last)` is empty or only whitespace.Value*
Value* 
parse(std::string const& str)
{
  return parse(str.c_str(), str.c_str() + str.size());
}


} // namespace json

} // namespace lingo 

