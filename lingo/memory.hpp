// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#ifndef LINGO_MEMORY_HPP
#define LINGO_MEMORY_HPP

#include "lingo/debug.hpp"

#include <list>
#include <set>
#include <unordered_map>
#include <vector>

namespace lingo
{

class Integer;
class String_view;
class Token;

// -------------------------------------------------------------------------- //
//                            Factories

// A unique factory will only allocate new objects if
// they have not been previously created. A strict weak
// order is used to determine the uniqueness of objects.
template<typename T, typename C>
struct Unique_factory : std::set<T, C>
{
  // Create an object of type T.
  template<typename... Args>
  T* make(Args&&... args) 
  {
    auto ins = this->emplace(std::forward<Args>(args)...);
    return const_cast<T*>(&*ins.first);
  }
};


// A singleton factory will only allocate a single
// object of the given type.
template<typename T>
struct Singleton_factory
{
  Singleton_factory()
    : object(nullptr)
  { }

  // Create an object of type T. If T has already been
  // created, the arguments given here must match those
  // given previously (or be empty).
  template<typename... Args>
  T* make(Args&&... args)
  {
    if (!object)
      object = new T(std::forward<Args>(args)...);
    return object;
  }

  T* object;
};


// -------------------------------------------------------------------------- //
//                          Garbage collector

class Collecting_factory;
class Reach;


// Every object allocated through the garbage collector is
// an instance of the collectable type.
struct Collectable
{
  Collectable()
    : marked(false)
  { }

  virtual ~Collectable() { }

  virtual void reach() = 0;

  bool marked;
};


// The following functions support the mark and sweep
// garbage collection algorithm. We define mark() for a
// large number of types, especially nodes. This provides 
// a generic facility for traversing the common structure 
// of those nodes.


// Do not mark arithmetic values.
template<typename T>
inline typename std::enable_if<std::is_arithmetic<T>::value>::type
mark(T) { }


// Do not mark enumerations.
template<typename T>
inline typename std::enable_if<std::is_enum<T>::value>::type
mark(T) { }


// Do not mark integers.
inline void
mark(Integer const&) { }


// Do not mark strings.
inline void
mark(String_view const&) { }


// Do not mark tokens.
inline void
mark(Token const&) { }


// Mark an empty node.
template<typename T>
inline void 
mark_nullary(T* t)
{
  if (Collectable* c = dynamic_cast<Collectable*>(t))
    c->marked = true;
}


// Mark a unary node.
template<typename T>
inline void 
mark_unary(T* t)
{
  mark_nullary(t);
  mark(t->first);
}


// Mark a binary node.
template<typename T>
inline void 
mark_binary(T* t)
{
  mark_unary(t);
  mark(t->second);
}


// Mark a ternary node.
template<typename T>
inline void 
mark_ternary(T* t)
{
  mark_binary(t);
  mark(t->third);
}


// The garbage factory class tracks each object that it allocates.
// At various phases of execution, the collect() function can be
// used to reclaim unreferenced memory.
//
// The garbage factory is designed to collect nodes.
//
// Note that the garbage collector is a global resource.
//
// TODO: The root set is not especially efficient since we have
// to traverse it (it's an unordered map).
class Collecting_factory
{
  // Make every T collectable.
  template<typename T>
  struct Collectable_type : T, Collectable
  {
    template<typename... Args>
    Collectable_type(Args&&... args)
      : T(std::forward<Args>(args)...), Collectable()
    { }

    void reach() override
    {
      using lingo::mark;
      mark(this);
    }
  };

public:
  // Create an object of type T. Note that T must be a node type.
  template<typename T, typename... Args>
  T* make(Args&&... args)
  {
    Collectable_type<T>* obj = new Collectable_type<T>(std::forward<Args>(args)...);
    objects.push_back(obj);
    return obj;
  }

  // Root management.
  void declare(Reach*);
  void undeclare(Reach*);

  void collect();

private:
  void mark();
  void sweep();

private:
  using Object_list = std::list<Collectable*>;
  using Root_set = std::unordered_map<Reach*, Reach*>;

  Object_list objects;
  Root_set    roots;
};


Collecting_factory& gc();


// The reach class declares a new root into the garbage
// collector. When this object is destroyed, it is no
// longer a collection root.
//
// Note that a GC root will only store references to
// collectable objects.
class Reach : std::vector<Collectable*>
{
  friend class Collecting_factory;
public:
  Reach()
  { 
    gc().declare(this); 
  }

  // Declare a new garbage collection root, initialized
  // with the given object.
  template<typename T>
  Reach(T* t)
    : Reach()
  { 
    declare(t);
  }

  // Declare a new garbage collection root, initialized
  // with the given sequence of objects.
  template<typename T, typename... Args>
  Reach(T* t, Args*... args)
    : Reach(args...)
  {
    declare(t);
  }

  ~Reach() 
  {
    gc().undeclare(this);
  }

  // Declare `u` to be reachable.
  template<typename T>
  void operator()(T* t)
  {
    declare(t);
  }

private:
  // Declare `t` as reachable. Note that this is only defined
  // for pointers to objects of polymorphic type. Otherwise,
  // don't track references.
  template<typename T>
  typename std::enable_if<std::is_polymorphic<T>::value>::type
  declare(T* t)
  {
    if (Collectable* obj = dynamic_cast<Collectable*>(t))
      push_back(obj); 
  }


  template<typename T>
  typename std::enable_if<!std::is_polymorphic<T>::value>::type
  declare(T* t)
  {
  }


  // Mark all nodes reachable from this root with
  // the given value. This is used by the garbage
  // collector.
  void reach() 
  { 
    for (Collectable* r : *this)
      r->reach();
  }
};



// -------------------------------------------------------------------------- //
//                            Algorithms


// This function deletes the object pointed at by p sets it
// to nullptr, returning that value. This is primarily used
// to reduce reduntant code on certain error conditions during
// parsing and elaboration.
template<typename T>
inline T*
reset(T*& p)
{
  delete p;
  return p = nullptr;
}


} // namespace lingo

#endif
