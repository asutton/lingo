// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#ifndef LINGO_FILE_HPP
#define LINGO_FILE_HPP

// The file module contains facilities for managing open
// files and their text.
//
// TODO: Seprate out line maps into a separate module.
// Also, re-organize this so that we can support source
// code locations in different kinds of memory buffers.
// These might be:
// - files
// - strings in memory
// - macro expansions

#include <iosfwd>
#include <map>
#include <string>
#include <unordered_map>

#include <boost/filesystem.hpp>

namespace lingo
{

// -------------------------------------------------------------------------- //
//                                Lines

// A line of text is the sequence of characters from column
// 0 up to its newline.
//
// The members of this class are public to allow them to be
// updated by a character stream during processing.
class Line
{
public:
  Line(int n, int c, char const* f, char const* l)
    : num(n), off(c), first(f), last(l)
  { }

  int number() const { return num; }
  int offset() const { return off; }

  char const* begin() const { return first; }
  char const* end() const   { return last; }

  int         num;    // Tjhe line number
  int         off;    // The character offset
  char const* first;
  char const* last;
};


// A line map associates an offset in the source code with
// it's underlying line of text.
struct Line_map : std::map<int, Line>
{
  Line const& line(int n) const;
};



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
class File
{
  friend class File_manager;

  File(Path const&, int);

public:
  // Observers
  Path const& path() const { return path_; }
  
  int index() const { return index_; }

  Line_map&       lines()       { return lines_; }
  Line_map const& lines() const { return lines_; }

  Line const& line(int n) const { return lines().line(n); }

  // Iterators
  char const* begin() const { return text_.c_str(); }
  char const* end() const   { return begin() + text_.size(); }

private:
  Path        path_;
  std::string text_;
  Line_map    lines_;
  int         index_;
};


// Streaming
std::ostream& operator<<(std::ostream&, File const&);



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
  using File_list     = std::vector<File*>;
  using File_map      = std::unordered_map<std::string, int>;

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
