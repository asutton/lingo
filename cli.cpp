// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "lingo/cli.hpp"
#include "lingo/error.hpp"

#include <cstring>
#include <algorithm>
#include <iostream>

namespace lingo
{

namespace cli
{

namespace
{

// -------------------------------------------------------------------------- //
//                              Parameters

// Retrieve the paramter name and its abbreviation.
void
parse_name_spec(Parameter& p, char const* first, char const* last)
{
  char const* iter = std::find(first, last, ',');
  p.name = std::string(first, iter);
  if (iter != last)
    p.abbr = *(iter + 1);
  else
    p.abbr = 0;
}

} // namespace


Parameter::Parameter(Parameter_kind k, char const* name)
  : kind(k), doc()
{
  parse_name_spec(*this, name, name + std::strlen(name));
}


Parameter::Parameter(Parameter_kind k, char const* name, char const* doc)
  : kind(k), doc(doc)
{
  parse_name_spec(*this, name, name + std::strlen(name));
}


// -------------------------------------------------------------------------- //
//                              Parser


namespace
{


// Parse the value associated with value parameter.
//
// TODO: Improve diagnotics.
json::Value*
parse_value(Parameter const& parm, int& argc, char** argv, char const* str)
{
  if (!*str)            // The case "-f value"
    str = argv[++argc];
  else if (*str == '=') // The case "-f=value"
    ++str;
  else
    throw std::runtime_error(format("expected value assignment for '{}'", parm.name));

  // Try to parse the string as JSON. If it's not valid
  // JSON, then it's clearly a string that could be valid
  // if it were enclosed in quotes.
  //
  // Note that we can't just modify the JSON parser to
  // accommodate a broader range values since we can
  // take literally any character sequence on the command
  // line, and those may not be compatible the JSON syntax.
  //
  // TODO: Allow for custom parsers for JSON values.
  try {
    return json::parse(str);
  } catch (...) {
    return json::make_string(str);
  }
}


// Parse a long argument, which is known to start with the
// character sequence '--'. Long-form characters have the
// form:
//
//    --flag      # Flags only
//    --opt value # Values only
//    --opt=value # Values only
void
parse_long_arg(Parser& p, int& argc, char** argv)
{
  json::Object& named = p.result.named_arguments();

  char const* first = &argv[argc][2];
  char const* last = first + 1;
  while (*last != 0 && *last != '=')
    ++last;

  // Find the parameter.  
  // TODO: Can we avoid allocating memory here?
  std::string name(first, last);
  auto iter = p.names.find(first);
  if (iter == p.names.end())
    throw std::runtime_error(format("no matching parameter for '{}'", first));
  Parameter* parm = iter->second;

  // Parse the value.
  if (parm->kind == flag)
    named[parm->name] = json::make_true();
  else
    named[parm->name] = parse_value(*parm, argc, argv, last);

  ++argc;
}


// Parse a short argument or a sequence of short arguments.
// The parse depends on the kind of parameter. For flags,
// we accept the following:
//
//    -a    # Enable a
//    -abc  # Enable a, b, and c
//
// These associate the value 'null', indicating presence.
//
// For valued parameters, we accept:
//
//    -f=value
//    -f value
//
// Note that we also accept the following:
//
//    -abf=value
//    -abf value
//
// When a and b are flags and f valued parameter.
void
parse_short_arg(Parser& p, int& argc, char** argv)
{
  json::Object& named = p.result.named_arguments();

  char const* s = &argv[argc][1];
  while (*s) {
    auto iter = p.chars.find(*s);
    if (iter == p.chars.end())
      throw std::runtime_error(format("no matching parameter for '{}'", *s));
    Parameter* parm = iter->second;

    // Assign a value to the parameter.
    ++s;
    if (parm->kind == flag) {
      named[parm->name] = json::make_true();
    } else {
      named[parm->name] = parse_value(*parm, argc, argv, s);
      break;
    }
  }
  ++argc;
}


void
parse_named_arg(Parser& p, int& argc, char** argv)
{
  if (argv[argc][1] == '-')
    return parse_long_arg(p, argc, argv);
  else
    return parse_short_arg(p, argc, argv);
}


void
parse_positional_arg(Parser& p, int& argc, char** argv)
{
  json::Array& args = p.result.positional_arguments();
  json::String* arg = json::make_string(argv[argc]);
  args.push_back(arg);
  ++argc;
}

} // naemspace


Parser::Parser(Parameter_list& p)
  : parms(p), result()
{
  // Build the lookup maps.
  for (Parameter& p : parms) {
    names[p.name] = &p;
    if (p.abbr)
      chars[p.abbr] = &p;
  }
}


Parsed_arguments const&
Parser::operator()(int argc, char** argv)
{
  for (int i = 0; i < argc; ) {
    if (argv[i][0] == '-')
      parse_named_arg(*this, i, argv);
    else
      parse_positional_arg(*this, i, argv);
  }
  return result;
}

} // namespace cli

} // namespace lingo
