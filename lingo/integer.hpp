// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#ifndef LINGO_INTEGER_HPP
#define LINGO_INTEGER_HPP

// The integer module provides arbitrary precision integers
// and associated functions.

#include <lingo/assert.hpp>
#include <lingo/string.hpp>

#include <llvm/ADT/APInt.h>


namespace lingo
{

// Represents an arbitrary precision integer. The defualt interpretation
// of this value is as a signed integer. However, unsigned variants can
// also be constructed.
//
// TODO: Consider extending this to support byte ordering.
class Integer
{
public:
  Integer();

  // Copy semantics
  Integer(Integer const&);
  Integer& operator=(Integer const&);

  // Move semantics
  Integer(Integer&&);
  Integer& operator=(Integer&&);

  Integer(llvm::APInt const&);
  Integer& operator=(llvm::APInt const&);

  Integer(llvm::APInt&&);
  Integer& operator=(llvm::APInt&&);


  // Value construction.
  //
  // TODO: Support initialization from an array.
  Integer(std::int64_t);
  Integer(std::uint64_t, bool);
  Integer(int, std::uint64_t, bool);

  Integer& operator+=(Integer const&);
  Integer& operator-=(Integer const&);
  Integer& operator*=(Integer const&);
  Integer& operator/=(Integer const&);
  Integer& operator%=(Integer const&);

  Integer& operator&=(Integer const&);
  Integer& operator|=(Integer const&);
  Integer& operator^=(Integer const&);

  Integer& operator<<=(Integer const&);
  Integer& operator>>=(Integer const&);

  // Sign
  int sign() const;
  bool is_positive() const;
  bool is_negative() const;
  bool is_nonpositive() const;
  bool is_nonnegative() const;

  // Truth value
  bool truth_value() const;

  int bits() const;
  std::uint64_t getu() const;
  std::int64_t gets() const;

  llvm::APInt const& impl() const;

private:
  llvm::APInt z;
};


// Initialize a 32-bit signed 0.
inline
Integer::Integer()
  : z(32, 0, true)
{ }


// Copy initialize this object with x.
inline
Integer::Integer(Integer const& x)
  : z(x.z)
{ }


// Copy assign this object to the value of x.
inline Integer&
Integer::operator=(Integer const& x)
{
  z = x.z;
  return *this;
}


inline
Integer::Integer(Integer&& x)
  : z(std::move(x.z))
{ }


inline Integer&
Integer::operator=(Integer&& x)
{
  z = std::move(x.z);
  return *this;
}


inline
Integer::Integer(llvm::APInt const& n)
  : z(n)
{ }


inline
Integer&
Integer::operator=(llvm::APInt const& n)
{
  z = n;
  return *this;
}


inline
Integer::Integer(llvm::APInt&& n)
  : z(std::move(n))
{ }


inline
Integer&
Integer::operator=(llvm::APInt&& n)
{
  z = std::move(n);
  return *this;
}


// Initialize an integer with the (signed) value.
inline
Integer::Integer(std::int64_t n)
  : z(32, n, true)
{ }


// Initialize an integer with the given value. The
// value is considered signed if `s` is true.
inline
Integer::Integer(std::uint64_t n, bool s)
  : z(32, n, s)
{ }


// Initialize an integer value with `w` bits of
// precision. The value is considered signed if
// `s` is true.
inline
Integer::Integer(int w, std::uint64_t n, bool s)
  : z(w, n, s)
{ }


inline Integer&
Integer::operator+=(Integer const& x)
{
  z += x.z;
  return *this;
}


inline Integer&
Integer::operator-=(Integer const& x)
{
  z -= x.z;
  return *this;
}


inline Integer&
Integer::operator*=(Integer const& x)
{
  z += x.z;
  return *this;
}


// Signed division.
inline Integer&
Integer::operator/=(Integer const& x)
{
  z = z.sdiv(x.z);
  return *this;
}


// Signed remainder
inline Integer&
Integer::operator%=(Integer const& x)
{
  z = z.srem(x.z);
  return *this;
}


inline Integer&
Integer::operator&=(Integer const& x)
{
  z &= x.z;
  return *this;
}


inline Integer&
Integer::operator|=(Integer const& x)
{
  z |= x.z;
  return *this;
}


inline Integer&
Integer::operator^=(Integer const& x)
{
  z &= x.z;
  return *this;
}


inline Integer&
Integer::operator<<=(Integer const& x)
{
  z = z.shl(x.z);
  return *this;
}


// Arithmetic right shift.
inline Integer&
Integer::operator>>=(Integer const& x)
{
  z = z.ashr(x.z);
  return *this;
}


// Returns true if the value is strictly positive.
inline bool
Integer::is_positive() const
{
  return z.isStrictlyPositive();
}


// Returns true if the value is strictly negative.
inline bool
Integer::is_negative() const
{
  return z.isNegative();
}


// Returns true if the value is nonpositive.
inline bool
Integer::is_nonpositive() const
{
  return !is_positive();
}


// Returns true if the value is nonnegative.
inline bool
Integer::is_nonnegative() const
{
  return !is_negative();
}


// Returns the truth value interpretation of the integer.
inline bool
Integer::truth_value() const
{
  return z.getBoolValue();
}


// Returns the number of bits in the integer representation.
inline int
Integer::bits() const
{
  return z.getBitWidth();
}



// Returns the value as an unsigned integer.
inline std::uint64_t
Integer::getu() const
{
  return z.getZExtValue();
}


// Returns the value as a signed integer.
inline std::int64_t
Integer::gets() const
{
  return z.getSExtValue();
}


// Returns a reference to the underlying data.
inline llvm::APInt const&
Integer::impl() const
{
  return z;
}


// Equality comparison
// Returns true when the two integers have the same value.
inline bool
operator==(Integer const& a, Integer const& b)
{
  return a.impl() == b.impl();
}


inline bool
operator!=(Integer const& a, Integer const& b)
{
  return !(a == b);
}


// Ordering, defined for signed integers by default.
inline bool
operator<(Integer const& a, Integer const& b)
{
  return a.impl().slt(b.impl());
}


inline bool
operator>(Integer const& a, Integer const& b)
{
  return a.impl().sgt(b.impl());
}


inline bool
operator<=(Integer const& a, Integer const& b)
{
  return a.impl().sle(b.impl());
}


inline bool
operator>=(Integer const& a, Integer const& b)
{
  return a.impl().sge(b.impl());
}


// Arithmetic
inline Integer
operator+(Integer const& a, Integer const& b)
{
  return Integer(a) += b;
}


inline Integer
operator-(Integer const& a, Integer const& b)
{
  return Integer(a) -= b;
}


inline Integer
operator*(Integer const& a, Integer const& b)
{
  return Integer(a) *= b;
}


inline Integer
operator/(Integer const& a, Integer const& b)
{
  return Integer(a) /= b;
}


inline Integer
operator%(Integer const& a, Integer const& b)
{
  return Integer(a) %= b;
}


inline Integer
operator-(Integer const& x)
{
  return -x.impl();
}


inline Integer
operator+(Integer const& x)
{
  return x;
}


inline Integer
operator&(Integer const& a, Integer const& b)
{
  return Integer(a) &= b;
}


inline Integer
operator|(Integer const& a, Integer const& b)
{
  return Integer(a) |= b;
}


inline Integer
operator^(Integer const& a, Integer const& b)
{
  return Integer(a) ^= b;
}


inline Integer
operator~(Integer const& x)
{
  return ~x.impl();
}


inline Integer
operator<<(Integer const& a, Integer const& b)
{
  return Integer(a) <<= b;
}


inline Integer
operator>>(Integer const& a, Integer const& b)
{
  return Integer(a) >>= b;
}


// Streaming
std::ostream& operator<<(std::ostream&, Integer const&);


} // namespace lingo

#endif
