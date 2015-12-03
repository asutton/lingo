// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#ifndef LINGO_BUFFER_HPP
#define LINGO_BUFFER_HPP

#include <lingo/string.hpp>
#include <lingo/line.hpp>
#include <lingo/location.hpp>

#include <map>


namespace lingo
{

// -------------------------------------------------------------------------- //
// Buffers

// A buffer is a region of text containing code. This class
// contains the underlying character sequence and an interface
// for resolving source code locations.
//
// The buffer class is the base class for different kinds of
// memory buffers. In particular, a file is kind of buffer
// that is read from disk.
//
// TODO: Provide better integration with I/O streams, etc. to
// avoid unnecessary copies.
class Buffer
{
public:
  Buffer(String const& str);

  virtual ~Buffer() { }

  // Returns the line map for the buffer.
  Line_map const& lines() const { return lines_; }

  // Iterators
  char const* begin() const { return text_.c_str(); }
  char const* end() const   { return begin() + text_.size(); }

  // String representation
  String_view   rep() const { return {begin(), end()}; }
  String const& str() const { return text_; }

protected:
  String   text_;
  Line_map lines_;
};


} // namespace

#endif
