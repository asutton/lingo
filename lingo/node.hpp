// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#ifndef LINGO_NODE_HPP
#define LINGO_NODE_HPP

#include <cassert>
#include <type_traits>

// This module provides defines a node abstraction for various
// kinds of trees used in different languages and facilities
// for working with those abstractions.

namespace lingo
{

// The Kind_of class is a helper class that supports the definition 
// of node models. This provides a static representation of the 
// node kind and a static `is` function for dynamic type testing.
template<typename T, T K>
struct Kind_base
{
  static constexpr T node_kind = K;

  static bool is(T k) { return node_kind == k; }
};


// -------------------------------------------------------------------------- //
//                        Generic terms


// The Term template provides a facility for making an arbitrary
// type model the requirements of a node. This is useful for 
// defining terms that do not fall into other categegories (types,
// expressions, declarations, statments, etc).
//
// The "kind" of the node can be specified by the integer template
// argument N, but it is rarely useful to define it as anything
// other.
template<int N = 0>
struct Term : Kind_base<int, N>
{
  char const* node_name() const { return "<unspecified term>"; }
  int kind() const              { return N; }
};


// -------------------------------------------------------------------------- //
//                        Dynamic type information


// Returns true if the object pointed to by `u` has 
// the dynamic type `T`.
template<typename T, typename U>
inline bool
is(U const* u)
{
  return T::is(u->kind());
}


// Statically cast a pointer to a Node of type T to a 
// pointer to a Node of type U. This is not a checked
// operation.
//
// TODO: Support checking by making node kind
// a static property of all node classes. 
template<typename T, typename U>
inline T* 
cast(U* u)
{
  assert(is<T>(u));
  return static_cast<T*>(u);
}


template<typename T, typename U>
inline T const*
cast(U const* u)
{
  assert(is<T>(u));
  return static_cast<T const*>(u);
}


// Returns `u` with type `T*` iff the object pointed
// to by `u` has dynamic type `T`.
template<typename T, typename U>
inline T*
as(U* u)
{
  return is<T>(u) ? cast<T>(u) : nullptr;
}


template<typename T, typename U>
inline T const*
as(U const* u)
{
  return is<T>(u) ? cast<T>(u) : nullptr;
}


// -------------------------------------------------------------------------- //
//                              Concepts
//
// There are several concepts describing the kinds of nodes over  
// which an algorithm can operate generically. The primary 
// characterization // of node types is based on their arity. 
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


// Detect the existince of the member T::node_kind. 
template<typename T>
struct node_kind_type
{
  template<typename U> static auto f(U* p) -> decltype(U::node_kind);
  static                      void f(...);

  using type = decltype(f(std::declval<T*>()));
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


// Returns true when `T` has the static member object `node_kind`.
template<typename T>
constexpr bool
has_node_kind()
{
  return is_non_void<typename node_kind_type<T>::type>::value;
}


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


// Returns true if T models the Node concept.
template<typename T>
constexpr bool 
is_node()
{
  return traits::has_node_kind<T>();
}


// Returns true if T is a Nullary_node.
template<typename T>
constexpr bool
is_nullary_node()
{
  return is_node<T>()
      && !traits::has_first<T>()  // Not a unary node
      && !traits::has_begin<T>(); // Not a k-ary node
}


// Returns true if T is a unary node.
template<typename T>
constexpr bool
is_unary_node()
{
  return is_node<T>() 
      && traits::has_first<T>() 
      && !traits::has_second<T>();
}


// Returns true if T is a binary node.
template<typename T>
constexpr bool
is_binary_node()
{
  return is_node<T>() 
      && traits::has_second<T>() 
      && !traits::has_third<T>();
}


// Returns true if T is a ternary node.
template<typename T>
constexpr bool
is_ternary_node()
{
  return is_node<T>() 
      && traits::has_first<T>()
      && traits::has_second<T>() 
      && traits::has_third<T>();
}


// Returns true if T is a k-ary node.
template<typename T>
constexpr bool
is_kary_node()
{
  return is_node<T>() && traits::has_begin<T>() && traits::has_end<T>();
}


// -------------------------------------------------------------------------- //
//                            Special values
//
// The following functions define special values of node pointers.


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


// The Maybe template is typically used to declare node 
// pointers within condition declarations.
//
//    if (Maybe<Var_decl> var = ...)
//
// This class contextually evaluates to `true` whenever
// it is initialized to a non-null, non-error value. 
//
// TODO: This should be a specialization of std::optional.
template<typename T>
struct Maybe
{
  Maybe(T const* p)
    : ptr(p)
  { }

  explicit operator bool() const { return is_valid_node(ptr); }

  T const* operator*() const { return ptr; }

  T const* ptr;
};


} // namespace lingo


#endif
