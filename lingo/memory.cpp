// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "config.hpp"

#include "lingo/memory.hpp"
#include "lingo/error.hpp"

namespace lingo
{

namespace
{

// The garbage collector.
Collecting_factory gc_;

} // namespace


// Declare a new GC root.
void
Collecting_factory::declare(Reach* r)
{
  lingo_assert(roots.count(r) == 0);
  roots.insert({r, r});
}


// Un-declare a GC root.
void
Collecting_factory::undeclare(Reach* r)
{
  lingo_assert(roots.count(r) == 1);
  roots.erase(r);
}


// Mark all reachable objects.
void
Collecting_factory::mark()
{
  // Make each object unreached.
  for (Collectable* p : objects)
    p->marked = false;

  // Mark reachable objects.
  for (auto& x : roots) {
    x.second->reach();
  }
}


// Reclaim unreachable objects.
void
Collecting_factory::sweep()
{
  // FIXME: Is there an algorithm that does this? list::remove_if
  // is close, but it won't delete the associated data.
  auto iter = objects.begin();
  while (iter != objects.end()) {
    Collectable* o = *iter;
    if (!o->marked) {
      delete o;
      iter = objects.erase(iter);
    } else {
      ++iter;
    }
  }
}


// Collect garbage. This uses a simple mark and sweep
// algorithm. We first traverse all roots, marking each
// reachable object. Then, we traverse all objects,
// deleting those that aren't marked.
void
Collecting_factory::collect()
{
  mark();
  sweep();
}


// Returns the garbage collector.
Collecting_factory&
gc()
{
  return gc_;
}


} // namespace lingo
