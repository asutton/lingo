// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "io.hpp"

#include <algorithm>
#include <iostream>


namespace lingo
{


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
  os << "\033[0m";
}


} // namespace lingo
