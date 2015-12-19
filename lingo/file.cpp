// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "config.hpp"

#include "lingo/file.hpp"
#include "lingo/error.hpp"

#include <fstream>
#include <iterator>

namespace lingo
{


namespace
{

// Return a string containing the text of the file.
String
read_file(Path const& p)
{
  std::ifstream f(p.native());

  String text;
  f.seekg(0, std::ios::end);
  text.reserve(f.tellg());
  f.seekg(0, std::ios::beg);

  std::istreambuf_iterator<char> first = f;
  std::istreambuf_iterator<char> last;
  text.assign(first, last);

  return text;
}

} // namespace


File::File(Path const& p)
  : Buffer(read_file(p)), path_(p), index_(0)
{ }


// Construct file with the given index. This will cache the
// text of the file.
File::File(Path const& p, int n)
  : Buffer(read_file(p)), path_(p), index_(n)
{ }


namespace
{

// The global file manager.
File_manager fm_;

} // namespace


// Returns the global file manager.
File_manager&
file_manager()
{
  return fm_;
}


// Open the file at the path indicatd by `p`. If the file has already
// been opened, then do nothing.
File&
File_manager::open(Path const& p)
{
  Path real = canonical(p);
  auto ins = lookup_.insert({p.native(), 0});
  if (ins.second) {
    File* file = new File(real, files_.size());
    files_.push_back(file);
    ins.first->second = file->index();
    return *file;
  } else {
    int n = ins.first->second;
    return *files_[n];
  }
}


File&
File_manager::file(int n)
{
  lingo_assert(0 <= n && n << files_.size());
  return *files_[n];
}


} // namespace lingo
