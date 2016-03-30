// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#ifndef LINGO_STRING_HPP
#define LINGO_STRING_HPP

// The string module defines facilities and abstractions for
// working with character strings.

#include <lingo/assert.hpp>

#include <cstdint>
#include <algorithm>
#include <iosfwd>
#include <iterator>
#include <limits>
#include <locale>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

namespace lingo
{

// -------------------------------------------------------------------------- //
//                           Types

// A string is a sequence of characters.
using String = std::string;


// A vector of strings.
using String_seq = std::vector<String>;


// -------------------------------------------------------------------------- //
//                           Algorithms

namespace
{

template<typename CharT>
struct digits
{
  static const CharT* value;
};

template<>
struct digits<char>
{
  static constexpr const char* value = u8"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
};

template<>
struct digits<signed char>
{
  static constexpr const signed char* value = reinterpret_cast<const signed char*>(digits<char>::value);
};

template<>
struct digits<unsigned char>
{
  static constexpr const unsigned char* value = reinterpret_cast<const unsigned char*>(digits<char>::value);
};

template<>
struct digits<wchar_t>
{
  static constexpr const wchar_t* value = L"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
};

template<>
struct digits<char16_t>
{
  static constexpr const char16_t* value = u"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
};

template<>
struct digits<char32_t>
{
  static constexpr const char32_t* value = U"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
};

} // namespace


// Returns true if c is the horizontal whitespace.
// Note that vertical tabs and carriage returns
// are considered horizontal white space.
template<typename CharT>
inline bool
is_space(CharT c)
{
  return std::isspace(c, std::locale::classic());
}


// Returns true if c is alphabetical.
template<typename CharT>
inline bool
is_alpha(CharT c)
{
  return std::isalpha(c, std::locale::classic());
}


// Returns true if c is the newline character.
template<typename CharT>
inline bool
is_newline(CharT c)
{
  return c == static_cast<CharT>('\n');
}


// Returns true if c is a digit in the specified base.
template<typename CharT>
inline bool
is_digit(CharT c, int base)
{
  lingo_assert(base > 0 && base <= 36);
  return std::char_traits<CharT>::find(digits<CharT>::value, base, std::toupper(c, std::locale::classic()));
}


// Returns true if c is in the class [01].
template<typename CharT>
inline bool
is_binary_digit(CharT c)
{
  return is_digit(c, 2);
}


// Returns true if c is an octal digit.
template<typename CharT>
inline bool
is_octal_digit(CharT c)
{
  return is_digit(c, 8);
}


// Returns true if c is a decimal digit.
template<typename CharT>
inline bool
is_decimal_digit(CharT c)
{
  return is_digit(c, 10);
}


// Returns true if c is a hexadecimal digit.
template<typename CharT>
inline bool
is_hexadecimal_digit(CharT c)
{
  return is_digit(c, 16);
}


// Returns the integral value of character c in the specified base, or -1 if
// c is not a valid digit.
template<typename CharT>
inline int
digit_value(CharT c, int base)
{
  lingo_assert(base > 0 && base <= 36);
  const CharT* p = std::char_traits<CharT>::find(digits<CharT>::value, base, std::toupper(c, std::locale::classic()));
  if (!p)
    return -1;
  return p - digits<CharT>::value;
}


// Returns the integer value of the string in [first, last),
// which contains an integer representation in base b. If no
// conversion could be performed, or if [first, last) contains
// any characters that are not digits in base b, this throws an
// std::invalid_argument exception. If the integer is outside
// the range of the result type, this throws an
// std::out_of_range exception.
//
// Note that T must be an integer type and is given as an
// explicit template argument.
template<typename T, typename I>
T
string_to_int(I first, I last, int b)
{
  typedef typename std::iterator_traits<I>::value_type char_type;

  typename std::common_type<std::intmax_t, T>::type n = 0;
  std::size_t digit_count = 0;
  bool neg = false;

  if (first != last) {
    switch (*first) {
      case static_cast<char_type>('+'):
        ++first;
        break;
      case static_cast<char_type>('-'):
        neg = true;
        ++first;
        break;
    }
  }

  while (first != last) {
    const int d = digit_value(*first, b);
    if (d == -1)
      throw std::invalid_argument("lingo::string_to_int");
    n = n * b + d;
    ++first;
    ++digit_count;
  }

  if (!digit_count)
    throw std::invalid_argument("lingo::string_to_int");

  n = neg ? -n : n;

  if (n < std::numeric_limits<T>::min() || n > std::numeric_limits<T>::max())
    throw std::out_of_range("lingo::string_to_int");

  return static_cast<T>(n);
}


// Returns the integer value of the null-terminated string str,
// which contains an integer representation in base b. If no
// conversion could be performed, or if str contains any
// characters that are not digits in base b, this throws an
// std::invalid_argument exception. If the integer is outside
// the range of the result type, this throws an
// std::out_of_range exception.
template<typename T, typename CharT>
inline T
string_to_int(CharT const* str, int b)
{
  return string_to_int<T>(str, str + std::char_traits<CharT>::length(str), b);
}


// Returns the integer value of the string str, which contains
// an integer representation in base b. If no conversion could
// be performed, or if str contains any characters that are not
// digits in base b, this throws an std::invalid_argument
// exception. If the integer is outside the range of the result
// type, this throws an std::out_of_range exception.
template<typename T, typename CharT>
inline T
string_to_int(std::basic_string<CharT> const& str, int b)
{
  return string_to_int<T>(str.begin(), str.end(), b);
}


// -------------------------------------------------------------------------- //
//                            String view

// A view of a string in a source file. A string view is
// represented as a pair of pointers into text owned by
// another object.
class String_view
{
public:
  String_view(char const* f, char const* l)
    : first(f), last(l)
  { }

  String_view(char const* s)
    : first(s), last(s + std::char_traits<char>::length(s))
  { }

  int size() const { return last - first; }
  std::size_t hash() const;
  std::string str() const { return {first, last}; }

  char operator[](int n) { return *(first + n); }

  // Iterators
  char const* begin() { return first; }
  char const* end() { return last; }

  char const* begin() const { return first; }
  char const* end() const { return last; }

private:
  char const* first;
  char const* last;
};


// Reference equality
inline bool
operator==(String_view const& a, String_view const& b)
{
  return a.begin() == b.begin() && a.end() == b.end();
}


inline bool
operator!=(String_view const& a, String_view const& b)
{
  return !(a == b);
}


// Streaming
std::ostream& operator<<(std::ostream&, String_view);


// Create a view of the underlying string.
inline String_view
make_view(String const& s)
{
  return {s.c_str(), s.c_str() + s.size()};
}


// This hash function is used by the symbol table to hash
// string views.
struct String_view_hash
{
  std::size_t
  operator()(String_view s) const
  {
    return s.hash();
  }
};


// Value equality for strings. This is used to compare entries
// in the symbol table when collisions occur.
struct String_view_eq
{
  bool
  operator()(String_view a, String_view b) const
  {
    return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin());
  }
};


// -------------------------------------------------------------------------- //
//                          String builder


// The string builder facilitates the caching of characters
// needed to form a string during lexical analysis.
//
// TODO: Allow strings greater than 128 bytes in length.
// Basically, this entails implementing the small-string
// optimization.
class String_builder
{
  static constexpr int init_size = 128;
public:
  String_builder();

  String str() const;
  String take();

  void put(char c);
  void put(char const*);
  void put(char const*, int n);
  void put(char const*, char const*);

  void clear();

private:
  char buf_[init_size];
  int  len_;
};


inline
String_builder::String_builder()
  : len_(0)
{
  std::fill(buf_, buf_ + init_size, 0);
}


// FIXME: This should return a string view, but until
// we can efficiently hash-compare a string view against
// a string, it won't matter.
inline String
String_builder::str() const
{
  return String(buf_, buf_ + len_);
}


// Return the string in the builder and then reset it.
inline String
String_builder::take()
{
  String s = str();
  clear();
  return s;
}


inline void
String_builder::put(char c)
{
  if (len_ == init_size)
    throw std::runtime_error("string builder overflow");
  buf_[len_++] = c;
}


inline void
String_builder::put(char const* s)
{
  put(s, std::char_traits<char>::length(s));
}


inline void
String_builder::put(char const* s, int n)
{
  if (len_ + n >= init_size)
    throw std::runtime_error("string builder overflow");
  std::copy_n(s, n, buf_ + len_);
  len_ += n;
}


inline void
String_builder::put(char const* first, char const* last)
{
  put(first, last - first);
}


// Reset the string builder so that it's content
// is empty.
inline void
String_builder::clear()
{
  std::fill(buf_, buf_ + init_size, 0);
  len_ = 0;
}


// -------------------------------------------------------------------------- //
//                            String buffer

// The string buffer class provides implements a simple
// string-based buffer for a stream. The string must not
// have null characters.
class Stringbuf
{
public:
  Stringbuf() = default;
  Stringbuf(String const&);
  Stringbuf(std::istream& is);

  void assign(std::istream& is);

  char const* begin() const;
  char const* end() const;

private:
  String   buf_;
};


// Initialize the sting buffer from a pre-existing
// string. Note that this copies the string.
inline
Stringbuf::Stringbuf(String const& s)
  : buf_(s)
{ }


// Returns an iterator to the beginning of the string
// buffer.
inline char const*
Stringbuf::begin() const
{
  return buf_.c_str();
}


// Returns an iterator past the end of the string buffer.
inline char const*
Stringbuf::end() const
{
  return begin() + buf_.size();
}


} // namespace lingo

#endif
