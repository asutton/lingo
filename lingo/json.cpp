// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "json.hpp"
#include "memory.hpp"
#include "character.hpp"
#include "lexing.hpp"
#include "parsing.hpp"

#include <iostream>

namespace lingo 
{

namespace json
{

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


// Provides temporary storage for parsing an aray, which
// also allows for proper memory cleanup.
struct Temp_array : Array_impl
{
  ~Temp_array() 
  {
    destroy_sequence(*this);
  }
};


// Provides temporary storage for parsing an object, which
// also allows for proper memory cleanup.
struct Temp_object : Object_impl
{
  ~Temp_object() 
  {
    destroy_mapping(*this);
  }
};


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

// The parse context provides information about the state
// of a parse. It provides the hooks for transforming characters
// into values and emitting errors.
struct Context 
{
  using result_type = Value*;

  Null* on_null(Location)
  {
    return make_null();
  }

  Bool* on_true(Location)
  {
    return make_true();
  }

  Bool* on_false(Location)
  {
    return make_false();
  }

  Int* on_decimal_integer(Location, char const* first, char const* last)
  {
    return make_int(std::string(first, last));
  }

  String* on_string(Location, char const* first, char const* last)
  {
    return make_string(first, last);
  }

  Array* on_array(Location)
  {
    return make_array();
  }

  Array* on_array(Location, Temp_array&& a)
  {
    return make_array(std::move(a));
  }

  Object* on_object(Location)
  {
    return make_object();
  }

  Object* on_object(Location, Temp_object&& o)
  {
    return make_object(std::move(o));
  }

  void on_expected(Location, char c)
  {
    throw std::runtime_error(format("expected '{}'", c));
  }

  void on_expected(Location, char const* cond)
  {
    throw std::runtime_error(format("expected '{}'", cond));
  }
};


// An alias for an underlying character stream.
using Stream = Character_stream;


// Returns true if the current character is one of the
// punctuation characters. Note that s.eof() is false.
bool
is_punctuation(char c)
{
  switch (c) {
  case '[':
  case ']':
  case '{':
  case '}':
  case '"':
  case ',':
  case ':':
    return true;
  default:
    return is_space(c);
  }
}


// Consume whitespace.
void
discard_whitespace(Stream& s)
{
  auto pred = [](Stream& s) { return next_element_if(s, is_space); };
  discard_if(s, pred);
}


// Ensure that whitespace is consumed before and after values.
struct Whitespace_guard
{
  Whitespace_guard(Stream& s)
    : s(s)
  {
    discard_whitespace(s);
  }
  ~Whitespace_guard()
  {
    discard_whitespace(s);
  }
  Stream& s;
};


Value* parse_value(Context&, Stream&);


// Support for parsing literal values exactly.
//
// TODO: For the specializations below, it would be more efficient
// to have an unbounded lookahead to search for the next non-punctuation 
// character and then analyze the result.
template<int N>
struct Literal_parser;


// 4-character literals.
template<>
struct Literal_parser<4>
{
  template<typename Make>
  Value* operator()(Context& cxt, Stream& s, char const* lit, Make make) const
  {
    Location loc = s.location();
    s.get(); // match lit[0]
    if (match_all(s, lit[1], lit[2], lit[3]))
      if (s.eof() || is_space(s.peek()) || is_punctuation(s.peek()))
        return (cxt.*make)(loc);
    return nullptr;
  }
};


// 5-character literals.
template<>
struct Literal_parser<5>
{
  template<typename Make>
  Value* operator()(Context& cxt, Stream& s, char const* lit, Make make) const
  {
    Location loc = s.location();
    s.get(); // match lit[0]
    if (match_all(s, lit[1], lit[2], lit[3], lit[4]))
      if (s.eof() || is_space(s.peek() || is_punctuation(s.peek())))
        return (cxt.*make)(loc);
    return nullptr;
  }
};


// TODO: Improve diagnostics. Emit the sequence of tokens up 
// to the first non-alphabetical character.
//
// TODO: Use the parser for semantic actions.
template<int N, typename Make>
Value*
parse_literal(Context& cxt, Stream& s, char const* str, Make make)
{
  Literal_parser<N> parse;
  if (Value* v = parse(cxt, s, str, make))
    return v;
  throw std::runtime_error(format("invalid {} literal", str));  
}


// Parse a null literal.
//
//    null ::= 'null'
inline Value*
parse_null(Context& cxt, Stream& s)
{
  return parse_literal<4>(cxt, s, "null", &Context::on_null);
}


// Parse a true literal.
//
//    true ::= 'true'
inline Value*
parse_true(Context& cxt, Stream& s)
{
  return parse_literal<4>(cxt, s, "true", &Context::on_true);
}


// Parse a false literal.
//
//    false ::= 'false'
inline Value*
parse_false(Context& cxt, Stream& s)
{
  return parse_literal<5>(cxt, s, "false", &Context::on_false);
}


// Parse a JSON number starting with a digit.
//
// FIXME: Support floating point values.
inline Value*
parse_nonnegative_number(Context& cxt, Stream& s)
{
  return lex_decimal_integer(cxt, s, Location::none);
}


// Parse a JSON number starting with a '-' sign.
//
// FIXME: Support floating point values.
//
// FIXME: This doesn't really support the p.on_xxx concept too
// well because we're using a canned lexer. Perhaps we should
// provide a flag that indicates negation? Positivity?
inline Value*
parse_negative_number(Context& cxt, Stream& s)
{
  s.get();
  Int* num = cast<Int>(lex_decimal_integer(cxt, s, Location::none));
  num->first.neg(); 
  return num;
}


// Parse a string with the usual escape characters. Note that
// the enclosing quotes are not saved with the string.
//
// TODO: Is this sufficiently common that we could lift
// it into the lexing header?
//
// TODO: It might be more effecient to search for the end '"'
// without calling get(). Note that we would have to advance
// later, and if there were newlines in the string, we'd have
// to search for and find those.
Value*
parse_string(Context& cxt, Stream& s)
{
  Location loc = s.location();
  char const* first = &s.get();
  while (next_element_is_not(s, '"')) {
    if (next_element_is(s, '\\'))
      s.get();
    s.get();
  }
  if (char const* last = expect(cxt, s, '"'))
    return cxt.on_string(loc, first + 1, last);
  else
    return {};
}


// A helper function for parsing empty arrays. Note that the
// opening bracket has already been matched.
Array*
parse_empty_array(Context& cxt, Stream& s)
{
  discard_whitespace(s);
  if (match(s, ']'))
    return cxt.on_array(Location::none);
  return nullptr;
}


// Parse an array of values.
//
//    array ::= '[' [value-list] ']'
//    value-list ::= value [','' value-list]
Array*
parse_array(Context& cxt, Stream& s)
{
  require(s, '[');
  
  if (Array* a = parse_empty_array(cxt, s))
    return a;

  Temp_array arr;
  do {
    Value* v = parse_value(cxt, s);
    arr.push_back(v);
  } while (match(s, ','));
  expect(cxt, s, ']');
  
  return cxt.on_array(Location::none, std::move(arr));
}


// Parse a key in a key value pair. This guarantees
// that whitespace is consumed around the string
// value.
//
//    key ::= string
String*
parse_key(Context& cxt, Stream& s)
{
  Whitespace_guard ws(s);
  if (next_element_is(s, '"'))
    return cast<String>(parse_string(cxt, s));
  throw std::runtime_error("ill-formed key");
}


// Parse a key/value pair.
//
//    key-value-pair ::= key ':' value
Pair
parse_pair(Context& cxt, Stream& s)
{
  String* k = parse_key(cxt, s);
  if (match(s, ':'))
    return {k, parse_value(cxt, s)};
  throw std::runtime_error("ill-formed key-value pair");
}


// A helper function for parsing empty objects. Note that the
// opening brace has already been matched.
inline Object*
parse_empty_object(Context& cxt, Stream& s)
{
  discard_whitespace(s);
  if (match(s, '}'))
    return cxt.on_object(Location::none);
  return nullptr;
}


// Parse an object.
//
//    object ::= '{' [key-value-list] '}'
//    key-value-list ::= key-value-pair [, key-value-list]
//
// Note that the key-value-list may be empty.
Value*
parse_object(Context& cxt, Stream& s)
{
  s.get();

  if (Object* o = parse_empty_object(cxt, s))
    return o;

  // TODO: There is a generic algorithm here. See the
  // parse_array function. We need a good strategy for
  // generalizing the accumulated elements.
  Temp_object map;
  do {
    Pair kv = parse_pair(cxt, s);
    map.insert(kv);
  } while (match(s, ','));
  expect(cxt, s, '}');

  return cxt.on_object(Location::none, std::move(map));
}


// Parse a value from the stream. Note that 
Value* 
parse_value(Context& cxt, Stream& s)
{
  Whitespace_guard ws(s);

  if (s.eof())
    throw std::runtime_error("missing value");

  switch (s.peek()) {
  case 'n':
    return parse_null(cxt, s);
  
  case 't':
    return parse_true(cxt, s);
  
  case 'f':
    return parse_false(cxt, s);

  case '-':
    return parse_negative_number(cxt, s);
  
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
    return parse_nonnegative_number(cxt, s);

  case '"':
    return parse_string(cxt, s);

  case '[':
    return parse_array(cxt, s);
  
  case '{':
    return parse_object(cxt, s);

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
//
// FIXME: Re-think how this should work.
Value*
parse(char const* first, char const* last)
{
  // Stream chars(first, last);
  // Context cxt;
  // return parse_value(cxt, chars);

  return nullptr;
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

