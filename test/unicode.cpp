// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include <cstdint>
#include <iostream>

#include <lingo/assert.hpp>
#include <lingo/unicode.hpp>

#define LOG_EXPR(x) std::cout << #x << " = " << (x) << std::endl

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
  out << "U+";
  out.flags(std::ios_base::hex | std::ios_base::right | std::ios_base::uppercase);
  out.width(8);
  out.fill('0');
  out << static_cast<std::uint32_t>(p.c);
  out.flags(flags);
  out.width(0);
  out.fill(' ');
  return out;
}

int main()
{
  const char* strings[] = {
    "",
    "H",
    "\\\"",
    "\\0",
    "\\1",
    "\\2",
    "\\033",
    "\\U0001f34c",
  };

  for (const char* string : strings) {
    char* end_ptr;
    try {
      std::cout << string << " = "
                << unicode_put(lingo::unescape(string, &end_ptr))
                << std::endl;
    }
    catch (const std::invalid_argument& exception) {
      std::cerr << "Invalid argument." << std::endl;
    }
    catch (const std::out_of_range& exception) {
      std::cerr << "Argument is out of range." << std::endl;
    }
    LOG_EXPR(end_ptr - string);
  }

  // Sample text is "I can eat glass, and it does not hurt me" in
  // traditional Chinese from http://www.columbia.edu/~fdc/utf8/.
  const std::string text1(u8"我能吞下玻璃而不傷身體。");

  std::u16string text2 = lingo::convert_UTF8_to_UTF16(text1);
  lingo_assert(text2.size() == 12);

  std::u32string text3 = lingo::convert_UTF8_to_UTF32(text1);
  lingo_assert(text3.size() == 12);

  return 0;
}
