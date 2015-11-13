// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "lingo/parsing.hpp"

#include <unordered_map>

namespace lingo
{

namespace
{

std::unordered_map<void(*)(), char const*> rules_;

} // namespace


// Install the name of a grammar, mapping `rule` to `name`. Note
// that `name` must be a C-string literal.
//
// TODO: Should we strdup `name`?
void
install_grammar(void(*rule)(), char const* name)
{
  lingo_alert(rules_.count(rule) == 0, "existing grammar named '{}'", name);
  rules_.insert({rule, name});
}


// Return a name associated with the rule. If the rule does not
// have an associated name, return a placeholder string.
char const*
get_grammar_name(void(*rule)())
{
  auto iter = rules_.find(rule);
  if (iter == rules_.end())
    return "<unspecified-grammar>";
  else
    return iter->second;
}


} // namespace lingo
