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
//                              Token stream


// A token stream provides a sequence of tokens and has a very 
// simple streaming interface consisting of only 5 functions:
// peek(), get(), and eof(), begin(), and end(). Character streams
// are the input to lexical analyzers.
struct Token_stream
{
  using value_type = Token;

  Token_stream(Token const* f, Token const* l)
    : first(f), last(l)
  { }

  Token_stream(Token_list const& toks)
    : Token_stream(toks.data(), toks.data() + toks.size())
  { }

  // Stream control
  bool eof() const { return first == last; }
  Token const& peek() const;
  Token        peek(int) const;
  Token const& get();

  // Iterators
  Token const* begin()       { return first; }
  Token const* begin() const { return first; }
  Token const* end()       { return last; }
  Token const* end() const { return last; }

  // Returns the source location of the the current token.
  Location location() { return eof() ? Location{} : peek().location(); }

  // Returns the empty value.
  static Token null() { return {}; }

  Token const* first; // Current character pointer
  Token const* last;  // Past the end of the character buffer
};


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
//                           Parser combinators

template<typename Parser, typename Stream, typename Rule>
Result_type<Parser>
parse_expected(Parser& p, Stream& s, Rule rule, char const* msg)
{
  if (auto result = rule(p, s))
    return result;
  else {
    error(s.location(), "expected {}", msg);
    return p.on_error();
  }
  return {};
}


// Parse a grammar production enclosed by a pair of tokens.
//
//    enclosed-term ::=
//      k1 rule k2
//
// Here, `k1` and `k2` are token kinds and `rule` is the 
// enclose grammar production.
template<typename Parser, typename Stream, typename Rule>
inline Result_type<Parser>
parse_enclosed(Parser& p, Stream& s, Token_kind k1, Token_kind k2, Rule rule, char const* msg)
{
  if (auto* left = expect_if(s, is_token(k1), get_token_spelling(k1))) {
    if (auto* enc = rule(p, s)) {
      if (expect_if(s, is_token(k2), get_token_spelling(k2)))
        return enc;
    } else {
      error(left->location(), "expected {} after '{}'", msg, *left);
      return p.on_error();
    }
  }
  return {};
}


// Parse a parentheses-enclosed production.
//
//    paren-enclosed-term ::=
//      '(' rule ')'
template<typename Parser, typename Stream, typename Rule>
inline Result_type<Parser>
parse_paren_enclosed(Parser& p, Stream& s, Rule rule, char const* msg)
{
  return parse_enclosed(p, s, lparen_tok, rparen_tok, rule, msg);
}


// Parse a brace-enclosed production.
//
//    brace-enclosed-term ::=
//      '{' rule '}'
template<typename Parser, typename Stream, typename Rule>
inline Result_type<Parser>
parse_brace_enclosed(Parser& p, Stream& s, Rule rule, char const* msg)
{
  return parse_enclosed(p, s, lbrace_tok, rbrace_tok, rule, msg);
}


// Parse a bracket-enclosed production.
//
//    bracket-enclosed-term ::=
//      '[' rule ']'
template<typename Parser, typename Stream, typename Rule>
inline Result_type<Parser>
parse_bracket_enclosed(Parser& p, Stream& s, Rule rule, char const* msg)
{
  return parse_enclosed(p, s, lbrack_tok, rbrack_tok, rule, msg);
}


// Parse a prefix term. A prefix term (often called a unary term)
// has the following form:
//
//    prefix-term ::=
//        rule
//      | token prefix-term
//
// Here, `token` is a matching function that accepts the set of
// prefix operators. `rule` is a parsing function that matches
// the next higher precedence in the grammar.
//
// For any grammar that uses this production, the parser must
// define a handler, `p.on_unary_term(tok, expr)`.
//
// To support disagnostics, `msg` is the name of the grammar
// production that invokes this parsing function.
template<typename Parser, typename Stream, typename Token, typename Rule>
Result_type<Parser>
parse_prefix_term(Parser& p, Stream& s, Token token, Rule rule, char const* msg)
{
  if (auto* tok = token(p, s)) {
    if (auto* expr = parse_prefix_term(p, s, token, rule, msg))
      return p.on_unary_term(tok, expr);
    else {
      error(tok->location(), "expected {} after '{}'", msg, *tok);
      return p.on_error();
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
template<typename Parser, typename Stream, typename Token, typename Rule>
Result_type<Parser>
parse_left_binary_term(Parser& p, Stream& s, Token token, Rule rule, char const* msg)
{
  if (auto* expr1 = rule(p, s)) {
    while (auto* tok = token(p, s)) {
      if (auto* expr2 = rule(p, s)) {
        expr1 = p.on_binary_term(tok, expr1, expr2);
      } else {
        error(tok->location(), "expected {} after '{}'", msg, *tok);
        return p.on_error();
      }
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


// Parse a sequence of terms with no intervening tokens. The
// action invoked for each term is to append each parsed term
// to a list. Note that the sequence may be empty. Only if
// an error occured do we return an empty parse.
template<typename Parser, typename P>
Parse
sequence(Parser& p, P production)
{
  int errs = error_count();
  Parse seq = list_init();
  while (Parse decl = production(p))
    seq = list_append(seq, decl);
  
  // Only return the sequence if we parsed it without errors.
  if (error_count() == errs)
    return seq;
  else
    return release(seq);
}


// Parse a comma-separated list of terms.
//
// TODO: Consider allowing a list to include an extra
// trailing comma. This would only be valid in certain
// contexts (e.g., enumerations, aggregate initializers).
template<typename Parser, typename P>
Parse
list(Parser& p, P production)
{
  int errs = error_count();
  Parse list = list_init();
  while (Parse arg = production(p)) {
    list = list_append(list, arg);
    if (next_token_is_not(p, comma_tok))
      break;
    consume(p);
  }

  // Only return the list if we parsed it without errros.
  if (error_count() == errs)
    return list;
  else
    return release(list);
}
#endif

} // namespace lingo

#endif
