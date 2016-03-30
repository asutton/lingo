// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "config.hpp"

#include <cstring>

#include "lingo/assert.hpp"
#include "lingo/unicode.hpp"

int main()
{
  // Sample text is "I can eat glass, and it does not hurt me." in
  // Vietnamese from http://www.columbia.edu/~fdc/utf8/.
  const std::string in_str = u8"Tôi có thể ăn thủy tinh mà không hại gì.";
  const std::size_t in_chars = in_str.size();
  const std::size_t in_bytes = in_chars * sizeof(char);
  const char* in_buf = reinterpret_cast<const char*>(in_str.data());

  const char* target_encoding_names[] = {
    "UTF-16",
    "UTF-16BE",
    "UTF-16LE",
    "UTF-32",
    "UTF-32BE",
    "UTF-32LE"
  };

  for (const char* target_encoding_name : target_encoding_names) {
    try {
      lingo::Character_set_converter converter("UTF-8", target_encoding_name);

      const std::size_t out_bytes = converter.converted_byte_length(in_buf, in_buf + in_bytes);
      converter.reset();

      const auto out_str1 = converter.convert<char>(in_str);
      converter.reset();
      const auto out_str2 = converter.convert<char16_t>(in_str);
      converter.reset();
      const auto out_str3 = converter.convert<char32_t>(in_str);
      converter.reset();

      lingo_assert(std::memcmp(out_str1.data(), out_str2.data(), out_bytes) == 0);
      lingo_assert(std::memcmp(out_str2.data(), out_str3.data(), out_bytes) == 0);
    }
    catch (...) {
      lingo_unreachable("Conversion from UTF-8 to {} unexpectedly failed.", target_encoding_name);
    }
  }

  return 0;
}
