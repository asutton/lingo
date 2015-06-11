// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "location.hpp"
#include "file.hpp"

#include <iostream>

namespace lingo
{


Location::Location(File const* f, int n)
  : loc_{f->index(), n}
{ }



// Returns the file associated with the location.
File const&
Location::file() const
{
  return file_manager().file(loc_.file);
}



Line const&
Location::line() const
{
  return file().line(loc_.offset);
}

int
Location::column_number() const
{
  return loc_.offset - line().offset();
}


std::ostream& 
operator<<(std::ostream& os, Location loc)
{
  if (loc == Location::none)
    return os;

  return os << loc.file() 
            << ':' << loc.line_number() 
            << ':' << loc.column_number();
}


// Static values.
Location Location::none { };


} // namespace lingo
