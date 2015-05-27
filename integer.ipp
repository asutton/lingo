// Copyright (c) 2015 Andrew Sutton
// All rights reserved

namespace lingo {

// Default initialize the integer value.
//
// Note that GMP does ! optimize for default-intialized values. 
inline
Integer::Integer() 
  : base_(10) 
{ 
  mpz_init(value_); 
}


// Copy initialize this object with x.
inline
Integer::Integer(Integer const& x) 
  : base_(x.base_) 
{ 
  mpz_init_set(value_, x.value_); 
}


// Copy assign this object to the value of x.
inline Integer&
Integer::operator=(Integer const& x) 
{
  if (this != &x) {
    mpz_clear(value_);
    mpz_set(value_, x.value_);
    base_ = x.base_;
  }
  return *this;
}


// Construct an integer with the value n.
inline
Integer::Integer(long n, int b)
  : base_(b)
{ 
  mpz_init_set_si(value_, n); 
}


// Destroy the ionteger, releasing resources.
inline
Integer::~Integer() 
{ 
  mpz_clear(value_); 
}


inline Integer& 
Integer::operator+=(Integer const& x) 
{
  mpz_add(value_, value_, x.value_);
  return *this;
}


inline Integer& 
Integer::operator-=(Integer const& x) 
{
  mpz_sub(value_, value_, x.value_);
  return *this;
}


inline Integer& 
Integer::operator*=(Integer const& x) 
{
  mpz_mul(value_, value_, x.value_);
  return *this;
}


// Divide this integer value by x. Integer division is implemented as
// floor division. A discussion of alternatives can be found in the paper,
// "The Euclidean definition of the functions div and mod" by Raymond T.
// Boute (http://dl.acm.org/citation.cfm?id=128862).
inline Integer& 
Integer::operator/=(Integer const& x) 
{
  mpz_fdiv_q(value_, value_, x.value_);
  return *this;
}


// Compute the remainder of the division of this value_ by x. Integer 
// division is implemented as floor division. See the notes on operator/= 
// for more discussion.
inline Integer& 
Integer::operator%=(Integer const& x) 
{
  mpz_fdiv_r(value_, value_, x.value_);
  return *this;
}


inline Integer&
Integer::operator&=(Integer const& x) 
{
  mpz_and(value_, value_, x.value_);
  return *this;
}


inline Integer&
Integer::operator|=(Integer const& x) 
{
  mpz_ior(value_, value_, x.value_);
  return *this;
}


inline Integer&
Integer::operator^=(Integer const& x) 
{
  mpz_xor(value_, value_, x.value_);
  return *this;
}


// Left-shift the integer value by the specified amount.
inline Integer&
Integer::operator<<=(Integer const& x) 
{
  mpz_mul_2exp(value_, value_, x.getu());
  return *this;
}


// Right-shift the integer value by the specified amount. This will
// just "do the right thing" and preserves the sign bit when shifting.
// It is appropriate for both signed and unsigned integer 
// representations.
inline Integer&
Integer::operator>>=(Integer const& x) 
{
  mpz_fdiv_q_2exp(value_, value_, x.getu());
  return *this;
}


// Negate this value.
inline Integer&
Integer::neg() 
{
  mpz_neg(value_, value_);
  return *this;
}


// Set this value to its absolute value.
inline Integer&
Integer::abs() 
{
  mpz_abs(value_, value_);
  return *this;
}


// Set this value to its one's complement.
inline Integer&
Integer::comp() 
{
  mpz_com(value_, value_);
  return *this;
}

// Returns the signum of the value.
inline int
Integer::sign() const 
{ 
  return mpz_sgn(value_); 
}


// Returns true if the value is strictly positive.
inline bool
Integer::is_positive() const 
{ 
  return sign() > 0; 
}


// Returns true if the value is strictly negative.
inline bool
Integer::is_negative() const 
{ 
  return sign() < 0; 
}


// Returns true if the value is nonpositive.
inline bool
Integer::is_nonpositive() const 
{ 
  return sign() <= 0; 
}


// Returns true if the value is nonnegative.
inline bool
Integer::is_nonnegative() const 
{ 
  return sign() >= 0; 
}


// Returns the number of bits in the integer representation.
inline int
Integer::bits() const 
{ 
  return mpz_sizeinbase(value_, 2); 
}


// Returns the base of in which the inteer should be formatted.
inline int
Integer::base() const 
{ 
  return base_; 
}


// Returns the value as an unsigned integer.
inline std::uintmax_t
Integer::getu() const 
{ 
  assert(is_nonnegative());
  return mpz_get_ui(value_);
}


// Returns the value as a signed integer.
inline std::intmax_t
Integer::gets() const 
{ 
  return mpz_get_si(value_);
}


// Returns a reference to the underlying data.
inline const mpz_t& 
Integer::data() const 
{ 
  return value_; 
}


// Equality comparison
// Returns true when the two integers have the same value.
inline bool
operator==(Integer const& a, Integer const& b) 
{
  return mpz_cmp(a.data(), b.data()) == 0;
}


inline bool 
operator!=(Integer const& a, Integer const& b) 
{ 
  return !(a == b); 
}


// Ordering
// Returns true when a is less than b.
inline bool
operator<(Integer const& a, Integer const& b) 
{
  return mpz_cmp(a.data(), b.data()) < 0;
}


inline bool
operator>(Integer const& a, Integer const& b) 
{ 
  return b < a; 
}


inline bool
operator<=(Integer const& a, Integer const& b) 
{ 
  return !(b < a); 
}


inline bool
operator>=(Integer const& a, Integer const& b) 
{ 
  return !(a < b); 
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
  return Integer(x).neg(); 
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
  return Integer(x).comp(); 
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


} // namespace lingo
