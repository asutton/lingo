// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#ifndef LINGO_PARSING_HPP
#define LINGO_PARSING_HPP

#include "lingo/token.hpp"
#include "lingo/algorithm.hpp"
#include "lingo/error.hpp"

namespace lingo
{

// -------------------------------------------------------------------------- //
//                         Grammar production names
//
// These functions provide support for associating the names of
// grammatical productions with the functions that implement them.



void install_grammar(void(*)(), char const*);
char const* get_grammar_name(void*);


// Install a grammar name for the given function.
template<typename T, typename... Args>
inline void 
install_grammar(T(*fn)(Args...), char const* name)
{
  return install_grammar(reinterpret_cast<void(*)()>(fn), name);
}


// Lookup a name for the given rule.
template<typename T, typename... Args>
inline char const* 
get_grammar_name(T(*fn)(Args...))
{
  return get_grammar_name(reinterpret_cast<void(*)()>(fn));
}


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


// Just return the next token, or nullptr if past the end.
template<typename Stream>
inline Iterator_type<Stream>
get_token(Stream& s)
{
  if (s.eof())
    return nullptr;
  else
    return &s.get();
}


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


template<typename Stream>
inline Iterator_type<Stream>
require_token(Stream& s, Token_kind k)
{
  lingo_alert(next_token_is(s, k), "required token '{}'", get_token_spelling(k));
  return match_if(s, is_token(k));
}


// -------------------------------------------------------------------------- //
//                           Parser combinators

// Strip information from the return type of a rule.
template<typename T>
using get_term_type = 
  typename std::remove_const<
    typename std::remove_pointer<T>::type
  >::type;


template<typename Parser, typename Stream, typename Rule>
inline auto
term_type(Parser& p, Stream& s, Rule rule)
  -> get_term_type<decltype(rule(p, s))>;


template<typename Parser, typename Stream, typename Rule>
inline auto
parse_expected(Parser& p, Stream& s, Rule rule)
  -> decltype(term_type(p, s, rule))
{
  if (auto result = rule(p, s))
    return result;
  else {
    return p.on_expected(s.location(), get_grammar_name(rule));
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
// FIXME: Pass token data back to the caller. A simple struct
// as an argument should suffice.
template<typename Parser, typename Stream, typename Grammar>
inline auto
parse_enclosed(Parser& p, Stream& s, Token_kind k1, Token_kind k2, Grammar rule)
  -> decltype(rule(p, s))
{
  using Term = decltype(term_type(p, s, rule));
  
  if (auto left = require_token(s, k1)) {
    
    // Match the empty enclosure.
    if (auto right = match_token(s, k2))
      return Term::empty();
    
    // Check the rule. Note to be careful about copying the
    // parsed term. Note that no allocations occur when returning
    // an error.
    if (Required<Term> term = rule(p, s)) {

      // Pick up the closing brace.
      if (auto right = expect_token(p, s, k2))  {
        return *term;
      } else {
        // TODO: Better error message?
        error("expected '{}'", k2);
        return Term::error(); // Unabalanced markers.
      }
    } else {
      return Term::error(); // Error parsing term.
    }
  }
  lingo_unreachable();
}


// Parse a parentheses-enclosed production.
//
//    paren-enclosed ::= '(' [rule] ')'
template<typename Parser, typename Stream, typename Grammar>
inline auto
parse_paren_enclosed(Parser& p, Stream& s, Grammar rule)
  -> decltype(rule(p, s))
{
  return parse_enclosed(p, s, lparen_tok, rparen_tok, rule);
}


// Parse a brace-enclosed production.
//
//    brace-enclosed ::= '{' rule '}'
template<typename Parser, typename Stream, typename Grammar>
inline auto
parse_brace_enclosed(Parser& p, Stream& s, Grammar rule)
  -> decltype(rule(p, s))
{
  return parse_enclosed(p, s, lbrace_tok, rbrace_tok, rule);
}


// Parse a bracket-enclosed production.
//
//    bracket-enclosed ::= '[' rule ']'
template<typename Parser, typename Stream, typename Grammar>
inline auto
parse_bracket_enclosed(Parser& p, Stream& s, Grammar rule)
  -> decltype(rule(p, s))
{
  return parse_enclosed(p, s, lbrack_tok, rbrack_tok, rule);
}


// Parse a prefix term. A prefix term (or unary, in some grammars) 
// has the following form:
//
//    prefix-term ::= rule | token prefix-term
//
// Here, `token` is a matching function that accepts the set of
// prefix operators. `rule` is a parsing function that matches
// the next higher precedence in the grammar.
//
// For any grammar that uses this production, the parser must
// define a handler, `p.on_prefix_term(op, term)`.
template<typename Parser, typename Stream, typename Op, typename Rule>
auto
parse_prefix_term(Parser& p, Stream& s, Op op, Rule rule)
  -> decltype(rule(p, s))
{
  using Term = decltype(term_type(p, s, rule));
  if (Token const* tok = op(p, s)) {
    if (Required<Term> term = parse_prefix_term(p, s, op, rule)) {
      return *term;
    } else {
      error(s.location(), "expected {}", get_grammar_name(rule));
      return Term::error();
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
auto
parse_left_infix_expression(Parser& p, Stream& s, Op op, Rule rule)
  -> decltype(rule(p, s))
{
  if (auto expr1 = rule(p, s)) {
    while (Token const* tok = op(p, s)) {
      if (auto expr2 = rule(p, s)) 
        expr1 = p.on_infix_expression(tok, expr1, expr2);
      else 
        return p.on_expected(tok->location(), get_grammar_name(rule), *tok);
    }
    return expr1;
  }
  return {};
}


// Parse a right associative binary expression. There are two
// arguments: a token recognizer and the immediate sub-production.
//
// TODO: Make this iterative?
template<typename Parser, typename Stream, typename Op, typename Grammar>
auto
parse_right_infix_expression(Parser& p, Stream& s, Op op, Grammar rule)
  -> decltype(rule(p, s))
{
  if (Result_type<Parser> expr1 = rule(p, s)) {
    if (Token const* tok = op(p, s)) {
      if (Result_type<Parser> expr2 = parse_right_infix_expression(p, s, op, rule))
        expr1 = p.on_infix_expression(tok, expr1, expr2);
      else
        return p.on_expected(tok->location(), get_grammar_name(rule), *tok);
    }
    return expr1;
  }
  return {};
}


// Parse a sequence of terms with no intervening tokens.
//
//    sequence(rule) ::= <empty> | rule [sequence(rule)]
//
// To use this function, the parser must define:
//
//    p.on_sequence(v)
//
// where v is a vector of parsed elements. 
template<typename Parser, typename Stream, typename Grammar>
auto
parse_sequence(Parser& p, Stream& s, Grammar rule)
  -> decltype(rule(p, s))
{
  std::vector<Result_type<Parser>> seq;
  while (!s.eof()) {
    Result_type<Parser> elem = rule(p, s);
    if (!elem || p.is_error(elem))
      return elem;
    seq.push_back(elem);
  }
  return p.on_sequence(std::move(seq));
}


// Parse a non-empty comma-separated list of terms.
//
//    list(rule) ::= rule [',' rule]*
//
// This takes the sequence being constructed as an argument
// and returns that value. If an error is encountered during
// the parse, the input sequence will be overrwritten by
// an error code.
//
// The list type must be a Seq<T const*> where T const* is
// the return type of the grammar.
template<typename Parser, typename Stream, typename Grammar>
auto
parse_list(Parser& p, Stream& s, Grammar rule)
  -> Sequence<decltype(term_type(p, s, rule))>
{
  using Term = decltype(term_type(p, s, rule));
  using List = Sequence<Term>;
  List list;
  do {
    if (Required<Term> term = rule(p, s))
      list.push_back(*term);
    else
      return List::error();
  } while (match_token(s, comma_tok));
  return list;
}


} // namespace lingo


#endif
