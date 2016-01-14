// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#ifndef LINGO_IO_HPP
#define LINGO_IO_HPP

// This module contains extensions for input and output.
//
// TODO: Support background colors and actually implement
// underlining and blinking.
//
// TODO: Support 256-color terminals.

#include <iosfwd>

namespace lingo
{


// Colors for text.
enum Text_color
{
  default_text, // Use the default color.
  black_text,
  red_text,
  green_text,
  yellow_text,
  blue_text,
  magenta_text,
  cyan_text,
  white_text,
};


// Text weight.
enum Text_weight
{
  normal_text,
  bright_text,
  dim_text,
};


// Additional formatting or effects.
enum Text_effects
{
  plain_text,
  underlined_text,
  blinking_text,
};


// Stores information about a font specification.
struct Font_spec
{
  Text_color   color;
  Text_weight  weight;
  Text_effects effects;
};


// TODO: Support background coloring?
template<typename T>
struct Styled_text
{
  Styled_text(T const& t, Text_color f, Text_weight w, Text_effects e)
    : txt(t), font{f, w, e}
  { }

  T const&  txt;
  Font_spec font;
};


template<typename T>
inline Styled_text<T>
styled(T const& t, Text_color c, Text_weight w, Text_effects e)
{
  return Styled_text<T>(t, c, w, e);
}


// -------------------------------------------------------------------------- //
// Normal colors


// Render t as normal text using the given color.
template<typename T>
inline Styled_text<T>
colored(T const& t, Text_color c)
{
  return styled(t, c, normal_text, plain_text);
}


// Render t as normal red text.
template<typename T>
inline Styled_text<T>
red(T const& t)
{
  return colored(t, red_text);
}


// Render t as normal blue text.
template<typename T>
inline Styled_text<T>
blue(T const& t)
{
  return colored(t, blue_text);
}


// Render t as normal green text.
template<typename T>
inline Styled_text<T>
green(T const& t)
{
  return colored(t, green_text);
}


// Render t as normal cyan text.
template<typename T>
inline Styled_text<T>
cyan(T const& t)
{
  return colored(t, cyan_text);
}


// Render t as normal magenta text.
template<typename T>
inline Styled_text<T>
magenta(T const& t)
{
  return colored(t, magenta_text);
}


// Render t as normal yellow text.
template<typename T>
inline Styled_text<T>
yellow(T const& t)
{
  return colored(t, yellow_text);
}


// Render t as normal white text.
template<typename T>
inline Styled_text<T>
white(T const& t)
{
  return colored(t, white_text);
}


// -------------------------------------------------------------------------- //
// Bright colors


// Render t as bright text.
template<typename T>
inline Styled_text<T>
bright(T const& t)
{
  return styled(t, default_text, bright_text, plain_text);
}


// Render t as bright text using the given color.
template<typename T>
inline Styled_text<T>
bright(T const& t, Text_color c)
{
  return styled(t, c, bright_text, plain_text);
}


// Render t as bright red text.
template<typename T>
inline Styled_text<T>
bright_red(T const& t)
{
  return bright(t, red_text);
}


// Render t as bright blue text.
template<typename T>
inline Styled_text<T>
bright_blue(T const& t)
{
  return bright(t, blue_text);
}


// Render t as bright green text.
template<typename T>
inline Styled_text<T>
bright_green(T const& t)
{
  return bright(t, green_text);
}


// Render t as bright cyan text.
template<typename T>
inline Styled_text<T>
bright_cyan(T const& t)
{
  return bright(t, cyan_text);
}


// Render t as bright magenta text.
template<typename T>
inline Styled_text<T>
bright_magenta(T const& t)
{
  return bright(t, magenta_text);
}


// Render t as bright yellow text.
template<typename T>
inline Styled_text<T>
bright_yellow(T const& t)
{
  return bright(t, yellow_text);
}


// Render t as bright white text.
template<typename T>
inline Styled_text<T>
bright_white(T const& t)
{
  return bright(t, white_text);
}


// -------------------------------------------------------------------------- //
// Dim colors


// Render t as bright text.
template<typename T>
inline Styled_text<T>
dim(T const& t)
{
  return styled(t, default_text, dim_text, plain_text);
}


// Render t as dim text using the given color.
template<typename T>
inline Styled_text<T>
dim(T const& t, Text_color c)
{
  return styled(t, c, dim_text, plain_text);
}


// Render t as dim red text.
template<typename T>
inline Styled_text<T>
dim_red(T const& t)
{
  return dim(t, red_text);
}


// Render t as dim blue text.
template<typename T>
inline Styled_text<T>
dim_blue(T const& t)
{
  return dim(t, blue_text);
}


// Render t as dim green text.
template<typename T>
inline Styled_text<T>
dim_green(T const& t)
{
  return dim(t, green_text);
}


// Render t as dim cyan text.
template<typename T>
inline Styled_text<T>
dim_cyan(T const& t)
{
  return dim(t, cyan_text);
}


// Render t as dim magenta text.
template<typename T>
inline Styled_text<T>
dim_magenta(T const& t)
{
  return dim(t, magenta_text);
}


// Render t as dim yellow text.
template<typename T>
inline Styled_text<T>
dim_yellow(T const& t)
{
  return dim(t, yellow_text);
}


// Render t as dim white text.
template<typename T>
inline Styled_text<T>
dim_white(T const& t)
{
  return dim(t, white_text);
}


void start_font(std::ostream&, Font_spec);
void end_font(std::ostream&);


template<typename T>
std::ostream&
operator<<(std::ostream& os, Styled_text<T> const& txt)
{
  start_font(os, txt.font);
  os << txt.txt;
  end_font(os);
  return os;
}


void init_colors();


} // namespace lingo

#endif
