// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#ifndef LINGO_PARSING_HPP
#define LINGO_PARSING_HPP

#include "lingo/token.hpp"
#include "lingo/algorithm.hpp"
#include "lingo/error.hpp"

#include <iostream>

namespace lingo
{

// -------------------------------------------------------------------------- //
//                            Token classifiers

// A function object used to compare tokens.
struct is_token_fn
{
  bool operator()(Token const& tok) const
  {
    return tok.kind() == k;
  }

  Token_kind k;
};


// A function object used to compare tokens.
struct is_not_token_fn
{
  bool operator()(Token const& tok) const
  {
    return tok.kind() != k;
  }

  Token_kind k;
};


// Returns a function that evalutes if a token has the given kind.
inline is_token_fn
is_token(Token_kind k)
{
  return {k};
}


// Returns a function that evaluates if a token does not have 
// the given kind.
inline is_not_token_fn
is_not_token(Token_kind k)
{
  return {k};
}


// Returns the kind of the next token in `s`.
template<typename Stream>
inline Token_kind
next_token_kind(Stream& s)
{
  if (s.eof())
    return {};
  else
    return s.peek().kind();
}


// Returns true if the next token has kind k.
template<typename Stream>
inline bool
next_token_is(Stream& s, Token_kind k)
{
  return next_token_kind(s) == k;
}


// Returns true if the next token does not have kind k.
template<typename Stream>
inline bool
next_token_is_not(Stream& s, Token_kind k)
{
  return next_token_kind(s) != k;
}


// Returns true if the kind of the next token in in the
// numeric range [first, last], inclusive.
template<typename Stream>
inline bool
next_token_in_range(Stream& s, Token_kind first, Token_kind last)
{
  return first <= next_token_kind(s) && next_token_kind(s) <= last;
}


// -------------------------------------------------------------------------- //
//                            Token matching


// Return a pointer to the next token if it has kind 'k'. Otherwise,
// returns a null pointer.
template<typename Stream>
inline Iterator_type<Stream>
match_token(Stream& s, Token_kind k)
{
  return match_if(s, is_token(k));
}


// Return a pointer to the next token if it has kind 'k'. Otherwise,
// returns a null pointer.
template<typename Parser, typename Stream>
inline Iterator_type<Stream>
expect_token(Parser& p, Stream& s, Token_kind k)
{
  return expect_if(p, s, is_token(k), get_token_spelling(k));
}


// -------------------------------------------------------------------------- //
//                           Parser combinators

template<typename Parser, typename Stream, typename Rule>
Result_type<Parser>
parse_expected(Parser& p, Stream& s, Rule rule, char const* msg)
{
  if (auto result = rule(p, s))
    return result;
  else {
    return p.on_expected(s.location(), msg);
  }
  return {};
}


// Parse a grammar production enclosed by a pair of tokens.
//
//    enclosed-term ::= k1 [rule] k2
//
// Here, `k1` and `k2` are token kinds and `rule` is the 
// enclose grammar production. Note that an empty enclosure
// is allowed.
//
// The parser must define the following operations:
//
//    p.on_enclosure(loc, loc)
//    p.on_enclosure(loc, loc, term)
//
// The first is invoked when the parsing an empty enclosure, and
// the second when the inner term is parsed.
template<typename Parser, typename Stream, typename Grammar>
inline Result_type<Parser>
parse_enclosed(Parser& p, Stream& s, Token_kind k1, Token_kind k2, Grammar rule, char const* msg)
{
  if (auto left = expect_token(p, s, k1)) {
    // Match the empty enclosure.
    if (auto right = match_token(s, k2))
      return p.on_enclosure(left, right);
    
    // Check the rule.
    if (auto mid = rule(p, s)) {
      if (auto right = expect_token(p, s, k2))
        return p.on_enclosure(left, right, mid);
      else
        return p.on_error();
    } else {
      return p.on_expected(s.location(), msg);
    }
  }
  return {};
}


// Parse a parentheses-enclosed production.
//
//    paren-enclosed ::= '(' [rule] ')'
template<typename Parser, typename Stream, typename Grammar>
inline Result_type<Parser>
parse_paren_enclosed(Parser& p, Stream& s, Grammar rule, char const* msg)
{
  return parse_enclosed(p, s, lparen_tok, rparen_tok, rule, msg);
}


// Parse a brace-enclosed production.
//
//    brace-enclosed ::= '{' rule '}'
template<typename Parser, typename Stream, typename Grammar>
inline Result_type<Parser>
parse_brace_enclosed(Parser& p, Stream& s, Grammar rule, char const* msg)
{
  return parse_enclosed(p, s, lbrace_tok, rbrace_tok, rule, msg);
}


// Parse a bracket-enclosed production.
//
//    bracket-enclosed ::= '[' rule ']'
template<typename Parser, typename Stream, typename Grammar>
inline Result_type<Parser>
parse_bracket_enclosed(Parser& p, Stream& s, Grammar rule, char const* msg)
{
  return parse_enclosed(p, s, lbrack_tok, rbrack_tok, rule, msg);
}


// Parse a prefix term. A prefix term (often called a unary term)
// has the following form:
//
//    prefix-term ::= rule | token prefix-term
//
// Here, `token` is a matching function that accepts the set of
// prefix operators. `rule` is a parsing function that matches
// the next higher precedence in the grammar.
//
// For any grammar that uses this production, the parser must
// define a handler, `p.on_unary_term(op, term)`.
//
// To support disagnostics, `msg` is the name of the grammar
// production that invokes this parsing function.
template<typename Parser, typename Stream, typename Token, typename Rule>
Result_type<Parser>
parse_prefix_term(Parser& p, Stream& s, Token token, Rule rule, char const* msg)
{
  if (auto* op = token(p, s)) {
    if (auto* term = parse_prefix_term(p, s, token, rule, msg)) {
      return p.on_unary_term(op, term);
    } else {
      return p.on_expected(s.location(), msg);
    }
  }
  return rule(p, s);
}


// Parse a left associative binary expression. This has the form:
//
//    left-binary-term ::=
//        rule
//      | left-binary-term token rule
//
// Here, `token` is a matching function that accepts the set of
// binary operators for the grammar. `rule` is a parsing function
// matches the next higher precedence in the grammar.
//
// Any grammar that invokes this parsing function must define the
// handler, `p.on_binary_term(tok, expr1, expr2)` where `tok` is
// the binary operator, `expr1` is the left-hand side, and `expr2` 
// is the right-hand side.
//
// To support diagnostics, `msg` is the name of the grammar
// production associated with `rule`.
template<typename Parser, typename Stream, typename Op, typename Rule>
Result_type<Parser>
parse_left_binary_term(Parser& p, Stream& s, Op op, Rule rule, char const* msg)
{
  if (Result_type<Parser> expr1 = rule(p, s)) {
    while (Token const* tok = op(p, s)) {
      if (Result_type<Parser> expr2 = rule(p, s)) 
        expr1 = p.on_binary_term(tok, expr1, expr2);
      else 
        return p.on_expected(tok->location(), msg, *tok);
    }
    return expr1;
  }
  return {};
}


#if 0
// Parse a right associative binary expression. There are two
// arguments: a token recognizer and the immediate sub-production.
//
// TODO: Make this iterative?
template<typename Parser, typename T, typename P>
Parse
right_binary(Parser& p, T token, P production)
{
  // A helper function for recursing.
  auto recurse = [&](Parser& p) -> Parse { 
    return right_binary(p, token, production);
  };

  if (Parse expr1 = production(p)) {
    if (Token const* tok = token(p)) {
      if (Parse expr2 = recurse(p)) {
        expr1 = on_binary_expression(*tok, expr1, expr2);
      } else {
        error(p, "expected '{}' after '{}'", production_name(production), *tok);
        return {};
      }
    }
    return expr1;
  }
  return {};
}
#endif


// Parse a sequence of terms with no intervening tokens.
//
//    sequence(rule) ::= <empty> | rule [sequence(rule)]
//
// To use this function, the parser must define:
//
//    p.on_sequence(v)
//
// where v is a vector of parsed elements. Note that the result of 
// the sub-rule 
template<typename Parser, typename Stream, typename Grammar>
Result_type<Parser>
parse_sequence(Parser& p, Token_stream& ts, Grammar rule)
{
  std::vector<Result_type<Parser>> seq;
  while (!ts.eof()) {
    Result_type<Parser> elem = rule(p, ts);
    if (!elem || is_error(elem))
      return elem;
    seq.push_back(elem);
  }
  return p.on_sequence(std::move(seq));
}


// Parse a non-empty comma-separated list of terms.
//
//    list(rule) ::= rule [',' rule]*
//
// TODO: Consider allowing a list to include an extra trailing comma. 
// This would only be valid in certain contexts (e.g., enumerations, 
// aggregate initializers).
template<typename Parser, typename Stream, typename Grammar>
Result_type<Parser>
parse_list(Parser& p, Stream& ts, Grammar rule)
{
  std::vector<Result_type<Parser>> list;
  while (!ts.eof()) {
    Result_type<Parser> elem = rule(p, ts);
    if (!elem || is_error(elem))
      return elem;
    list.push_back(elem);

    if (!expect_token(p, ts))
      return p.on_error();
    consume(p);
  }
  return p.on_list(std::move(list));
}

} // namespace lingo

#endif
