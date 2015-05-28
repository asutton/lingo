
# Introduction

The `lingo` library is a set of facilities that support the construction 
of compilers, interpreters, and other language related tools.  In particular,
this library aims to provide reasonable implementations of common facilities
that can be difficult and time-consume to construct as part of a compiler
implementation.

This library provides the following facilities:

- Memory management -- A set of factories that facilities that allocation
  and cleanup of nodes. This includes an opt-in garbage collector that
  works tree structures that mix collected and non-collected noes.
- String representations -- A lightweight string view that supports
  zero-copy access to parsed text.
- String formatting -- Using cppformat, tools for formatting strings.
- Source locations -- Facilities for efficiently representing locations in
  source code.
- Symbol table -- A facility for ensuring that each symbols in a parsed
  file are uniquely represented. This also allows additional information
  to be associated with those symbols. 
- Token representation -- A lightweight and extensible representation
  of lexical tokens, integrate with the symbol table for ease of used.
- Error handling -- Tools for asserting logical conditions and control
  flow impossibilities.
- Diagnostics -- A set of facilities for emitting error and warning
  diagnostics.
- Node abstraction -- Generic data types and algorithms that support the 
  definition of abstract syntax trees. Many of the other facilities in
  this library are designed on node concepts.
- Pretty printing -- A facility that supports the generation of formatted
  string output for languages built on the Node concepts.
- Debug printing -- An extension of pretty printing that emits an s-expr
  representation of abstract syntax trees.
- Parsing and lexing facilities -- A set of generic algorithms that can
  be used to construct efficient lexers and parsers. These modules
  also define the concepts of character and token streams as well as the
  parsers and lexers that operate over them.

There are a number of features missing from this library. In particular:

- File management
- Application configuration (command line parsing, etc).
- Real number types
- Character and string literal support
- Unicode support for parsing and lexing
- Support multithreaded environments
- Windows support

Also, note that many of the facilities could be further optimized. In
particular, the garbage collector is fairly naive with respect to
allocation policies, and the unique representation of source locations
could be dramatically improved.


# Getting the source

This library is designed to be included as a submodule in a larger
project. For your compiler add this submodule from your top-level
source directory using the command

    git submodule add https://github.com/asutton/lingo


The `lingo` library also depends on a few external libraries:

  - cppformat -- This is included as a git submodule of `lingo`
  - GMP -- the GNU Multiprecision library must be installed on
    your system.


# Building `lingo`

The `lingo` library is built using CMake (version 3.0 or greater).
Adding the cloned `lingo` directory should be sufficient to make it
build. `lingo` produces a static library named `liblingo`. This should
be linked against your compiler implementations.
