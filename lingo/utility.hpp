// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#ifndef LINGO_UTILITY_HPP
#define LINGO_UTILITY_HPP

#include "lingo/string.hpp"

#include <typeinfo>


namespace lingo
{

// -------------------------------------------------------------------------- //
//                              Useful types

// Like void, but has a value. This is used as
// a replacement for void types in some templates.
struct Void { };


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

// This class provides support for caching the result
// of visitor functors, with appropriate support for
// void return types. F is the function type containing
// the dispatch table, and T is the return type of that
// function.
template<typename F, typename T>
struct Generic_visitor
{
  // Determine a reasonable result type. Note that
  // void cannot be the type of a member.
  using R = typename std::conditional<std::is_void<T>::value, Void, T>::type;

  // A tag type used for dispatch. This is either
  // std::true_type or std::false_type.
  using X = typename std::is_void<T>::type;

  Generic_visitor(F f)
    : fn(f), r()
  { }

  ~Generic_visitor()
  { }

  // Called when the result of fn(u) is void.
  template<typename U>
  void invoke(U const* u, std::true_type) { fn(u); }

  // Called when the resyult of fn(u) is non-void.
  template<typename U>
  void invoke(U const* u, std::false_type) { 
    r = fn(u); 
  }

  // Invoke one of the functions above.
  template<typename U>
  void invoke(U const* u) { invoke(u, X()); }

  F fn;
  R r;
};


// Apply the function f to the type t. This overload
// is valid when the result type of R is void.
template<typename T, typename V, typename R = typename V::R>
inline typename std::enable_if<std::is_same<R, Void>::value, void>::type
accept(T const* t, V v)
{
  t->accept(v);
}


// Apply the function f to the type t. This overload
// is valid when the result type of R is non-void.
template<typename T, typename V, typename R = typename V::R>
inline typename std::enable_if<!std::is_same<R, Void>::value, R>::type
accept(T const* t, V v)
{
  t->accept(v);
  return v.r;
}


} // namespace lingo


#endif
