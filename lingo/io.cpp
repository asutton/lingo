// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "lingo/io.hpp"

#include <unistd.h>

#include <algorithm>
#include <iostream>


namespace lingo
{

// Allocate a space for storing color flags.
int ios_color_flag = std::ios_base::xalloc();


// Determine if colors should be enabled on the
// output and error devices.
void
init_colors()
{
  if (isatty(1))
    std::cout.iword(ios_color_flag) = true;
  if (isatty(2))
    std::cerr.iword(ios_color_flag) = true;
}


// Return true if color should be used for the given stream.
//
// NOTE: I use the iword of a stream to store the color to indicate
// that a non-standard stream should use color, set iword to 1.
bool
supports_color(std::ostream& os)
{
  return os.iword(ios_color_flag);
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
  if (!supports_color(os))
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
  if (!supports_color(os))
    return;

  os << "\033[0m";
}


} // namespace lingo
