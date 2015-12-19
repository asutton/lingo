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
  #define CHAR8_T_TYPE std::uint8_t
#endif

// The type for an UTF-8 code unit.
typedef CHAR8_T_TYPE char8_t;

namespace lingo
{
  // UTF-8 encoded string type.
  typedef std::basic_string<char8_t> u8string;

  // UTF-16 encoded string type.
  typedef std::basic_string<char16_t> u16string;

  // UTF-32 encoded string type.
  typedef std::basic_string<char32_t> u32string;

  // Unescape a single character escape sequence.
  char32_t unescape(const char* str, char** str_end);

  u16string convert_UTF8_to_UTF16(const std::string& str);
  u16string convert_UTF8_to_UTF16(const u8string& str);
  u32string convert_UTF8_to_UTF32(const std::string& str);
  u32string convert_UTF8_to_UTF32(const u8string& str);
} // namespace lingo

#endif
