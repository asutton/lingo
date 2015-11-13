// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#ifndef LINGO_FILE_HPP
#define LINGO_FILE_HPP

// The file module contains facilities for managing open
// files and their text.

#include <lingo/buffer.hpp>

#include <unordered_map>
#include <vector>

#include <boost/filesystem.hpp>

namespace lingo
{

// -------------------------------------------------------------------------- //
//                                Files


// A path name.
//
// TODO: Replace this with the std::path when it becomes
// available.
using Path = boost::filesystem::path;


// Represents a file system file. This object is primary used
// to house the text the file contains, providing long-term
// storage for its text.
//
// A file also includes a line map, which is used to associate
// file information with lines.
//
// TODO: Use std::filesystem when it becomes standard.
class File : public Buffer
{
  friend class File_manager;

  File(Path const&, int);

public:
  // Observers
  Path const& path() const { return path_; }

  int index() const { return index_; }

private:
  Path        path_;
  int         index_;
};


// -------------------------------------------------------------------------- //
//                            File manager

// The file manager provides a facility for globally managing
// opened files. This effectively a list of opened (note: not
// open) files and a side-table for efficient path-based lookup.
class File_manager
{
public:
  File& open(char const*);
  File& open(std::string const&);
  File& open(Path const&);

  File& file(int);

private:
  using File_list = std::vector<File*>;
  using File_map  = std::unordered_map<std::string, int>;

  File_list files_;
  File_map  lookup_;
};


// Open the given file.
inline File&
File_manager::open(const char* p)
{
  return open(Path(p));
}


// Open the given file.
inline File&
File_manager::open(std::string const& p)
{
  return open(Path(p));
}


File_manager& file_manager();


// Open the the file indicated by the given path or throw
// an exception.
inline File&
open_file(char const* p)
{
  return file_manager().open(p);
}


// Open the file indicated by the given path or throw
// an exception.
inline File&
open_file(std::string const& p)
{
  return file_manager().open(p);
}


} // namespace lingo

#endif
