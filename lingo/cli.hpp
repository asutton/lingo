// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#ifndef LINGO_CLI_HPP
#define LINGO_CLI_HPP

// The argument module provides tools for parsing command line
// arguments. The result of parsing command line arguments
// is a JSON object associating the named options with their
// assigned values.
//
// There are two components of the argument parser:
//
//    - a method of specifying command line parameters,
//    - the parser itself.
//
// TODO: Make this better.
//
// TODO: This is a configuration library, not just CLI stuff.

#include <lingo/json.hpp>

namespace lingo
{

namespace cli
{

// Different kinds of parameters. A flag is a simple switch
// used to enable or disable a feature. A value parameter
// associates the flag with a value.
enum Parameter_kind
{
  flag,
  value,
};


// A parameter is a specification of arguments that can be accepted
// on the command line.
//
// A parameter name can have two forms: a short form and a long
// form. These are combined by the parameter's constructor as:
//
//    "long,c"
//
// Where `long` is the long name of the parameter, and `c` is a single
// character abbreviation. The abbreviation may be omitted if there
// is no short form.
//
// TODO: Ths isn't just a specification of parameters. This needs to
// be a specification of the app configuration along with all of its
// defaults, etc.
//
// TODO: Differentiate between flags that are enabled to determine
// a mode, flags that can be enabled or disabled (and have some default
// state), and options that take a typed value.
struct Parameter
{
  Parameter(Parameter_kind, char const*);
  Parameter(Parameter_kind, char const*, char const*);

  Parameter_kind kind;
  char           abbr; // A short form
  std::string    name; // The parameter name
  std::string    doc;  // Documentation
};


// The parameter list defines a sequence of parameters that can
// be parsed from an argument list.
using Parameter_list = std::vector<Parameter>;


// -------------------------------------------------------------------------- //
//                           Parsed arguments

// The parsed arguments class stores the result of parsing.
// Note that this class is non-copyable.
struct Parsed_arguments : std::pair<json::Object*, json::Array*>
{
  Parsed_arguments();
  Parsed_arguments(Parsed_arguments const&) = delete;
  Parsed_arguments& operator=(Parsed_arguments const&) = delete;
  ~Parsed_arguments();

  json::Object&       named_arguments()            { return *first; }
  json::Object const& named_arguments() const      { return *first; }

  json::Array&        positional_arguments()       { return *second; }
  json::Array const&  positional_arguments() const { return *second; }

  json::Value const* operator[](char const*) const;
  json::Value const* operator[](std::size_t n) const;
};


inline
Parsed_arguments::Parsed_arguments()
  : std::pair<json::Object*, json::Array*>(json::make_object(), json::make_array())
{ }

inline
Parsed_arguments::~Parsed_arguments()
{
  json::destroy(first);
  json::destroy(second);
}



// Returns a pointer to the parsed argument with the given
// name, or nullptr if no such argument was parsed.
inline json::Value const*
Parsed_arguments::operator[](char const* name) const
{
  // return named_arguments()[name];
  return nullptr;
}


// Returns a pointer to the nth positional argument, or
// nullptr if there is no such argument.
inline json::Value const*
Parsed_arguments::operator[](std::size_t n) const
{
  if (n < positional_arguments().size())
    return positional_arguments()[n];
  else
    return nullptr;
}


// -------------------------------------------------------------------------- //
//                              Parser


// The command line parser, given a sequence parameters, will
// return a JSON object containing the parsed values.
struct Parser
{
  using Name_lookup = std::unordered_map<std::string, Parameter*>;
  using Char_lookup = std::unordered_map<char, Parameter*>;

  Parser(Parameter_list& p);

  Parsed_arguments const& operator()(int, char**);

  Parameter_list& parms;   // Parameter specs
  Name_lookup     names;   // Names to parameters
  Char_lookup     chars;   // Abbreviated names to parameters
  Parsed_arguments result; // Parse results.
};


} // namespace cli

} // namespace lingo

#endif
