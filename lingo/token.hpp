// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#ifndef LINGO_TOKEN_HPP
#define LINGO_TOKEN_HPP

#include <lingo/symbol.hpp>
#include <lingo/location.hpp>

#include <list>
#include <iosfwd>

namespace lingo
{

// -------------------------------------------------------------------------- //
//                            Token class

// The kind of an invalid token.
constexpr int invalid_tok = -1;


// A classified symbol in the source language.
//
// Note that tokens internally track their kind as
// an integer value. This allows client languages
// to define their own token enumeration without
// having to instantiate a new token class.
class Token
{
public:
  Token();
  Token(Location);
  Token(Location, Symbol const*);

  explicit operator bool() const;

  int           kind() const;
  String const& spelling() const;
  Location      location() const;

  Symbol const*         symbol() const;
  Identifier_sym const* identifier_symbol() const;
  Boolean_sym const*    boolean_symbol() const;
  Integer_sym const*    integer_symbol() const;
  Character_sym const*  character_symbol() const;
  String_sym const*     string_symbol() const;

private:
  Location      loc_;
  Symbol const* sym_;
};


// Initialize the token to the error token.
inline
Token::Token()
  : Token({}, nullptr)
{ }


// Initialize an invalid token at the given source
// location.
inline
Token::Token(Location loc)
  : Token(loc, nullptr)
{ }


// Initialize a token of kind k with the given
// symbol table entry.
inline
Token::Token(Location loc, Symbol const* s)
  : loc_(loc), sym_(s)
{ }


// Returns true if the token is valid.
inline
Token::operator bool() const
{
  return sym_;
}


// Returns the token kind.
inline int
Token::kind() const
{
  if (sym_)
    return sym_->token();
  else
    return -1;
}


// Returns the spelling of the token.
inline String const&
Token::spelling() const
{
  return sym_->spelling();
}


// Returns the source location of the token.
inline Location
Token::location() const
{
  return loc_;
}


// Returns the token's symbol and attributes.
inline Symbol const*
Token::symbol() const
{
  return sym_;
}


// Return the identifier symbol for the token.
inline Identifier_sym const*
Token::identifier_symbol() const
{
  return cast<Identifier_sym>(sym_);
}


// Return the boolean symbol for the token.
inline Boolean_sym const*
Token::boolean_symbol() const
{
  return cast<Boolean_sym>(sym_);
}


// Returns the integer symbol for the token.
inline Integer_sym const*
Token::integer_symbol() const
{
  return cast<Integer_sym>(sym_);
}


// Return the character symbol for the token.
inline Character_sym const*
Token::character_symbol() const
{
  return cast<Character_sym>(sym_);
}


// Return the string symbol for the token.
inline String_sym const*
Token::string_symbol() const
{
  return cast<String_sym>(sym_);
}


std::ostream& operator<<(std::ostream&, Token);


// -------------------------------------------------------------------------- //
// Token buffer


// A token buffer is a finite sequence of tokens. Note that tokens are
// maintained in a linked list so that modifications during parsing won't
// invalidate iterators.
using Token_seq = std::list<Token>;


// -------------------------------------------------------------------------- //
//                            Token stream

// A token stream provides a stream interface to a token buffer. Note
// that tokens in the stream can be source from multiple input buffers.
class Token_stream
{
public:
  using Position = Token_seq::iterator;

  Token_stream();
  Token_stream(Token_seq const&);

  bool eof() const;
  Token peek() const;
  Token peek(int) const;
  Token get();
  void put(Token);

  Token_seq const& tokens() const { return buf_; }
  Token_seq&       tokens()       { return buf_; }

  Location location() const;

  // FIXME: Use iterators, begin, and end instead
  // of a stream position.
  Position position() const;
  void     reposition(Position);

// private:
  Token_seq buf_;   // The underlying token buffer
  Position pos_;    // The current input/output position.
};


// Initialize a token stream with an empty token buffer.
inline
Token_stream::Token_stream()
  : buf_(), pos_(buf_.begin())
{ }


// Initialize a token stream with the given token buffer.
inline
Token_stream::Token_stream(Token_seq const& toks)
  : buf_(toks), pos_(buf_.begin())
{ }


// Returns true if the stream is at the end of the file.
inline bool
Token_stream::eof() const
{
  return pos_ == buf_.end();
}


// Returns the current token. If at the end of file, return an invalid
// token.
inline Token
Token_stream::peek() const
{
  if (eof())
    return Token();
  else
    return *pos_;
}


// Returns the nth token past the current position.
inline Token
Token_stream::peek(int n) const
{
  // Get the nth token, but restore the stream position
  // afterwards. Note that this will gracefully handle
  // an eof during lookahead.
  Position i = pos_;
  while (i != buf_.end() && n) {
    ++i;
    --n;
  }
  if (i == buf_.end())
    return Token();
  else
    return *i;
}


// Returns the current token and advances the stream. If past the end of
// input, this returns an invalid token.
inline Token
Token_stream::get()
{
  if (eof())
    return Token();
  else
    return *pos_++;
}


// Puts the given token at the end of the stream.
inline void
Token_stream::put(Token tok)
{
  buf_.push_back(tok);

  // Make sure that the pos_ isn't pointing past
  // then end after insertion into an empty list.
  if (pos_ == buf_.end())
    pos_ = buf_.begin();
}


// Returns the current position of the stream. This
// effectively an iterator into the buffer.
inline Token_stream::Position
Token_stream::position() const
{
  return pos_;
}


// Sets the position of the stream that indicated. The
// position must be in the current buffer.
inline void
Token_stream::reposition(Position p)
{
  pos_ = p;
}


// Returns the source location of the current token.
inline Location
Token_stream::location() const
{
  return peek().location();
}


} // namespace lingo

#endif
