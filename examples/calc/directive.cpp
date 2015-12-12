// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "directive.hpp"

#include "lingo/error.hpp"

#include <iostream>


namespace calc
{

using namespace lingo;


namespace
{

Evaluation_mode mode_;

}

// Returns the evaluation mode.
Evaluation_mode
evaluation_mode()
{
  return mode_;
}


// Set the evaluation mode.
void
evaluation_mode(Evaluation_mode m)
{
  mode_ = m;
}


// Process the directive. Note that errors may occur.
void
process_directive(Buffer const& buf)
{
  String const& str = buf.str();
  std::size_t end = str.find_first_of(' ');
  String dir = str.substr(1, end);

  // TODO: Use a hash table.
  if (dir == "step") {
    mode_ = step_mode;
    note("evaluation mode set to 'step'");
  } else if (dir == "eval") {
    mode_ = eval_mode;
    note("evaluation mode set to 'eval'");
  } else {
    error("unknown directive '{}'", dir);
  }
}


} // namespace calc
