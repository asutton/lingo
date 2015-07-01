// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#ifndef LINGO_NODE_HPP
#define LINGO_NODE_HPP

#include <cassert>

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
  Maybe(T* p)
    : ptr(p)
  { }

  explicit operator bool() const { return is_valid_node(ptr); }

  T*       operator*()       { return ptr; }
  T const* operator*() const { return ptr; }

  T* ptr;
};


} // namespace lingo


#endif
