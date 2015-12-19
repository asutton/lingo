// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "config.hpp"

#include "lingo/io.hpp"

#include <unistd.h>

#include <algorithm>
#include <iostream>


namespace lingo
{


namespace
{

bool color_output = false;
bool color_error = false;

} // namespace


// Determine if colors should be enabled on the
// output and error devices.
void
init_colors()
{
  color_output = isatty(1);
  color_error = isatty(2);
}


// Return true if color should be used for the
// given stream.
bool
enable_color(std::ostream& os)
{
  if (&os == &std::cout)
    return color_output;
  if (&os == &std::cerr)
    return color_error;
  return false;
}


char const*
get_weight(Text_weight w)
{
  if (w == bright_text)
    return "1";
  if (w == dim_text)
    return "2";
  return nullptr;
}


char const*
get_foreground_color(Text_color c)
{
  static char const* fg[] {
    "30", // Black
    "31", // Red
    "32", // Green
    "33", // Yellow
    "34", // Blue
    "35", // Magenta
    "36", // Cyan
    "37", // White
  };
  if (c > 0)
    return fg[c - 1];
  else
    return nullptr;
}


// TODO: Implement me.
char const*
get_effects(Text_effects)
{
  return nullptr;
}


void
start_font(std::ostream& os, Font_spec font)
{
  if (!enable_color(os))
    return;

  char const* codes[3] {
    get_weight(font.weight),
    get_foreground_color(font.color),
    get_effects(font.effects)
  };
  char const** last = std::remove(codes, codes + 3, nullptr);
  char const** first = codes;

  os << "\033[";
  while (first != last) {
    os << *first;
    if (first + 1 != last)
      os << ';';
    ++first;
  }
  os << 'm';
}


void
end_font(std::ostream& os)
{
  if (!enable_color(os))
    return;

  os << "\033[0m";
}


} // namespace lingo
