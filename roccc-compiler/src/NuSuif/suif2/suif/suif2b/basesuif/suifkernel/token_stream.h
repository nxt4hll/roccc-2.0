#ifndef SUIFKERNEL__TOKEN_STREAM_H
#define SUIFKERNEL__TOKEN_STREAM_H

#include "suifkernel_forwarders.h"

#include "char_source.h"

/**
 * The token parser considers a token to be either:
 *
 * <ul>
 * <li> 1. one of ';', '#', '{', '}', '\n', '[', ']', '$'
 * <li> 2. a string of characters without whitespaces or any char in 1.
 * </ul>
 *
 * When the token is parsed from an istream, the token object also
 * contains a prefix and a postfix, which are strings
 * of whitespaces surrounding the token.
 * The token parser uses a greedy algorithm which consumes as much
 * noise characters as possible after each token.
 */
struct Token {

  Token( String prefix = emptyString,
         String token = emptyString,
         String separator = emptyString );

  String _prefix;
  String _token;
  String _separator;    // The postfix of noice characters

  public:
  bool is_equal(const char *t);
  String get_name(void) { return _token; };
  String get_prefix(void) { return _prefix; };
  String get_postfix(void) { return _separator; };

};


typedef suif_vector<Token> Tokens;
typedef suif_vector<Option*> Options;



/** A token stream is a source of tokens.
   * A token stream can be built from either
   *   - an array of char*, each char* will be a token; or
   *   - an istream, the token parser will parse the chars into tokens.
   *
   * A token stream owns the char source.
   */
class TokenStream {
public:
  TokenStream( CharSource *i );
  TokenStream( int argc, char* argv[] );
  virtual ~TokenStream();

  /** returns true if succeeded */
  virtual bool get_token( Token& t );

  virtual bool peek_token( Token& t );

  virtual void push_back( Token& t );


  /**
   * Assume a "{" has been read off, read off tokens until
   * the matching "}".  Append all tokens read into a strstream
   * and return it.  The match "}" is not included in the strstream.
   */
  virtual std::istream* get_stream();

  /** Return true if no more token can be read.
   */
  virtual bool is_empty();

  /** Return true if next token is either '\n', ';'
   *  or no more token can be read.
   */
  virtual bool is_at_end_of_command();


protected:

  /** subclass is responsible for initializing either _tokens or _input_stream;
    * An empty suif_vector<Token> will be assigned to the _tokens slot.
    */
  TokenStream(void);

  Tokens* _tokens;

  CharSource *_input_stream;
};

class IStreamTokenStream : public TokenStream {
public:
  IStreamTokenStream( std::istream& i ) : TokenStream(new IstreamCharSource(&i)){};
  ~IStreamTokenStream(){};
};

#endif








