// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#ifndef LINGO_NODE_HPP
#define LINGO_NODE_HPP

#include <lingo/utility.hpp>

#include <cassert>
#include <vector>
#include <type_traits>

// This module provides defines a node abstraction for various
// kinds of trees used in different languages and facilities
// for working with those abstractions.

namespace lingo
{

// Returns the name of the object pointed to
// by t.
template<typename T>
inline String
get_node_name(T const* t)
{
  return type_str(*t);
}


// -------------------------------------------------------------------------- //
//                            Special values
//
// The following functions define special values of node pointers.


// Returns true if the node is empty.
template<typename T>
inline bool
is_empty_node(T const* t)
{
  return t == nullptr;
}


// Construct a node pointer that acts as an error value.
// The type of the node is explicitly given as a template
// argument.
template<typename T>
inline T*
make_error_node()
{
  return (T*)0x01;
}


// Returns true if `t` is an error node.
template<typename T>
inline bool
is_error_node(T const* t)
{
  return t == make_error_node<T>();
}


// Returns true if `t` is neither null nor an error.
template<typename T>
inline bool
is_valid_node(T const* t)
{
  return t && !is_error_node(t);
}


// -------------------------------------------------------------------------- //
//                        Dynamic type information


// Returns true if the object pointed to by `u` has
// the dynamic type `T`.
template<typename T, typename U>
inline bool
is(U const* u)
{
  return dynamic_cast<T const*>(u);
}


// Statically cast a pointer to a Node of type T to a
// pointer to a Node of type U. This is not a checked
// operation (except in debug mode).
//
// Note that this allows null and error nodes to be
// interpreted as nodes of the given type (as their
// values are considered common to all).
template<typename T, typename U>
inline T*
cast(U* u)
{
  lingo_assert(is_valid_node(u) ? is<T>(u) : true);
  return static_cast<T*>(u);
}


template<typename T, typename U>
inline T const*
cast(U const* u)
{
  assert(is_valid_node(u) ? is<T>(u) : true);
  return static_cast<T const*>(u);
}


// Returns `u` with type `T*` iff the object pointed
// to by `u` has dynamic type `T`.
template<typename T, typename U>
inline T*
as(U* u)
{
  return dynamic_cast<T*>(u);
}


template<typename T, typename U>
inline T const*
as(U const* u)
{
  return dynamic_cast<T const*>(u);
}


// Return a non-const pointer to the term. This is used
// to modify a term post-initializatoin (which should
// be rare).
template<typename T>
inline T*
modify(T const* t)
{
  return const_cast<T*>(t);
}


// -------------------------------------------------------------------------- //
//                              Concepts
//
// There are several concepts describing the kinds of nodes over
// which an algorithm can operate generically. The primary
// characterization of node types is based on their arity.
//
// ## The Node concept
//
// Every node in an abstract syntax tree must provide a static
// constexpr member, node_kind that statically defines the kind
// of node.
//
// ## Node arity
//
// Nodes with fixed arity have tuple-like member names (e.g.,
// first, second, third). These accessor members correspond to
// the sub-terms of each node. Accessor members may have different
// tpyes, and are not required to be nodes.
//
// A nullary node is a Node is an empty tuple, and has no
// accessor members. A unary node has only accessor member
// `first`. A binary node has only `first` and `second`.
// A ternary node has `first`, second`, and `third`.
//
// Note that more arity nodes can be defined if needed.
//
// A k-ary has k sub-terms, and that range of terms is accessed
// by its `begin()` and `end()` members. The types of these
// sub-terms are the same.


namespace traits
{


// A helper trait used to detect substitution failures.
template<typename T>
struct is_non_void
{
  static constexpr bool value = true;
};


template<>
struct is_non_void<void>
{
  static constexpr bool value = false;
};


// Detect the existince of the member t->first.
template<typename T>
struct first_type
{
  template<typename U> static auto f(U* p) -> decltype(p->first);
  static                      void f(...);

  using type = decltype(f(std::declval<T*>()));
};


// Detect the existence of the member t->second;
template<typename T>
struct second_type
{
  template<typename U> static auto f(U* p) -> decltype(p->second);
  static                      void f(...);

  using type = decltype(f(std::declval<T*>()));
};


// Detect the existence of the member t->third;
template<typename T>
struct third_type
{
  template<typename U> static auto f(U* p) -> decltype(p->third);
  static                      void f(...);

  using type = decltype(f(std::declval<T*>()));
};


// Detect the existence of the member t->begin().
template<typename T>
struct begin_type
{
  template<typename U> static auto f(U* p) -> decltype(p->begin());
  static                      void f(...);

  using type = decltype(f(std::declval<T*>()));
};



// Detect the existence of the member t->end().
template<typename T>
struct end_type
{
  template<typename U> static auto f(U* p) -> decltype(p->end());
  static                      void f(...);

  using type = decltype(f(std::declval<T*>()));
};


// Returns true when `T` has the member `first`.
template<typename T>
constexpr bool
has_first()
{
  return is_non_void<typename first_type<T>::type>::value;
}


// Returns true when `T` has the member `second`.
template<typename T>
constexpr bool
has_second()
{
  return is_non_void<typename second_type<T>::type>::value;
}


// Returns true when `T` has the member `third`.
template<typename T>
constexpr bool
has_third()
{
  return is_non_void<typename third_type<T>::type>::value;
}


// Returns true when `T` has the member `begin`.
template<typename T>
constexpr bool
has_begin()
{
  return is_non_void<typename begin_type<T>::type>::value;
}


// Returns true when `T` has the member `end`.
template<typename T>
constexpr bool
has_end()
{
  return is_non_void<typename end_type<T>::type>::value;
}


} // namesapce traits


// Returns true if T is a Nullary_node.
template<typename T>
constexpr bool
is_nullary_node()
{
  return !traits::has_first<T>();
}


// Returns true if T is a unary node.
template<typename T>
constexpr bool
is_unary_node()
{
  return traits::has_first<T>()
      && !traits::has_second<T>();
}


// Returns true if T is a binary node.
template<typename T>
constexpr bool
is_binary_node()
{
  return traits::has_second<T>()
      && !traits::has_third<T>();
}


// Returns true if T is a ternary node.
template<typename T>
constexpr bool
is_ternary_node()
{
  return traits::has_first<T>()
      && traits::has_second<T>()
      && traits::has_third<T>();
}


// Returns true if T is a k-ary node.
template<typename T>
constexpr bool
is_kary_node()
{
  return traits::has_begin<T>() && traits::has_end<T>();
}



// -------------------------------------------------------------------------- //
//                        Reqiured term

// The Maybe template is typically used to declare node
// pointers within condition declarations.
//
//    if (Required<Var_decl> var = ...)
//
// This class contextually evaluates to `true` whenever
// it is initialized to a non-null, non-error value.
template<typename T>
struct Required
{
  Required()
    : ptr(nullptr)
  { }

  Required(T const* p)
    : ptr(p)
  { }

  // Returns true iff the term is valid.
  explicit operator bool() const { return is_valid_node(ptr); }

  // Returns the underlying term, even if it is an error or
  // empty term.
  T const* operator*() const { return ptr; }
  T const* operator->() const { return ptr; }

  bool is_error() const { return is_error_node(ptr); }
  bool is_empty() const { return is_empty_node(ptr); }

  T const* ptr;
};


// -------------------------------------------------------------------------- //
//                        Optional results

// The Optional template is typically used to declare node
// pointers within condition declarations.
//
//    if (Optional<Var_decl> var = ...)
//
// This class contextually evaluates to `true` whenever
// it is a non-error value. Note that the term may be empty.
template<typename T>
struct Optional
{
  Optional()
    : ptr(nullptr)
  { }

  Optional(T const* p)
    : ptr(p)
  { }

  // Returns true iff the term is valid or empty.
  explicit operator bool() const { return !is_error_node(ptr); }

  // Returns the underlying term, even if it is an error.
  T const* operator*() const { return ptr; }
  T const* operator->() const { return ptr; }

  bool is_error() const { return is_error_node(ptr); }
  bool is_empty() const { return is_empty_node(ptr); }

  T const* ptr;
};


// -------------------------------------------------------------------------- //
//                        Nonempty results

// The Nonempty template is typically used to declare node
// pointers within condition declarations.
//
//    if (Nonempty<Var_decl> var = ...)
//
// This class contextually evaluates to `true` whenever
// is non-empty. Note that error conditions are treated as
// valid results.
template<typename T>
struct Nonempty
{
  Nonempty()
    : ptr(nullptr)
  { }

  Nonempty(T const* p)
    : ptr(p)
  { }

  // Returns true iff the term is non-empty.
  explicit operator bool() const { return !is_empty_node(ptr); }

  // Returns the underlying term, even if it is a empty.
  T const* operator*() const { return ptr; }
  T const* operator->() const { return ptr; }

  bool is_error() const { return is_error_node(ptr); }
  bool is_empty() const { return is_empty_node(ptr); }

  T const* ptr;
};



} // namespace lingo


#endif
