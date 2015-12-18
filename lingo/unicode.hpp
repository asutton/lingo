// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#ifndef LINGO_UNICODE_HPP
#define LINGO_UNICODE_HPP

#include <climits>
#include <cstdint>
#include <string>

#define UNICODE_MIN 0
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
  typedef std::basic_string<char8_t> u8string;
  typedef std::basic_string<char16_t> u16string;
  typedef std::basic_string<char32_t> u32string;

  char32_t unescape_at(const char* str, char** str_end);

  u16string convert_UTF8_to_UTF16(const std::string& str);
  u16string convert_UTF8_to_UTF16(const u8string& str);
  u32string convert_UTF8_to_UTF32(const std::string& str);
  u32string convert_UTF8_to_UTF32(const u8string& str);
} // namespace lingo

#endif
