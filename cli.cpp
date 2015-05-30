// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "lingo/cli.hpp"
#include "lingo/error.hpp"

#include <cstring>
#include <algorithm>

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


Parameter::Parameter(char const* name)
{
  parse_name_spec(*this, name, name + std::strlen(name));
}


// -------------------------------------------------------------------------- //
//                              Parser


namespace
{

void
parse_long_arg(Parser& p, int& argc, char const* first, char const* last)
{
  ++argc;
}


void
parse_short_arg(Parser& p, int& argc, char const* first, char const* last)
{
  json::Object& named = p.result.named_arguments();

  while (first != last) {
    // Try to find the parameter.
    auto iter = p.chars.find(*first);
    if (iter == p.chars.end())
      error(Location::cli, "no matching parameter for '{}'", *first);
    Parameter* parm = iter->second;

    // Look at the next character. If it's an equals, then we
    // need to parse the value.
    ++first;
    if (first != last && *first == '=') {
      named[parm->name] = json::parse(first + 1, last);
      first = last;
    } else {
      named[parm->name] = json::make_true();
    }
  }
  ++argc;
}


void
parse_named_arg(Parser& p, int& argc, char** argv)
{
  char const* first = &argv[argc][1];
  char const* last = first + std::strlen(argv[argc]);
  if (*first == '-')
    return parse_long_arg(p, argc, ++first, last);
  else
    return parse_short_arg(p, argc, first, last);
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
  for (int i = 0; i < argc; ++i) {
    if (argv[i][0] == '-')
      parse_named_arg(*this, argc, argv);
    else
      parse_positional_arg(*this, argc, argv);
  }

  return result;
}

} // namespace cli

} // namespace lingo
