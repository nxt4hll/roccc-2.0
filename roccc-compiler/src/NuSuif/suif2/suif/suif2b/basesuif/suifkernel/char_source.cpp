#include "common/system_specific.h"

#include "suifkernel/char_source.h"

using namespace std;

/**
 *  Representation:
 *  _prompt	- the prompt to be written to cout after every '\n'.
 *		NULL means no prompt is needed.
 *  _isNewline	- true if last char read is a newline.
 *  _cbuf	- to hold a char for the next get().
 *  _isBufFull	- true if cbuf is not empty
 */

StdCharSource::StdCharSource(const char* prompt)
  : _prompt(prompt),
    _isNewline(true),
    _cbuf(0),
    _isBufFull(false)
{
}



int StdCharSource::get(void)
{
  int c;
  if (_isBufFull) {
    c = _cbuf;
    _isBufFull = false;
  } else {
    if (_isNewline  &&  _prompt != NULL) {
	cout << _prompt;
	cout.flush();
    }   
    c = cin.get();
  }
  // c is the char to be returned
  _isNewline = (c == '\n');
  return c;
}


int StdCharSource::peek(void)
{
  if (!_isBufFull) {
    _cbuf = get();
    _isBufFull = true;
  }
  return _cbuf;
}





IstreamCharSource::IstreamCharSource(istream* ins) :
  _istream(ins)
{
}


IstreamCharSource::~IstreamCharSource(void)
{
  delete _istream;
}


int IstreamCharSource::get(void)
{
  return _istream->get();
}


int IstreamCharSource::peek(void)
{
  return _istream->peek();
}
