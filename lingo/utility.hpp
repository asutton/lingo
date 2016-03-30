// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#ifndef LINGO_UTILITY_HPP
#define LINGO_UTILITY_HPP

#include <lingo/assert.hpp>

#include <string>
#include <typeinfo>
#include <utility>

namespace lingo
{

// -------------------------------------------------------------------------- //
// String representation of types

std::string
type_str(std::type_info const&);


// Returns the name of an object of type t.
template<typename T>
inline std::string
type_str(T const& t)
{
  return type_str(typeid(t));
}


// -------------------------------------------------------------------------- //
// Dynamic type information


// Returns true if the object pointed to by `u` has
// the dynamic type `T`.
template<typename T, typename U>
inline bool
is(U const* u)
{
  return dynamic_cast<T const*>(u);
}


// Statically cast a pointer to a Node of type T to a pointer
// to a Node of type U. This is not a checked operation (except
// in debug mode).
template<typename T, typename U>
inline T*
cast(U* u)
{
  lingo_assert(u ? is<T>(u) : true);
  return static_cast<T*>(u);
}


template<typename T, typename U>
inline T const*
cast(U const* u)
{
  lingo_assert(u ? is<T>(u) : true);
  return static_cast<T const*>(u);
}


// Statically cast a reference to U to a reference to T.
// Behavior is undefined if the ast is invalid.
template<typename T, typename U>
inline T&
cast(U& u)
{
  return static_cast<T&>(u);
}


template<typename T, typename U>
inline T const&
cast(U const& u)
{
  return static_cast<T const&>(u);
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


// Dynamically cast a reference to U to a reference to T.
// An exception is thrown if this is a bad cast.
template<typename T, typename U>
inline T&
as(U& u)
{
  return dynamic_cast<T&>(u);
}


template<typename T, typename U>
inline T const&
as(U const& u)
{
  return dynamic_cast<T const&>(u);
}


// Return a non-const pointer to the term. This is used
// to modify a term post-initialization (which should
// be rare).
template<typename T>
inline T*
modify(T const* t)
{
  return const_cast<T*>(t);
}


template<typename T>
inline T&
modify(T const& t)
{
  return const_cast<T&>(t);
}


// -------------------------------------------------------------------------- //
// Source code locations

// A locus is a line/column offset within a file.
using Locus = std::pair<int, int>;


// -------------------------------------------------------------------------- //
// Generic visitors

struct void_tag { };
struct non_void_tag { };


// This class provides support for caching the result
// of visitor functors, with appropriate support for
// void return types. F is the function type containing
// the dispatch table, and T is the return type of that
// function.
//
// TODO: Build a specialization for reference return types.
// That has more to do with initialization than anything
// else.
template<typename F, typename T>
struct Generic_visitor
{
  Generic_visitor(F f)
    : fn(f), r()
  { }

  // Dispatch to the wrapped function object.
  template<typename U>
  void invoke(U const* u) { r = fn(u); }

  template<typename U>
  void invoke(U const& u) { r = fn(u); }

  T result() const { return r; }

  // Enable tag dispatch.
  static non_void_tag tag() { return {}; }

  F fn;
  T r;
};


// Specialization for reference returns. The return value must
// outlive the visitor, otherwise visitation will result in
// undefined behavior.
template<typename F, typename T>
struct Generic_visitor<F, T&>
{
  Generic_visitor(F f)
    : fn(f), r()
  { }

  // Dispatch to the wrapped function object.
  template<typename U>
  void invoke(U const* u) { r = &fn(u); }

  template<typename U>
  void invoke(U const& u) { r = &fn(u); }

  T& result() const { return *r; }

  // Enable tag dispatch.
  static non_void_tag tag() { return {}; }

  F fn;
  T* r;
};


template<typename F>
struct Generic_visitor<F, void>
{
  Generic_visitor(F f)
    : fn(f)
  { }

  // Dispatch to the wrapped function object.
  template<typename U>
  void invoke(U const* u) { fn(u); }

  template<typename U>
  void invoke(U const& u) { fn(u); }

  // Enable tag dispatch.
  static void_tag tag() { return {}; }

  F fn;
};


// Invoke the visitor, returning the function's value.
template<typename T, typename V>
inline auto
accept(T const* t, V& v, non_void_tag) -> decltype(v.result())
{
  t->accept(v);
  return v.result();
}


// Invoke the visitor. This does not return a value.
template<typename T, typename V>
inline void
accept(T const* t, V& v, void_tag)
{
  t->accept(v);
}



// Invoke the visitor, returning the function's value.
template<typename T, typename V>
inline decltype(auto)
accept(T const* t, V& v)
{
  return accept(t, v, v.tag());
}


template<typename T, typename V>
inline decltype(auto)
accept(T const& t, V& v)
{
  return accept(&t, v, v.tag());
}


// -------------------------------------------------------------------------- //
//                            Generic mutators


// A visitor that allows modification of values.
//
// TODO: Build a specialization for reference return types.
// That has more to do with initialization than anything
// else.
template<typename F, typename T>
struct Generic_mutator
{
  Generic_mutator(F f)
    : fn(f), r()
  { }

  // Dispatch to the wrapped function object.
  template<typename U>
  void invoke(U* u) { r = fn(u); }

  template<typename U>
  void invoke(U& u) { r = fn(u); }

  T result() const { return r; }

  // Enable tag dispatch.
  static non_void_tag tag() { return {}; }

  F fn;
  T r;
};


// Specialization for reference returns. The return value must
// outlive the visitor, otherwise visitation will result in
// undefined behavior.
template<typename F, typename T>
struct Generic_mutator<F, T&>
{
  Generic_mutator(F f)
    : fn(f), r()
  { }

  // Dispatch to the wrapped function object.
  template<typename U>
  void invoke(U* u) { r = &fn(u); }

  template<typename U>
  void invoke(U& u) { r = &fn(u); }

  T& result() const { return *r; }

  // Enable tag dispatch.
  static non_void_tag tag() { return {}; }

  F fn;
  T* r;
};


template<typename F>
struct Generic_mutator<F, void>
{
  Generic_mutator(F f)
    : fn(f)
  { }

  // Dispatch to the wrapped function object.
  template<typename U>
  void invoke(U* u) { fn(u); }

  template<typename U>
  void invoke(U& u) { fn(u); }

  // Enable tag dispatch.
  static void_tag tag() { return {}; }

  F fn;
};


// Invoke the visitor, returning the function's value.
template<typename T, typename V>
inline auto
accept(T* t, V& v, non_void_tag) -> decltype(v.result())
{
  t->accept(v);
  return v.result();
}


// Invoke the visitor. This does not return a value.
template<typename T, typename V>
inline void
accept(T* t, V& v, void_tag)
{
  t->accept(v);
}


// Invoke the visitor, returning the function's value.
template<typename T, typename V>
inline decltype(auto)
accept(T* t, V& v)
{
  return accept(t, v, v.tag());
}


template<typename T, typename V>
inline decltype(auto)
accept(T& t, V& v)
{
  return accept(&t, v, v.tag());
}


} // namespace lingo

#endif
