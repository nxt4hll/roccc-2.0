#include "common/system_specific.h"
#include "suif_exception.h"

//#include <iostream.h>
#include <iostream>
using namespace std;

SuifException::SuifException(String msg) :
  _msgbuf(new StringMessageBuffer(msg))
{
}

SuifException::SuifException(String effect, SuifException *cause) :
  _msgbuf(new CausalMessageBuffer(effect, cause->get_message_buffer()))
{
}

SuifException::SuifException(MessageBuffer *mbuf) :
  _msgbuf(mbuf->clone())
{
}


SuifException::SuifException(const SuifException& that) :
  _msgbuf((that._msgbuf)->clone())
{
}


SuifException::~SuifException(void)
{
  delete _msgbuf;
}


String SuifException::get_message(void)
{
  return _msgbuf->get_string();
}


MessageBuffer *SuifException::get_message_buffer(void)
{
  return _msgbuf;
}




/* *************************************************************
 * SuifDevException
 * ************************************************************* */

SuifDevException::SuifDevException(const char* file, int line, String msg) :
  SuifException(String(file) + ":" + String(line) + " " + msg)
{
}
 
SuifDevException::SuifDevException(const char* file, int line,
				   SuifException *cause) :
  SuifException(String(file) + ":" + String(line) + " " +
		cause->get_message())
{
}

SuifDevException::SuifDevException(const char* file, int line,
				   MessageBuffer *mbuf) :
  SuifException(String(file) + ":" + String(line) + " " + mbuf->get_string())
{
}


SuifDevException::SuifDevException(const SuifDevException& that) :
  SuifException(that)
{
};
