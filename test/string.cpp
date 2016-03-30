// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "config.hpp"

#include "lingo/string.hpp"

int main()
{
  lingo_assert(!lingo::is_digit('2', 2));
  lingo_assert(!lingo::is_digit('f', 10));
  lingo_assert(lingo::is_digit('9', 10));
  lingo_assert(lingo::is_digit('a', 11));
  lingo_assert(lingo::digit_value('1', 3) == 1);
  lingo_assert(lingo::digit_value('a', 16) == 10);
  lingo_assert(lingo::digit_value('a', 10) == -1);

  try {
    lingo::string_to_int<int>("", 10);
    lingo_unreachable("lingo::string_to_int() unexpectedly succeeded.");
  }
  catch (const std::invalid_argument& exception) {}

  try {
    auto value = lingo::string_to_int<int>("001", 10);
    lingo_assert(value == 1);
  }
  catch (...) {
    lingo_unreachable("lingo::string_to_int() unexpectedly failed.");
  }

  try {
    auto value = lingo::string_to_int<int>("+1", 10);
    lingo_assert(value == 1);
  }
  catch (...) {
    lingo_unreachable("lingo::string_to_int() unexpectedly failed.");
  }

  try {
    auto value = lingo::string_to_int<int>("-1", 10);
    lingo_assert(value == -1);
  }
  catch (...) {
    lingo_unreachable("lingo::string_to_int() unexpectedly failed.");
  }

  try {
    lingo::string_to_int<unsigned int>("-1", 10);
    lingo_unreachable("lingo::string_to_int() unexpectedly succeeded.");
  }
  catch (const std::out_of_range& exception) {}

  try {
    auto value = lingo::string_to_int<std::int8_t>("-128", 10);
    lingo_assert(value == INT8_C(-128));
  }
  catch (...) {
    lingo_unreachable("lingo::string_to_int() unexpectedly failed.");
  }

  try {
    lingo::string_to_int<std::int8_t>("-129", 10);
    lingo_unreachable("lingo::string_to_int() unexpectedly succeeded.");
  }
  catch (const std::out_of_range& exception) {}

  try {
    lingo::string_to_int<std::int8_t>("FF", 16);
    lingo_unreachable("lingo::string_to_int() unexpectedly succeeded.");
  }
  catch (const std::out_of_range& exception) {}

  try {
    auto value = lingo::string_to_int<std::uint8_t>("FF", 16);
    lingo_assert(value == UINT8_C(0xFF));
  }
  catch (...) {
    lingo_unreachable("lingo::string_to_int() unexpectedly failed.");
  }

  try {
    auto value = lingo::string_to_int<std::intmax_t>("9223372036854775807", 10);
    lingo_assert(value == INTMAX_C(9223372036854775807));
  }
  catch (...) {
    lingo_unreachable("lingo::string_to_int() unexpectedly failed.");
  }

  try {
    lingo::string_to_int<std::intmax_t>("9223372036854775808", 10);
    lingo_unreachable("lingo::string_to_int() unexpectedly succeeded.");
  }
  catch (const std::out_of_range& exception) {}

  try {
    lingo::string_to_int<std::uintmax_t>("-1", 10);
    lingo_unreachable("lingo::string_to_int() unexpectedly succeeded.");
  }
  catch (const std::out_of_range& exception) {}

  try {
    auto value = lingo::string_to_int<std::uintmax_t>("FFFFFFFFFFFFFFFF", 16);
    lingo_assert(value = UINTMAX_C(0xFFFFFFFFFFFFFFFF));
  }
  catch (...) {
    lingo_unreachable("lingo::string_to_int() unexpectedly failed.");
  }

  try {
    lingo::string_to_int<std::uintmax_t>("10000000000000000", 16);
    lingo_unreachable("lingo::string_to_int() unexpectedly succeeded.");
  }
  catch (const std::out_of_range& exception) {}

  return 0;
}
