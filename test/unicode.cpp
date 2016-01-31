// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "config.hpp"

#include <iostream>
#include <utility>

#include "lingo/assert.hpp"
#include "lingo/unicode.hpp"

struct unicode_put_proxy
{
  char32_t c;
};

inline unicode_put_proxy unicode_put(char32_t c)
{
  return {c};
}

inline std::ostream& operator<<(std::ostream& out, unicode_put_proxy p)
{
  const std::ios_base::fmtflags flags = out.flags();
  const char fill = out.fill();
  out << "U+";
  out.flags(std::ios_base::hex | std::ios_base::right | std::ios_base::uppercase);
  out.width(4);
  out.fill('0');
  out << static_cast<std::uint_least32_t>(p.c);
  out.flags(flags);
  out.fill(fill);
  return out;
}

int main()
{
  const std::pair<const char*, char32_t> valid_escape_sequences[] = {
    {"a", U'a'},
    {"\\\'", U'\''},
    {"\\\"", U'\"'},
    {"\\\?", U'\?'},
    {"\\\\", U'\\'},
    {"\\a", U'\a'},
    {"\\b", U'\b'},
    {"\\f", U'\f'},
    {"\\n", U'\n'},
    {"\\r", U'\r'},
    {"\\t", U'\t'},
    {"\\v", U'\v'},
    {"\\0", U'\0'},
    {"\\1", U'\1'},
    {"\\033", U'\033'},
    {"\\200", U'\200'},
    {"\\x7f", U'\x7f'},
    {"\\xFF", U'\xff'},
    {"\\u61", U'\u0061'},
    {"\\u00e9", U'\u00e9'},
    {"\\U0001f34c", U'\U0001f34c'},
    {"\\U1F4A9", U'\U0001f4a9'} // 💩
  };
  const char* invalid_escape_sequences[] = {
    "\\",
    "\\$",
    "\\8",
    "\\x",
    "\\xN",
    "\\u",
    "\\uNNNN",
    "\\U",
    "\\UNNNNNNNN"
  };

  for (const auto& valid_escape_sequence : valid_escape_sequences) {
    try {
      const char* str = valid_escape_sequence.first;
      const std::size_t n = std::char_traits<char>::length(str);
      const char32_t expected_value = valid_escape_sequence.second;

      char* end_ptr;
      char32_t value = lingo::to_unescaped<char32_t>(str, &end_ptr);

      std::cout << str << " = " << unicode_put(value) << '\n';

      lingo_assert(value == expected_value);
      lingo_assert(end_ptr == str + n);
    }
    catch (...) {
      lingo_unreachable("lingo::to_unescaped() unexpectedly failed.");
    }
  }

  for (const char* invalid_escape_sequence : invalid_escape_sequences) {
    try {
      const char* str = invalid_escape_sequence;

      char* end_ptr;
      lingo::to_unescaped<char32_t>(str, &end_ptr);

      lingo_unreachable("lingo::to_unescaped() unexpectedly succeeded.");
    }
    catch (...) {}
  }

  try {
    const char* text = "\0";
    char* end_ptr;
    auto value = lingo::to_unescaped<char>(text, &end_ptr);
    lingo_assert(value == '\0');
    lingo_assert(end_ptr == text + 1);
  }
  catch (...) {
    lingo_unreachable("lingo::to_unescaped() unexpectedly failed.");
  }

  try {
    const signed char text[] = {-1, '\0'};
    auto value = lingo::to_unescaped<signed char, signed char>(text, nullptr);
    lingo_assert(value == -1);
  }
  catch (...) {
    lingo_unreachable("lingo::to_unescaped() unexpectedly failed.");
  }

  try {
    const signed char text[] = {-1, '\0'};
    lingo::to_unescaped<unsigned char, signed char>(text, nullptr);
    lingo_unreachable("lingo::to_unescaped() unexpectedly succeeded.");
  }
  catch (const std::out_of_range&) {}

  try {
    const char16_t* text = u"π"; // U+03C0 (Greek Small Letter Pi)
    lingo::to_unescaped<std::uint8_t, char16_t>(text, nullptr);
    lingo_unreachable("lingo::to_unescaped() unexpectedly succeeded.");
  }
  catch (const std::out_of_range&) {}

  try {
    const char* text = "\\u0080";
    lingo::to_unescaped<std::int8_t, char>(text, nullptr);
    lingo_unreachable("lingo::to_unescaped() unexpectedly succeeded.");
  }
  catch (const std::out_of_range&) {}

  try {
    const char* text = "\\u0080";
    auto value = lingo::to_unescaped<std::uint8_t, char>(text, nullptr);
    lingo_assert(value == UINT8_C(0x80));
  }
  catch (...) {
    lingo_unreachable("lingo::to_unescaped() unexpectedly failed.");
  }

  return 0;
}
