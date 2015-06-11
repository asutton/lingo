// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "file.hpp"
#include "error.hpp"

#include <fstream>
#include <iterator>

namespace lingo
{

// Returns the line number for the character offset. Behavior
// is undefined if n is not within the line map.
Line const&
Line_map::line(int n) const
{
  lingo_assert(lower_bound(n) != end());
  return lower_bound(n)->second;
}


// Construct file with the given index. This will cache the 
// text of the file.
File::File(Path const& p, int n)
  : path_(p), text_(), lines_(), index_(n)
{
  std::ifstream f(path_.native());

  f.seekg(0, std::ios::end);   
  text_.reserve(f.tellg());
  f.seekg(0, std::ios::beg);

  std::istreambuf_iterator<char> first = f;
  std::istreambuf_iterator<char> last;
  text_.assign(first, last);
}


std::ostream&
operator<<(std::ostream& os, File const& f)
{
  return os << f.path();
}


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
  if (!ins.second) {
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
