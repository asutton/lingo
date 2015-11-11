// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#ifndef LINGO_UTILITY_HPP
#define LINGO_UTILITY_HPP

#include "lingo/string.hpp"

#include <typeinfo>


namespace lingo
{

// -------------------------------------------------------------------------- //
//                    String representation of types

String 
type_str(std::type_info const&);


// Returns the name of an object of type t.
template<typename T>
inline String 
type_str(T const& t)
{
  return type_str(typeid(t));
}


// -------------------------------------------------------------------------- //
//                            Generic visitors


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

  // Enable tag dispatch.
  static non_void_tag tag() { return {}; }

  F fn;
  T r;
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
accept(T const* t, V& v, non_void_tag)
{
  t->accept(v);
  return v.r;
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
inline auto
accept(T const* t, V& v)
{
  return accept(t, v, v.tag());
}


template<typename T, typename V>
inline auto
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

  // Enable tag dispatch.
  static non_void_tag tag() { return {}; }

  F fn;
  T r;
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
accept(T* t, V& v, non_void_tag)
{
  t->accept(v);
  return v.r;
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
inline auto
accept(T* t, V& v)
{
  return accept(t, v, v.tag());
}


template<typename T, typename V>
inline auto
accept(T& t, V& v)
{
  return accept(&t, v, v.tag());
}


} // namespace lingo


#endif
