
/** A charSource is a character source designed for token_stream.
 *  It behaves like a character stream and hides the actual source
 *  of characters; e.g. from a file, a string, or standard input.
 *  It support only two methods 
 *   get()  - read off and return the next character
 *   peek() - return the next character without reading it off.
 *
 */

#ifndef SUIFKERNEL__CHARSOURCE_H
#define SUIFKERNEL__CHARSOURCE_H

//#include <iostream.h>
#include <iostream>

class CharSource {
 public:
  virtual int get(void) = 0;
  virtual int peek(void) = 0;
  virtual ~CharSource(){}
};



/** A StdCharSource takes its input from cin.
 *  If a prompt is defined, it will be printed on cout after
 *  reading each newline char from cin.
 */
class StdCharSource : public CharSource {
 private:
  const char*	_prompt;
  bool		_isNewline;
  int		_cbuf;
  bool		_isBufFull;
 public:
  StdCharSource(const char* prompt = NULL);
  virtual ~StdCharSource(){}
  virtual int get(void);
  virtual int peek(void);
};



/*
 * An IstreamCharSource is a CharSource whose characters come
 * from an istream.
 */
class IstreamCharSource : public CharSource {
 private:
     std::istream	*_istream;
 public:
  IstreamCharSource(std::istream*);
  virtual ~IstreamCharSource(void);
  virtual int get(void);
  virtual int peek(void);
};


#endif /* SUIFKERNEL__CHARSOURCE_H */
