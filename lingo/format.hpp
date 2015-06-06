// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#ifndef LINGO_FORMAT_HPP
#define LINGO_FORMAT_HPP

// This module imports the cppformat library by Victor 
// Zverovich. In particular, it makes a number of facilities 
// available in the lingo namesapce.

#include "cppformat/format.h"

#include <iosfwd>

namespace lingo 
{

// The format facility.
using fmt::format;


// Text formatters.
using fmt::bin; 
using fmt::oct;
using fmt::hex;
using fmt::pad;


// Import the Writer class as an alternative to stringstream.
using fmt::Writer;


// Other formatting and streaming utilities.
int stream_base(const std::ios_base&);


} // namespace lingo

#endif
