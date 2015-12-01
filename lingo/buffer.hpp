// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#ifndef LINGO_BUFFER_HPP
#define LINGO_BUFFER_HPP

#include "lingo/string.hpp"
#include "lingo/line.hpp"
#include "lingo/location.hpp"

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
class Buffer : private Line_map
{
public:
  Buffer(String const& str);

  virtual ~Buffer() { }

  // Returns the line map for the buffer.
  Line_map const& lines() const { return *this; }

  // Iterators
  char const* begin() const { return text_.c_str(); }
  char const* end() const   { return begin() + text_.size(); }

  // String representation
  String_view   rep() const { return {begin(), end()}; }
  String const& str() const { return text_; }

protected:
  String   text_;
};


// -------------------------------------------------------------------------- //
//                               Input context
//
// It's often useful to have lexers and parsers simply update a global
// input location for the purpose of simplifying diagnostics. This
// prevents languages from having to continually pass these tokens
// through the interface.


Buffer& input_buffer();
Location input_location();

void set_input_buffer(Buffer&);
void set_input_location(Location);


// The input context is a facility used to manage the
// current input buffer and source location.
struct Input_context
{
  Input_context(Location);
  Input_context(Buffer&);
  Input_context(Buffer&, Location);
  ~Input_context();

  Buffer*  saved_buf;
  Location saved_loc;
};


} // namespace

#endif
