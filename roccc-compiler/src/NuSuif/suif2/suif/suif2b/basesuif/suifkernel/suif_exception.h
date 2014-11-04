#ifndef _SUIF_EXCEPTION_H_
#define _SUIF_EXCEPTION_H_
#include <stdlib.h>
#include "common/MString.h"
#include "message_buffer.h"
#include "suifkernel_messages.h"

#ifdef USE_CPP_EXCEPTION
#define SUIF_THROW(exp) throw exp
#else // USE_CPP_EXCEPTION
#define SUIF_THROW(exp) { \
  suif_error("%s", exp.get_message().c_str()); \
  exit(1);  \
}
#endif // USE_CPP_EXCEPTION




class SuifException;

class SuifException {
 public:
  SuifException(String msg);
  SuifException(String effect, SuifException *cause);
  SuifException(MessageBuffer *mbuf);
  SuifException(const SuifException&);
  virtual ~SuifException(void);

  virtual String get_message(void);
  virtual MessageBuffer *get_message_buffer(void);
 private:
  MessageBuffer *_msgbuf;
};



class SuifDevException : public SuifException {
 public:
  SuifDevException(const char* file, int line, String msg);
  SuifDevException(const char* file, int line, SuifException *cause);
  SuifDevException(const char* file, int line, MessageBuffer *mbuf);
  SuifDevException(const SuifDevException& that);
};






#endif // _SUIF_EXCEPTION_H_
