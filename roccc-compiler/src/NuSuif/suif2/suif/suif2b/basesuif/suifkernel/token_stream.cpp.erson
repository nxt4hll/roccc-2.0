#include "common/system_specific.h"
#include "token_stream.h"
#include "common/suif_vector.h"
#include "char_source.h"
#ifndef MSVC
#include <strstream.h>
#else
#include <strstrea.h>
#endif
#include <string.h>


/**
 * Return true if c is a special character that could be a token itself.
 */
static bool
is_singleton(int c)
{
  return (strchr(";{}#[]$\n", c) != NULL);
}

/**
 * Return true if c is the escape character
 */
static bool
is_escape(int c)
{
  return (c == '\\');
}

/**
 * Return true if c is a white space for tokens.
 * Note that newline char is not a white space but a token by itself.
 */
static bool
is_noise(int c)
{
  return (strchr(" \t\r\f", c) != NULL);
}



static String get_blank_prefix( CharSource *i ) {
  String result;
  int ch = i->peek();
  while (is_noise(ch)) {
    result.push( i->get() );
    ch = i->peek();
  }
  // i.peek() is not one of noises

  return result;
}



static String get_string( CharSource *i ) {
  String result;
  int ch = i->peek();
  if (is_singleton(ch)) {
    result.push(i->get());
  } else {
    while ((ch != EOF) &&
	   (!is_noise(ch)) &&
	   (!is_singleton(ch))) {
      if (is_escape(ch)) {
	i->get();
	ch = i->peek();
      }
      if (ch != EOF) {
	result.push( i->get() );	
	ch = i->peek();
      }
    }
  }
  return result;
}


Token::Token( String prefix, String token, String separator ) :
  _prefix( prefix ),
  _token( token ),
  _separator( separator ) {
}


bool Token::is_equal(const char* s)
{
  if (s == NULL)
    return _token.is_empty();
  else
    return (strcmp(s, _token.c_str()) == 0);
}





TokenStream::TokenStream( CharSource *i ) :
   _tokens( new Tokens ),
   _input_stream( i )
{
}



TokenStream::TokenStream( int argc, char* argv[] ) :
         _tokens( new Tokens ),
         _input_stream( NULL )
{
  for (int i = argc-1; 0 <= i; i--) {
    Token current(String(""), String(argv[i]), String(" "));
    _tokens->push_back(current);
  }
}


TokenStream::TokenStream(void) :
  _tokens(new Tokens),
  _input_stream(NULL)
{
}


TokenStream::~TokenStream() {
  delete _tokens;
}


bool TokenStream::get_token( Token& t ) {
  int size = _tokens->size();
  if (size > 0) {
    t = (*_tokens)[size-1];
    _tokens -> pop_back();
    return true;
  } else if ( _input_stream != NULL) {
    t._prefix    = get_blank_prefix( _input_stream );
    t._token     = get_string( _input_stream );
    t._separator = emptyString; // get_blank_prefix( _input_stream );
    return (t._token.length() > 0);
  } else {
    return false;
  }
}



bool TokenStream::peek_token( Token& t ) {
  if ( get_token( t ) ) {
    push_back( t );
    return true;
  }
  return false;
}


void TokenStream::push_back( Token& t ) {
  _tokens->push_back( t  );
}



istream* TokenStream::get_stream() {
   int braces_open = 1;
   strstream* stream = new strstream;

   Token t;
   while ( get_token( t ) ) {
     if (t.is_equal("}") && braces_open == 1)
       return stream;
     else if (t.is_equal("{"))
       braces_open++;
     else if (t.is_equal("}"))
       braces_open--;
     (*stream)<<t._prefix<<t._token<<t._separator;
   }
   return stream;
}


bool TokenStream::is_at_end_of_command() {
  Token t;
  if (peek_token(t)) {
    return (t.is_equal("\n") || t.is_equal(";"));
  }
  return true;
}


bool TokenStream::is_empty() {
  if ( _tokens->size() > 0)
    return false;
  Token t;
  return (!peek_token(t));
}




/*
 * Skip a token.  Supposed to be '\n' or \;' only.

bool TokenStream::advance() {
  Token t;
  bool status = get_token(t);
  return status;
}
*/







