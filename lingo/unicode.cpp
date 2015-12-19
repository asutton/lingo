// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "config.hpp"

#include "lingo/unicode.hpp"
#include "lingo/string.hpp"

#include <cerrno>
#include <stdexcept>

#include <iconv.h>

#define kUTF8Encoding "UTF-8"
#if WORDS_BIGENDIAN
  #define kUTF16Encoding "UTF-16BE"
  #define kUTF32Encoding "UTF-32BE"
#else
  #define kUTF16Encoding "UTF-16LE"
  #define kUTF32Encoding "UTF-32LE"
#endif

namespace lingo
{

  char32_t
  unescape(const char* str, char** str_end)
  {
    const std::size_t len = std::char_traits<char>::length(str);
    const char* first = str;
    const char* last = str + len;

    char32_t result = U'\0';

    if (first != last) {
      if (*first == '\\' && first + 1 < last) {
        ++first;
        switch (*first) {
          case '\'':
            result = '\'';
            break;
          case '\"':
            result = '\"';
            break;
          case '\?':
            result = '\?';
            break;
          case '\\':
            result = '\\';
            break;
          case 'a':
            result = '\a';
            break;
          case 'b':
            result = '\b';
            break;
          case 'f':
            result = '\f';
            break;
          case 'n':
            result = '\n';
            break;
          case 'r':
            result = '\r';
            break;
          case 't':
            result = '\t';
            break;
          case 'v':
            result = '\v';
            break;
          case 'x': {
            char digits[2];
            int digit_count = 0;
            for (int j = 1; j <= 2; j++) {
              if (first + j >= last || !is_hexadecimal_digit(*(first + j)))
                break;
              digits[j - 1] = *(first + j);
              digit_count++;
            }
            if (digit_count == 0)
              throw std::invalid_argument("invalid hexadecimal escape sequence");
            result = string_to_int<char32_t>(digits, digits + digit_count, 16);
            first += digit_count;
            break;
          }
          case 'u': {
            char digits[4];
            int digit_count = 0;
            for (int j = 1; j <= 4; j++) {
              if (first + j >= last || !is_hexadecimal_digit(*(first + j)))
                break;
              digits[j - 1] = *(first + j);
              digit_count++;
            }
            if (digit_count == 0)
              throw std::invalid_argument("invalid UTF-16 escape sequence");
            result = string_to_int<char32_t>(digits, digits + digit_count, 16);
            first += digit_count;
            break;
          }
          case 'U': {
            char digits[8];
            int digit_count = 0;
            for (int j = 1; j <= 8; j++) {
              if (first + j >= last || !is_hexadecimal_digit(*(first + j)))
                break;
              digits[j - 1] = *(first + j);
              digit_count++;
            }
            if (digit_count == 0)
              throw std::invalid_argument("invalid UTF-32 escape sequence");
            result = string_to_int<char32_t>(digits, digits + digit_count, 16);
            first += digit_count;
            break;
          }
          default: {
            char digits[3];
            int digit_count = 0;
            for (int j = 1; j <= 3; j++) {
              if (first + j - 1 >= last || !is_octal_digit(*(first + j - 1)))
                break;
              digits[j - 1] = *(first + j - 1);
              digit_count++;
            }
            if (digit_count) {
              result = string_to_int<char32_t>(digits, digits + digit_count, 8);
              first += digit_count - 1;
            }
            else
              result = *first;
            break;
          }
        }
      }
      else
        result = *first;
      ++first;
    }

    if (str_end)
      *str_end = const_cast<char*>(first);

    return result;
  }


  template<typename CharT>
  static std::basic_string<CharT>
  convert_from_UTF8(const char* tocode, const char* str, std::size_t n)
  {
    std::basic_string<CharT> result;

    errno = 0;

    // Open the conversion descriptor.
    iconv_t utf8_to_dest = iconv_open(tocode, kUTF8Encoding);

    if (errno == EINVAL)
      abort("The conversion from {} to {} is not supported by the implementation of iconv().", kUTF8Encoding, tocode);

    const std::size_t max_bytes = n * sizeof(CharT);
    ICONV_CONST char* in_buf = const_cast<ICONV_CONST char*>(str);
    char* out_buf = new char[max_bytes];
    std::size_t in_bytes = n;
    std::size_t out_bytes = max_bytes;

    // Perform the character set conversion.
    iconv(utf8_to_dest, &in_buf, &in_bytes, &out_buf, &out_bytes);

    if (errno == EILSEQ || errno == EINVAL)
      throw std::invalid_argument("lingo::convert_from_UTF8");

    const std::size_t out_size = (max_bytes - out_bytes) / sizeof(CharT);

    result.reserve(out_size);
    result.assign(reinterpret_cast<CharT*>(out_buf), out_size);

    // Clean up.
    delete[] out_buf;
    iconv_close(utf8_to_dest);

    return result;
  }


  u16string
  convert_UTF8_to_UTF16(const std::string& str)
  {
    const char* c_str = reinterpret_cast<const char*>(str.data());
    return convert_from_UTF8<char16_t>(kUTF16Encoding, c_str, str.size());
  }


  u16string
  convert_UTF8_to_UTF16(const u8string& str)
  {
    const char* c_str = reinterpret_cast<const char*>(str.data());
    return convert_from_UTF8<char16_t>(kUTF16Encoding, c_str, str.size());
  }


  u32string
  convert_UTF8_to_UTF32(const std::string& str)
  {
    const char* c_str = reinterpret_cast<const char*>(str.data());
    return convert_from_UTF8<char32_t>(kUTF32Encoding, c_str, str.size());
  }


  u32string
  convert_UTF8_to_UTF32(const u8string& str)
  {
    const char* c_str = reinterpret_cast<const char*>(str.data());
    return convert_from_UTF8<char32_t>(kUTF32Encoding, c_str, str.size());
  }

} // namespace lingo
