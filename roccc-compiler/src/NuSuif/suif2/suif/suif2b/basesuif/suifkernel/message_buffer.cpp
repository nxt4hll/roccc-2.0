#include "common/system_specific.h"
#include "message_buffer.h"

MessageBuffer::~MessageBuffer(void)
{
}




StringMessageBuffer::StringMessageBuffer(const String msg) :
  MessageBuffer(),
  _msg(msg)
{
}


String StringMessageBuffer::get_string(void)
{
  return _msg;
}


MessageBuffer *StringMessageBuffer::clone(void)
{
  return new StringMessageBuffer(_msg);
}








/* ********************************************************
 * AndMessageBuffer
 * ********************************************************* */


AndMessageBuffer::AndMessageBuffer(void) :
  MessageBuffer(),
  _msg_list()
{
}


AndMessageBuffer::~AndMessageBuffer(void)
{
  for (list<MessageBuffer*>::iterator it = _msg_list.begin();
       it != _msg_list.end();
       it++ ) {
    delete (*it);
  }
}
       
  



String AndMessageBuffer::get_string(void)
{
  list<MessageBuffer*>::iterator it = _msg_list.begin();
  if (it == _msg_list.end())
    return (*it)->get_string();
  String str("(");
  str += (*it)->get_string();
  for (it++; it != _msg_list.end(); it++) {
    str += ", and\n";
    str += (*it)->get_string();
  }
  str += ")";
  return str;
}



void AndMessageBuffer::add_message(const String msg)
{
  _msg_list.push_back(new StringMessageBuffer(msg));
}


void AndMessageBuffer::add_message(MessageBuffer *mbuf)
{
  _msg_list.push_back(mbuf->clone());
}


MessageBuffer *AndMessageBuffer::clone(void)
{
  AndMessageBuffer *abuf = new AndMessageBuffer();
  for (list<MessageBuffer*>::iterator it = _msg_list.begin();
       it != _msg_list.end();
       it++) {
    abuf->add_message(*it);
  }
  return abuf;
}

int AndMessageBuffer::get_message_count(void)
{
  return _msg_list.length();
}



/* *******************************************************
 * CausalMessageBuffer
 * ******************************************************* */


CausalMessageBuffer::CausalMessageBuffer(const String effect,
					 MessageBuffer *cause) :
  MessageBuffer(),
  _effect(effect),
  _cause(cause->clone())
{
}


CausalMessageBuffer::CausalMessageBuffer(const String effect,
					 const String cause) :
  MessageBuffer(),
  _effect(effect),
  _cause(new StringMessageBuffer(cause))
{
}


CausalMessageBuffer::~CausalMessageBuffer(void)
{
  delete _cause;
}

String CausalMessageBuffer::get_string(void)
{
  return String("(") + _effect + " because\n" + _cause->get_string() +")";
}

MessageBuffer* CausalMessageBuffer::clone(void)
{
  return new CausalMessageBuffer(_effect, _cause);
}
