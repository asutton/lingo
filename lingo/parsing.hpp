// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#ifndef LINGO_PARSING_HPP
#define LINGO_PARSING_HPP

#include <lingo/token.hpp>
#include <lingo/memory.hpp>
#include <lingo/algorithm.hpp>
#include <lingo/error.hpp>

namespace lingo
{

// -------------------------------------------------------------------------- //
//                         Grammar production names
//
// These functions provide support for associating the names of
// grammatical productions with the functions that implement them.



void install_grammar(void(*)(), char const*);
char const* get_grammar_name(void(*)());


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

  int k;
};


// A function object used to compare tokens.
struct is_not_token_fn
{
  bool operator()(Token const& tok) const
  {
    return tok.kind() != k;
  }

  int k;
};


// Returns a function that evalutes if a token has the given kind.
inline is_token_fn
is_token(int k)
{
  return {k};
}


// Returns a function that evaluates if a token does not have
// the given kind.
inline is_not_token_fn
is_not_token(int k)
{
  return {k};
}


// Returns the kind of the next token in `s`.
template<typename Stream>
inline int
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
next_token_is(Stream& s, int k)
{
  return next_token_kind(s) == k;
}


// Returns true if the next token does not have kind k.
template<typename Stream>
inline bool
next_token_is_not(Stream& s, int k)
{
  return next_token_kind(s) != k;
}


// Returns true if the kind of the next token in in the
// numeric range [first, last], inclusive.
template<typename Stream>
inline bool
next_token_in_range(Stream& s, int first, int last)
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
match_token(Stream& s, int k)
{
  return match_if(s, is_token(k));
}


// Return a pointer to the next token if it has kind 'k'. Otherwise,
// returns a null pointer.
template<typename Parser, typename Stream>
inline Iterator_type<Stream>
expect_token(Parser& p, Stream& s, int k)
{
  return expect_if(p, s, is_token(k), get_token_spelling(k));
}


template<typename Stream>
inline Iterator_type<Stream>
require_token(Stream& s, int k)
{
  lingo_alert(next_token_is(s, k), "required token '{}'", get_token_spelling(k));
  return match_if(s, is_token(k));
}


// -------------------------------------------------------------------------- //
//                           Parser types

// Strip information from the return type of a rule.
template<typename T>
using get_term_type =
  typename std::remove_const<
    typename std::remove_pointer<T>::type
  >::type;


// The term type is the core term returned by a grammatical
// rule. For example, if the rule is the function:
//
//    Node const* parse_foo(Parsre&, Stream&);
//
// Then the term type of the rule is `Node`.
template<typename Parser, typename Stream, typename Rule>
using Term_type =
  get_term_type<
    decltype(std::declval<Rule>()(std::declval<Parser&>(),
                                  std::declval<Stream&>()))
  >;


// -------------------------------------------------------------------------- //
//                           Expected term


// Parse a term, guaranteeing that this emits a diagnostics if
// the term cannot be matched.
template<typename Parser,
         typename Stream,
         typename Rule,
         typename Term = Term_type<Parser, Stream, Rule>>
inline Term const*
parse_expected(Parser& p, Stream& s, Rule rule)
{
  Location loc = s.location();
  if (Required<Term> result = rule(p, s))
    return *result;
  else {
    if (result.is_empty())
      error(loc, "expected {}", get_grammar_name(rule));
    return *result;
  }
  lingo_unreachable();
}



// -------------------------------------------------------------------------- //
//                           Enclosure parsing


// An enclosed term represents a sub-term bracketed by a pair
// of tokens. This class includes the locations of those tokens.
//
// Note that the enclosure may be empty.
template<typename T>
struct Enclosed_term
{
  Enclosed_term(Token const* o, Token const* c)
    : open_(o), close_(c), first(nullptr)
  { }

  Enclosed_term(Token const* o, Token const* c, T const* t)
    : open_(o), close_(c), first(t)
  { }

  Token const* open() const  { return open_; }
  Token const* close() const { return close_; }
  T const* term() const      { return first; }

  bool is_empty() const { return first != nullptr; }

  // Factory interface
  static Enclosed_term* make(Token const*, Token const*);
  static Enclosed_term* make(Token const*, Token const*, T const*);

  Token const* open_;
  Token const* close_;
  T const* first;
};


template<typename T>
inline Enclosed_term<T>*
Enclosed_term<T>::make(Token const* o, Token const* c)
{
  return new Enclosed_term(o, c);
}


template<typename T>
inline Enclosed_term<T>*
Enclosed_term<T>::make(Token const* o, Token const* c, T const* t)
{
  return new Enclosed_term(o, c, t);
}


// Parse a grammar production enclosed by a pair of tokens.
//
//    enclosed-term ::= k1 [rule] k2
//
// Here, `k1` and `k2` are token kinds and `rule` is the
// enclose grammar production. Note that an empty enclosure
// is allowed.
template<typename Parser,
         typename Stream,
         typename Rule,
         typename Term = Term_type<Parser, Stream, Rule>>
Enclosed_term<Term> const*
parse_enclosed(Parser& p, Stream& s, int k1, int k2, Rule rule)
{
  using Result = Enclosed_term<Term>;

  auto const* left = require_token(s, k1);

  // Match the empty enclosure.
  if (auto const* right = match_token(s, k2))
    return Result::make(left, right);

  // Check the rule. Note to be careful about copying the
  // parsed term. Note that no allocations occur when returning
  // an error. Note that we've covered the emtpy case above,
  // so this term is required.
  if (Required<Term> term = rule(p, s)) {
    if (auto const* right = expect_token(p, s, k2)) {
      return Result::make(left, right, *term);
    } else {
      // Unbalanced brace.
      //
      // TODO: It would be nice to point at the end of the
      // enclosed term, but that requires more extensive
      // range support (which we should probably provide).
      //
      // TODO: Show the position of the starting bracket
      // to improve diagnostics?
      error(left->location(), "expected '{}' after {}",
            get_token_spelling(k2),
            get_grammar_name(rule));
    }
  } else {
    // Failed to parse the enclosed term.
    if (term.is_empty())
      error(left->location(), "expected {} after '{}'",
            get_grammar_name(rule),
            get_token_spelling(k1));
  }
  return make_error_node<Result>();
}


// -------------------------------------------------------------------------- //
//                           Sequence parsers


// A sequence term is a possibly empty list of subterms.
//
// TODO: Should we also track the location of interleaving
// tokens for the case where the sequence is a list?
//
// TODO: The factories leak memory.
template<typename T>
struct Sequence_term : std::vector<T const*>
{
  using std::vector<T const*>::vector;

  char const* node_name() const { return "sequence"; };

  Location location() const { return this->front()->location(); }

  // Factories
  static Sequence_term* make();
  static Sequence_term* make(Sequence_term&&);
  static Sequence_term* make(std::initializer_list<T const*>);
};


// Create a new empty sequence.
template<typename T>
inline Sequence_term<T>*
Sequence_term<T>::make()
{
  return new Sequence_term();
}


// Create a new sequence with the contents of `seq`.
template<typename T>
inline Sequence_term<T>*
Sequence_term<T>::make(Sequence_term&& seq)
{
  return new Sequence_term(std::move(seq));
}


// Create a new sequence whose contents are given in `list`.
template<typename T>
inline Sequence_term<T>*
Sequence_term<T>::make(std::initializer_list<T const*> list)
{
  return new Sequence_term(list);
}


// Parse a sequence of terms with no intervening tokens.
//
//    sequence ::= <empty> | rule | rule sequence
//
// Note that an empty sequence will produce a valid (i.e.,
// non-empty) node.
template<typename Parser,
         typename Stream,
         typename Rule,
         typename Term = Term_type<Parser, Stream, Rule>>
inline Sequence_term<Term> const*
parse_sequence(Parser& p, Stream& s, Rule rule)
{
  using Result = Sequence_term<Term>;
  Result result;
  while (!s.eof()) {
    if (Required<Term> term = rule(p, s))
      result.push_back(*term);
    else if (term.is_error())
      return make_error_node<Result>();
    else if (term.is_empty())
      break;
  }
  return Result::make(std::move(result));
}


// Parse a token-separated list of terms.
//
//    list ::= <empty> | rule | rule <token> list
//
// Here, <token> is the punctuator for the list.
template<typename Parser,
         typename Stream,
         typename Rule,
         typename Term = Term_type<Parser, Stream, Rule>>
inline Sequence_term<Term> const*
parse_list(Parser& p, Stream& s, int k, Rule rule)
{
  using Result = Sequence_term<Term>;
  Result result;

  // Match the first term. Note that this can be empty.
  if (Optional<Term> first = rule(p, s)) {

    // There were no matching terms.
    if (first.is_empty())
      return Result::make();

    // Save the first term and match all subsequent terms.
    result.push_back(*first);

    // Match a series of ',' rule terms.
    while (Token const* tok = match_token(s, k)) {
      if (Required<Term> next = rule(p, s)) {
        result.push_back(*next);
      } else {
        if (next.is_error())
          error(tok->location(), "expected {} after '{}'",
                get_grammar_name(rule),
                get_token_spelling(k));
        return make_error_node<Result>();
      }
    }

    return Result::make(std::move(result));
  } else {
    return make_error_node<Result>();
  }
}


// -------------------------------------------------------------------------- //
//                           Prefix parsing


// Parse a prefix term. A prefix term (or unary, in some grammars)
// has the following form:
//
//    prefix-term ::= rule | op prefix-term
//
// Here, `op` is a function that accepts the set of prefix operators
// and returns a pointer to a token. It must have the form:
//
//    op(p, s)
//
// Where `p` is the the parser and `s` is the token stream. The
// `rule` is a Parse_function that parses the next higher precedence
// term in the grammar.
//
// The `act` parameter specifies an action to execute whenever
// the prefix term is matched. It must be invokable as:
//
//    act(k, t)
//
// Where `k` is a token pointer and `t` is the nested term. The
// return type of the pointer must be the same as that of `rule`.
template<typename Parser,
         typename Stream,
         typename Op,
         typename Rule,
         typename Action,
         typename Term = Term_type<Parser, Stream, Rule>>
Term const*
parse_prefix_term(Parser& p, Stream& s, Op op, Rule rule, Action act)
{
  if (auto const* tok = op(p, s)) {
    if (Required<Term> term = parse_prefix_term(p, s, op, rule, act)) {
      return act(tok, *term);
    } else {
      // Failed to parse the sub-term after the prefix token.
      if (term.is_empty()) {
        error(tok->location(), "expected {} after '{}'",
              get_grammar_name(rule),
              tok->str());
        return make_error_node<Term>();
      }
      return *term;
    }
  }
  return rule(p, s);
}


// -------------------------------------------------------------------------- //
//                           Infix parsers


// Parse a left associative binary expression. This has the form:
//
//    left-infix-term ::= rule | left-infix-term op rule
//
// Here, `op` is a function that accepts the set of infix operators
// and returns a pointer to a token. It must have the form:
//
//    op(p, s)
//
// Where `p` is the the parser and `s` is the token stream. The
// `rule` is a Parse_function that parses the next higher precedence
// term in the grammar.
//
// The `act` parameter specifies an action to execute whenever
// the prefix term is matched. It must be invokable as:
//
//    act(k, t)
//
// Where `k` is a token pointer and `t` is the nested term. The
// return type of the pointer must be the same as that of `rule`.
template<typename Parser,
         typename Stream,
         typename Op,
         typename Rule,
         typename Action,
         typename Term = Term_type<Parser, Stream, Rule>>
Term const*
parse_left_infix_term(Parser& p, Stream& s, Op op, Rule rule, Action act)
{
  Location loc = s.location();
  if (Required<Term> left = rule(p, s)) {
    while (Token const* tok = op(p, s)) {
      if (Required<Term> right = rule(p, s)) {
        left = act(tok, *left, *right);
      } else  {
        // We did not match the right operand after the token.
        if (right.is_empty()) {
          error(tok->location(), "expected {} after '{}'",
                get_grammar_name(rule),
                tok->str());
          return make_error_node<Term>();
        }
      }
    }

    // We matched the left term, but not the operator.
    return *left;
  } else if (left.is_empty()) {
    // We did not match the left operand and got an empty
    // node. This is an error. This shhould never actually
    // happen. There should be an error if matching fails,
    // this is jus provided as a safeguard.
    error(loc, "expected {}", get_grammar_name(rule));
    return make_error_node<Term>();
  } else {
    // There was an error matching the left operand.
    return *left;
  }
}


// Parse a right associative infix interm with the form:
//
//    right-infix-term ::= rule | rule op right-infix-term
//
// Here, `op` is a function that accepts the set of infix operators
// and returns a pointer to a token. It must have the form:
//
//    op(p, s)
//
// Where `p` is the the parser and `s` is the token stream. The
// `rule` is a Parse_function that parses the next higher precedence
// term in the grammar.
//
// The `act` parameter specifies an action to execute whenever
// the prefix term is matched. It must be invokable as:
//
//    act(k, t)
//
// Where `k` is a token pointer and `t` is the nested term. The
// return type of the pointer must be the same as that of `rule`.
//
// TODO: Make this iterative?
template<typename Parser,
         typename Stream,
         typename Op,
         typename Rule,
         typename Action,
         typename Term = Term_type<Parser, Stream, Rule>>
Term const*
parse_right_infix_term(Parser& p, Stream& s, Op op, Rule rule, Action act)
{
  Location loc = s.location();
  if (Required<Term> left = rule(p, s)) {
    if (Token const* tok = op(p, s)) {
      if (Required<Term> right = parse_right_infix_term(p, s, op, rule, act)) {
        left = act(tok, *left, *right);
      } else {
        // We matched the token but not the right operand.
        if (right.is_empty()) {
          error(tok->location(), "expected {} after '{}'",
                get_grammar_name(rule),
                tok->str());
          return make_error_node<Term>();
        }
        return *left;
      }
    }

    // We matched the left operand, but not the token
    return *left;
  } else if (left.is_empty()) {
    // We did not match the left operand and got an empty
    // node. This is an error. This shhould never actually
    // happen. There should be an error if matching fails,
    // this is jus provided as a safeguard.
    error(loc, "expected {}", get_grammar_name(rule));
    return make_error_node<Term>();
  } else {
    // There was an error matching the left operand.
    return *left;
  }
}


} // namespace lingo


#endif
