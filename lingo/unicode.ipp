// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#ifndef LINGO_UNICODE_IPP
#define LINGO_UNICODE_IPP

#include <lingo/string.hpp>

#include <memory>

#define DIV_CEIL(a, b) (((a) + (b) - 1) / (b))

namespace lingo
{

// -------------------------------------------------------------------------- //
//                                Algorithms

template<typename UCharT, typename CharT>
UCharT
to_unescaped(const CharT* str, CharT** str_end)
{
  typedef typename std::make_unsigned<UCharT>::type unsigned_unescaped_char_type;

  const CharT* first = str;
  const CharT* last = str + std::char_traits<CharT>::length(str);

  UCharT result = UCharT();

  if (first != last) {
    if (*first == static_cast<CharT>('\\') && first + 1 < last) {
      ++first;
      switch (*first) {
        case static_cast<CharT>('\''):
          result = static_cast<UCharT>('\'');
          break;
        case static_cast<CharT>('\"'):
          result = static_cast<UCharT>('\"');
          break;
        case static_cast<CharT>('\?'):
          result = static_cast<UCharT>('\?');
          break;
        case static_cast<CharT>('\\'):
          result = static_cast<UCharT>('\\');
          break;
        case static_cast<CharT>('a'):
          result = static_cast<UCharT>('\a');
          break;
        case static_cast<CharT>('b'):
          result = static_cast<UCharT>('\b');
          break;
        case static_cast<CharT>('f'):
          result = static_cast<UCharT>('\f');
          break;
        case static_cast<CharT>('n'):
          result = static_cast<UCharT>('\n');
          break;
        case static_cast<CharT>('r'):
          result = static_cast<UCharT>('\r');
          break;
        case static_cast<CharT>('t'):
          result = static_cast<UCharT>('\t');
          break;
        case static_cast<CharT>('v'):
          result = static_cast<UCharT>('\v');
          break;
        case static_cast<CharT>('x'): {
          // Parse hexadecimal escape sequence.
          CharT digits[2];
          int digit_count = 0;
          for (int j = 1; j <= 2; j++) {
            if (first + j >= last || !is_hexadecimal_digit(*(first + j)))
              break;
            digits[j - 1] = *(first + j);
            digit_count++;
          }
          result = static_cast<UCharT>(string_to_int<unsigned_unescaped_char_type>(digits, digits + digit_count, 16));
          first += digit_count;
          break;
        }
        case static_cast<CharT>('u'): {
          // Parse 16-bit Unicode code point escape sequence.
          CharT digits[4];
          int digit_count = 0;
          for (int j = 1; j <= 4; j++) {
            if (first + j >= last || !is_hexadecimal_digit(*(first + j)))
              break;
            digits[j - 1] = *(first + j);
            digit_count++;
          }
          result = string_to_int<UCharT>(digits, digits + digit_count, 16);
          first += digit_count;
          break;
        }
        case static_cast<CharT>('U'): {
          // Parse 32-bit Unicode code point escape sequence.
          CharT digits[8];
          int digit_count = 0;
          for (int j = 1; j <= 8; j++) {
            if (first + j >= last || !is_hexadecimal_digit(*(first + j)))
              break;
            digits[j - 1] = *(first + j);
            digit_count++;
          }
          result = string_to_int<UCharT>(digits, digits + digit_count, 16);
          first += digit_count;
          // Verify that code point is valid.
          if (result > UNICODE_MAX)
            throw std::out_of_range("lingo::to_unescaped");
          break;
        }
        default: {
          // Parse octal escape sequence.
          CharT digits[3];
          int digit_count = 0;
          for (int j = 1; j <= 3; j++) {
            if (first + j - 1 >= last || !is_octal_digit(*(first + j - 1)))
              break;
            digits[j - 1] = *(first + j - 1);
            digit_count++;
          }
          if (digit_count) {
            result = static_cast<UCharT>(string_to_int<unsigned_unescaped_char_type>(digits, digits + digit_count, 8));
            first += digit_count - 1;
          }
          else
            // Strip backslash from single escaped character.
            goto no_escape_sequence;
          break;
        }
      }
    }
    else {
no_escape_sequence:
      const typename std::common_type<UCharT, CharT>::type c = *first;
      if (c < std::numeric_limits<UCharT>::min() || c > std::numeric_limits<UCharT>::max())
        throw std::out_of_range("lingo::to_unescaped");
      result = static_cast<UCharT>(*first);
    }
    ++first;
  }

  if (str_end)
    *str_end = const_cast<CharT*>(first);

  return result;
}


// -------------------------------------------------------------------------- //
//                         Character set conversion

template<typename TargetT>
typename Character_set_converter::Result
Character_set_converter::reset(TargetT* to, TargetT* to_end, TargetT*& to_next)
{
  return reset_bytes(reinterpret_cast<char*>(to), reinterpret_cast<char*>(to_end), reinterpret_cast<char*&>(to_next));
}


template<typename TargetT, typename SourceT>
typename Character_set_converter::Result
Character_set_converter::convert(const SourceT* from, const SourceT* from_end, const SourceT*& from_next, TargetT* to, TargetT* to_end, TargetT*& to_next)
{
  return convert_bytes(reinterpret_cast<const char*>(from), reinterpret_cast<const char*>(from_end), reinterpret_cast<const char*&>(from_next), reinterpret_cast<char*>(to), reinterpret_cast<char*>(to_end), reinterpret_cast<char*&>(to_next));
}


template<typename TargetT, typename SourceT>
std::size_t
Character_set_converter::converted_length(const SourceT* from, const SourceT* from_end)
{
  return DIV_CEIL(converted_byte_length(reinterpret_cast<const char*>(from), reinterpret_cast<const char*>(from_end)), sizeof(TargetT));
}


template<typename TargetT, typename SourceT>
inline std::basic_string<TargetT>
Character_set_converter::convert(SourceT c)
{
  return convert<TargetT>(reinterpret_cast<SourceT*>(&c), 1);
}


template<typename TargetT, typename SourceT>
std::basic_string<TargetT>
Character_set_converter::convert(const SourceT* str, typename std::basic_string<SourceT>::size_type n)
{
  if (n == std::basic_string<SourceT>::npos)
    n = std::char_traits<SourceT>::length(str);

  const std::size_t in_buf_size = n;
  const SourceT* in_buf = str;
  const SourceT* in_buf_next;

  const std::size_t out_buf_size = converted_length<TargetT>(in_buf, in_buf + in_buf_size);
  // Wrap out_buf in a std::unique_ptr for exception safety.
  std::unique_ptr<TargetT[]> out_buf_ptr(new TargetT[out_buf_size]);
  TargetT* out_buf = out_buf_ptr.get();
  TargetT* out_buf_next;

  // Reset to initial shift state first.
  reset();
  // Perform character set conversion.
  Result conversion_result = convert(in_buf, in_buf + in_buf_size, in_buf_next, out_buf, out_buf + out_buf_size, out_buf_next);

  switch (conversion_result) {
    case ok:
      break;
    case partial:
      // Write the termination/reset character sequence.
      conversion_result = reset(out_buf_next, out_buf + out_buf_size, out_buf_next);
      lingo_assert(conversion_result == ok);
      break;
    case error:
      throw std::invalid_argument("lingo::Character_set_converter::convert");
  }

  std::basic_string<TargetT> result;
  result.reserve(out_buf_size);
  result.assign(reinterpret_cast<TargetT*>(out_buf), out_buf_size);

  return result;
}


template<typename TargetT, typename SourceT>
inline std::basic_string<TargetT>
Character_set_converter::convert(const std::basic_string<SourceT>& str)
{
  return convert<TargetT>(str.data(), str.size());
}


template<typename TargetT, typename InputIterator>
inline std::basic_string<TargetT>
Character_set_converter::convert(InputIterator first, InputIterator last)
{
  typedef typename std::iterator_traits<InputIterator>::value_type source_type;

  std::basic_string<source_type> str;

  while (first != last)
    str.push_back(*first++);

  return convert<TargetT>(str);
}


template<typename TargetT>
inline std::basic_string<TargetT>
convert_UTF32(Encoding tocode, char32_t c)
{
  Character_set_converter converter(UTF32_platform_endian_encoding, tocode);
  return converter.convert<TargetT>(c);
}


template<typename TargetT>
inline std::basic_string<TargetT>
convert_UTF32(const char* tocode, char32_t c)
{
  Character_set_converter converter(get_encoding_name(UTF32_platform_endian_encoding), tocode);
  return converter.convert<TargetT>(c);
}


} // namespace lingo

#endif
