// Copyright (c) 2015-2016 Andrew Sutton
// All rights reserved

#ifndef LINGO_REAL_HPP
#define LINGO_REAL_HPP

#include <lingo/assert.hpp>
#include <lingo/string.hpp>

#include <llvm/ADT/APFloat.h>

namespace lingo
{

// Represents an arbitrary precision floating point value. By default,
// this represents an IEEE double precision value, but other models
// are also supported.
//
// TODO: This class needs some TLC.
class Real
{
public:
  Real();

  // Copy semantics
  Real(Real const&);
  Real& operator=(Real const&);

  // Move semantics
  Real(Real&&);
  Real& operator=(Real&&);

  Real(llvm::APFloat const&);
  Real& operator=(llvm::APFloat const&);

  Real(llvm::APFloat&&);
  Real& operator=(llvm::APFloat&&);


  // Value construction.
  //
  // TODO: Support initialization over floating
  // supported floating point semantics.
  explicit Real(double);


  llvm::APFloat const& impl() const;

private:
  llvm::APFloat f;
};


// Initialize an IEEE double precision floating point
// value to 0.0.
inline
Real::Real()
  : f(0.0)
{ }


// Copy initialize this object with x.
inline
Real::Real(Real const& x)
  : f(x.f)
{ }


// Copy assign this object to the value of x.
inline Real&
Real::operator=(Real const& x)
{
  f = x.f;
  return *this;
}


inline
Real::Real(Real&& x)
  : f(std::move(x.f))
{ }


inline Real&
Real::operator=(Real&& x)
{
  f = std::move(x.f);
  return *this;
}


inline
Real::Real(llvm::APFloat const& n)
  : f(n)
{ }


inline
Real&
Real::operator=(llvm::APFloat const& n)
{
  f = n;
  return *this;
}


inline
Real::Real(llvm::APFloat&& n)
  : f(std::move(n))
{ }


inline
Real&
Real::operator=(llvm::APFloat&& n)
{
  f = std::move(n);
  return *this;
}


// Initialize a floating point value with the (signed) value.
inline
Real::Real(double n)
  : f(n)
{ }


// Returns a reference to the underlying data.
inline llvm::APFloat const&
Real::impl() const
{
  return f;
}


// Equality comparison
// Returns true when the two floating point values have the same value.
inline bool
operator==(Real const& a, Real const& b)
{
  return a.impl().compare(b.impl()) == llvm::APFloat::cmpEqual;
}


inline bool
operator!=(Real const& a, Real const& b)
{
  return !(a == b);
}


// Ordering, defined for signed floating point values by default.
inline bool
operator<(Real const& a, Real const& b)
{
  return a.impl().compare(b.impl()) == llvm::APFloat::cmpLessThan;
}


inline bool
operator>(Real const& a, Real const& b)
{
  return a.impl().compare(b.impl()) == llvm::APFloat::cmpGreaterThan;
}


inline bool
operator<=(Real const& a, Real const& b)
{
  return a < b || a == b;
}


inline bool
operator>=(Real const& a, Real const& b)
{
  return a > b || a == b;
}


// Arithmetic
inline Real
operator+(Real const& a, Real const& b)
{
  return a.impl() + b.impl();
}


inline Real
operator-(Real const& a, Real const& b)
{
  return a.impl() - b.impl();
}


inline Real
operator*(Real const& a, Real const& b)
{
  return a.impl() * b.impl();
}


inline Real
operator/(Real const& a, Real const& b)
{
  return a.impl() / b.impl();
}


// FIXME: Make sure that zero has the same floating point
// semantics as x.
inline Real
operator-(Real const& x)
{
  Real zero;
  return zero - x.impl();
}


inline Real
operator+(Real const& x)
{
  return x;
}


// Streaming
std::ostream& operator<<(std::ostream&, Real const&);


} // namespace lingo

#endif
