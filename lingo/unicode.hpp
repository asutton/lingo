// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#ifndef LINGO_UNICODE_HPP
#define LINGO_UNICODE_HPP

#include <climits>
#include <cstdint>
#include <string>

// The minimum Unicode code point value.
#define UNICODE_MIN 0

// The maximum Unicode code point value.
#define UNICODE_MAX 0x10FFFF

#if CHAR_BIT == 8
  #define CHAR8_T_TYPE unsigned char
#else
  #define CHAR8_T_TYPE std::uint_least8_t
#endif

// The type for an UTF-8 code unit.
typedef CHAR8_T_TYPE char8_t;

namespace lingo
{

// -------------------------------------------------------------------------- //
//                                  Types

// UTF-8 encoded string type.
typedef std::basic_string<char8_t> u8string;

// UTF-16 encoded string type.
typedef std::basic_string<char16_t> u16string;

// UTF-32 encoded string type.
typedef std::basic_string<char32_t> u32string;


// -------------------------------------------------------------------------- //
//                                Algorithms

// Unescape a single character escape sequence.
template<typename UCharT, typename CharT>
UCharT to_unescaped(const CharT* str, CharT** str_end);


// -------------------------------------------------------------------------- //
//                         Character Set Conversion

// String encodings.
enum Encoding
{
  ASCII_encoding                 = 0,
  UTF8_encoding                  = 1,
  UTF16_encoding                 = 2,
  UTF16_big_endian_encoding      = 3,
  UTF16_little_endian_encoding   = 4,
#if WORDS_BIGENDIAN
  UTF16_platform_endian_encoding = UTF16_big_endian_encoding,
  UTF16_opposite_endian_encoding = UTF16_little_endian_encoding,
#else
  UTF16_platform_endian_encoding = UTF16_little_endian_encoding,
  UTF16_opposite_endian_encoding = UTF16_big_endian_encoding,
#endif
  UTF32_encoding                 = 5,
  UTF32_big_endian_encoding      = 6,
  UTF32_little_endian_encoding   = 7,
#if WORDS_BIGENDIAN
  UTF32_platform_endian_encoding = UTF32_big_endian_encoding,
  UTF32_opposite_endian_encoding = UTF32_little_endian_encoding
#else
  UTF32_platform_endian_encoding = UTF32_little_endian_encoding,
  UTF32_opposite_endian_encoding = UTF32_big_endian_encoding
#endif
};


// Returns the name of the specified string encoding.
const char* get_encoding_name(Encoding code);


// Encapsulates conversion of character strings from one encoding to another.
class Character_set_converter
{
public:
  // Constants that indicate the status of a character set conversion operation.
  enum Result
  {
    ok,       // Conversion was completed with no error.
    partial,  // Conversion stopped due to incomplete character sequence or insufficient space in the output buffer.
    error     // Conversion stopped due to illegal byte sequence.
  };

  Character_set_converter() noexcept;
  Character_set_converter(Encoding fromcode, Encoding tocode);
  Character_set_converter(const char* fromcode, const char* tocode);
  Character_set_converter(const Character_set_converter&) = delete;
  Character_set_converter(Character_set_converter&& c) noexcept;
  ~Character_set_converter();

  Character_set_converter& operator=(const Character_set_converter&) = delete;
  Character_set_converter& operator=(Character_set_converter&& c) noexcept;

  bool is_open() const;
  void open(const char* fromcode, const char* tocode);
  void close();

  void reset();

  Result reset_bytes(char* to, char* to_end, char*& to_next);
  Result convert_bytes(const char* from, const char* from_end, const char*& from_next, char* to, char* to_end, char*& to_next);
  std::size_t converted_byte_length(const char* from, const char* from_end);

  template<typename TargetT>
  Result reset(TargetT* to, TargetT* to_end, TargetT*& to_next);

  template<typename TargetT, typename SourceT>
  Result convert(const SourceT* from, const SourceT* from_end, const SourceT*& from_next, TargetT* to, TargetT* to_end, TargetT*& to_next);

  template<typename TargetT, typename SourceT>
  std::size_t converted_length(const SourceT* from, const SourceT* from_end);

  template<typename TargetT, typename SourceT>
  std::basic_string<TargetT> convert(SourceT c);

  template<typename TargetT, typename SourceT>
  std::basic_string<TargetT> convert(const SourceT* str, typename std::basic_string<SourceT>::size_type n = std::basic_string<SourceT>::npos);

  template<typename TargetT, typename SourceT>
  std::basic_string<TargetT> convert(const std::basic_string<SourceT>& str);

  template<typename TargetT, typename InputIterator>
  std::basic_string<TargetT> convert(InputIterator first, InputIterator last);

  void swap(Character_set_converter& c) noexcept;

private:
  void* m_rep;
};


// Converts a Unicode code point to the specified encoding.
template<typename TargetT>
std::basic_string<TargetT> convert_UTF32(Encoding tocode, char32_t c);

// Converts a Unicode code point to the specified encoding.
template<typename TargetT>
std::basic_string<TargetT> convert_UTF32(const char* tocode, char32_t c);


} // namespace lingo

#include <lingo/unicode.ipp>

#endif
