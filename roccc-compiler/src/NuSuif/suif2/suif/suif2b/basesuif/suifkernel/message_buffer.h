#ifndef _MESSAGE_BUFFER_H_
#define _MESSAGE_BUFFER_H_

#include "common/MString.h"
#include "common/suif_list.h"


class MessageBuffer {
 public:
  virtual String get_string(void) = 0;
  virtual ~MessageBuffer(void);
  virtual MessageBuffer *clone(void) = 0;
};


class StringMessageBuffer : public MessageBuffer {
 public:
  StringMessageBuffer(const String);
  virtual String get_string(void);
  virtual MessageBuffer *clone(void);
 private:
  String _msg;
};



class AndMessageBuffer : public MessageBuffer {
 public:
  AndMessageBuffer(void);
  ~AndMessageBuffer(void);
  virtual String get_string(void);
  void add_message(const String msg);
  void add_message(MessageBuffer *mbuf);
  virtual MessageBuffer *clone(void);
  int get_message_count(void);
 private:
  list<MessageBuffer*> _msg_list;
};



class CausalMessageBuffer : public MessageBuffer {
 public:
  CausalMessageBuffer(const String effect, MessageBuffer *cause);
  CausalMessageBuffer(const String effect, const String cause);
  ~CausalMessageBuffer(void);
  virtual String get_string(void);
  virtual MessageBuffer *clone(void);
 private:
  const String _effect;
  MessageBuffer *_cause;
};


#endif /* _MESSAGE_BUFFER_H_ */
