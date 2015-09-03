// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "lingo/buffer.hpp"


namespace calc
{

// The evaluation mode.
enum Evaluation_mode
{
  step_mode,  // Show each evaluation.
  eval_mode,  // Just show the result.
};


Evaluation_mode evaluation_mode();
void evaluation_mode(Evaluation_mode);


// Returns true if the interpreter is in evaluation mode.
inline bool
is_eval_mode()
{
  return evaluation_mode() == eval_mode;
}


// Returns true if the interpreter is in step mode.
inline bool
is_step_mode()
{
  return evaluation_mode() == step_mode;
}


void process_directive(lingo::Buffer const&);


// Returns true if the buffer contains a directive
// instead of an expression.
inline bool
contains_directive(lingo::Buffer const& buf)
{
  return buf.str()[0] == ':';
}



} // namespace
