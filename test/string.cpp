// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "config.hpp"

#include "lingo/string.hpp"

int main()
{
  lingo_assert(!lingo::is_digit('2', 2));
  lingo_assert(lingo::is_digit('9', 10));
  lingo_assert(lingo::digit_value('1', 8) == 1);
  lingo_assert(lingo::digit_value('a', 16) == 10);
  lingo_assert(lingo::digit_value('a', 10) == -1);

  try {
    lingo::string_to_int<int>("", 10);
    lingo_unreachable("lingo::string_to_int() unexpectedly succeeded.");
  }
  catch (const std::invalid_argument& exception) {}

  try {
    auto value = lingo::string_to_int<int>("00", 10);
    lingo_assert(value == 0);
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
    auto value = lingo::string_to_int<char>("-128", 10);
    lingo_assert(value == -128);
  }
  catch (...) {
    lingo_unreachable("lingo::string_to_int() unexpectedly failed.");
  }

  try {
    lingo::string_to_int<char>("-129", 10);
    lingo_unreachable("lingo::string_to_int() unexpectedly succeeded.");
  }
  catch (const std::out_of_range& exception) {}

  try {
    lingo::string_to_int<char>("FF", 10);
    lingo_unreachable("lingo::string_to_int() unexpectedly succeeded.");
  }
  catch (const std::invalid_argument& exception) {}

  try {
    lingo::string_to_int<char>("FF", 16);
    lingo_unreachable("lingo::string_to_int() unexpectedly succeeded.");
  }
  catch (const std::out_of_range& exception) {}

  try {
    auto value = lingo::string_to_int<unsigned char>("FF", 16);
    lingo_assert(value == 0xFF);
  }
  catch (...) {
    lingo_unreachable("lingo::string_to_int() unexpectedly failed.");
  }

  return 0;
}
