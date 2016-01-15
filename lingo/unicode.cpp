// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "config.hpp"

#include "lingo/unicode.hpp"

#include <cerrno>
#include <stdexcept>
#include <system_error>
#include <utility>

#include <iconv.h>

#define kBufferSize 4096

namespace lingo
{

// -------------------------------------------------------------------------- //
//                         Character set conversion

namespace
{

const char* encoding_names[] = {
  "ASCII",
  "UTF-8",
  "UTF-16",
  "UTF-16BE",
  "UTF-16LE",
  "UTF-32",
  "UTF-32BE",
  "UTF-32LE"
};

} // namespace


const char*
get_encoding_name(Encoding code)
{
  static const std::size_t n_encodings = std::end(encoding_names) - std::begin(encoding_names);

  if (code < 0 || code >= n_encodings)
    throw std::invalid_argument("lingo::get_encoding_name");

  return encoding_names[code];
}


Character_set_converter::Character_set_converter() noexcept
  : m_rep(reinterpret_cast<void*>(-1))
{}


Character_set_converter::Character_set_converter(Encoding fromcode, Encoding tocode)
  : Character_set_converter()
{
  open(get_encoding_name(fromcode), get_encoding_name(tocode));
}


Character_set_converter::Character_set_converter(const char* fromcode, const char* tocode)
  : Character_set_converter()
{
  open(fromcode, tocode);
}


Character_set_converter::Character_set_converter(Character_set_converter&& c) noexcept
  : m_rep(std::exchange(c.m_rep, reinterpret_cast<void*>(-1)))
{}


Character_set_converter::~Character_set_converter()
{
  if (is_open())
    iconv_close(reinterpret_cast<iconv_t>(m_rep));
}


Character_set_converter&
Character_set_converter::operator=(Character_set_converter&& c) noexcept
{
  m_rep = std::exchange(c.m_rep, reinterpret_cast<void*>(-1));
  return *this;
}


bool
Character_set_converter::is_open() const
{
  return m_rep != reinterpret_cast<void*>(-1);
}


void
Character_set_converter::open(const char* fromcode, const char* tocode)
{
  if (!is_open()) {
    iconv_t cd = iconv_open(tocode, fromcode);
    if (cd == iconv_t(-1))
      throw std::system_error(errno, std::generic_category());
    m_rep = reinterpret_cast<void*>(cd);
  }
}


void
Character_set_converter::close()
{
  if (is_open()) {
    if (iconv_close(reinterpret_cast<iconv_t>(m_rep)) == -1)
      throw std::system_error(errno, std::generic_category());
    m_rep = reinterpret_cast<void*>(-1);
  }
}


void
Character_set_converter::reset()
{
  std::size_t n_conv = iconv(reinterpret_cast<iconv_t>(m_rep), nullptr, nullptr, nullptr, nullptr);
  if (n_conv == std::size_t(-1))
    throw std::system_error(errno, std::generic_category());
}


typename Character_set_converter::Result
Character_set_converter::reset_bytes(char* to, char* to_end, char*& to_next)
{
  lingo_assert(to <= to_end);

  Result result = ok;

  char* out_buf = to;
  std::size_t out_bytes = to_end - to;

  std::size_t n_conv = iconv(reinterpret_cast<iconv_t>(m_rep), nullptr, nullptr, &out_buf, &out_bytes);
  to_next = out_buf;

  if (n_conv == std::size_t(-1)) {
    switch (errno) {
      case E2BIG:
        result = partial;
        break;
      default:
        throw std::system_error(errno, std::generic_category());
    }
  }

  return result;
}


typename Character_set_converter::Result
Character_set_converter::convert_bytes(const char* from, const char* from_end, const char*& from_next, char* to, char* to_end, char*& to_next)
{
  lingo_assert(from <= from_end);
  lingo_assert(to <= to_end);

  Result result = ok;

  ICONV_CONST char* in_buf = const_cast<ICONV_CONST char*>(from);
  char* out_buf = to;
  std::size_t in_bytes = from_end - from;
  std::size_t out_bytes = to_end - to;

  std::size_t n_conv = iconv(reinterpret_cast<iconv_t>(m_rep), &in_buf, &in_bytes, &out_buf, &out_bytes);
  from_next = const_cast<const char*>(in_buf);
  to_next = out_buf;

  if (n_conv == std::size_t(-1)) {
    switch (errno) {
      case E2BIG:
      case EINVAL:
        result = partial;
        break;
      case EILSEQ:
        result = error;
        break;
      default:
        throw std::system_error(errno, std::generic_category());
    }
  }

  return result;
}


std::size_t
Character_set_converter::converted_byte_length(const char* from, const char* from_end)
{
  lingo_assert(from <= from_end);

  std::size_t result = 0;

  char temp_buf[kBufferSize];

  ICONV_CONST char* in_buf = const_cast<ICONV_CONST char*>(from);
  std::size_t in_bytes = from_end - from;

  while (in_bytes > 0) {
    char* out_buf = temp_buf;
    std::size_t out_bytes = kBufferSize;

    std::size_t n_conv = iconv(reinterpret_cast<iconv_t>(m_rep), &in_buf, &in_bytes, &out_buf, &out_bytes);
    result += out_buf - temp_buf;

    if (n_conv == std::size_t(-1)) {
      switch (errno) {
        case E2BIG:
          break;
        case EINVAL:
          errno = EILSEQ;
          return result;
        case EILSEQ:
          return result;
        default:
          throw std::system_error(errno, std::generic_category());
      }
    }
  }

  // Also take into account the termination/reset character sequence for when
  // [from, from_end) ends with an incomplete character or shift sequence.
  char* out_buf = temp_buf;
  std::size_t out_bytes = kBufferSize;

  std::size_t n_conv = iconv(reinterpret_cast<iconv_t>(m_rep), nullptr, nullptr, &out_buf, &out_bytes);
  result += out_buf - temp_buf;

  if (n_conv == std::size_t(-1)) {
    switch (errno) {
      case E2BIG:
        lingo_unreachable("Temporary output buffer is not large enough to contain termination/reset character sequence.");
      default:
        throw std::system_error(errno, std::generic_category());
    }
  }

  return result;
}


void
Character_set_converter::swap(Character_set_converter& c) noexcept
{
  std::swap(m_rep, c.m_rep);
}


} // namespace lingo
