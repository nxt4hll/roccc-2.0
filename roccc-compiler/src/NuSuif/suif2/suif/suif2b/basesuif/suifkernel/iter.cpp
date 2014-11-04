#include "common/system_specific.h"
#include "iter.h"

const MetaClass* IterHelper::current_meta_class() const {
  if (_iter)return _iter->current_meta_class();
  return 0;
}

ObjectWrapper IterHelper::current_object() const {
  ObjectWrapper obj;
  if (_iter)
    obj = _iter->current_object();
  return(obj);
}


const LString& IterHelper::current_name() const {
  if (_iter)return _iter->current_name();
  return emptyLString;
}

  
bool IterHelper::is_valid() const {
  if (_iter)return _iter->is_valid();
  return false;
}


void IterHelper::next() {
  if (!_owned)clone_iter();
  if (_iter)_iter->next();
}

void *IterHelper::current() const {
  if (_iter)return _iter->current();
  return 0;
  }
  

void IterHelper::previous() {
  if(!_owned)clone_iter();
  if (_iter)_iter->previous();
}


void IterHelper::set_to( size_t index ) {
  if(!_owned)clone_iter();
  if (_iter)_iter->set_to( index );
}

  
size_t IterHelper::length() const {
  if (_iter)return _iter->length();
  return 0;
}

IterHelper::IterHelper( Iterator* iter ) : _iter( iter ),_owned(true) {
//IterHelper::IterHelper( Iterator* iter ) : _owned(true) {
  //_iter = iter->clone();
  //fprintf(stderr,"[JUL] iter.cpp@IterHelper::IterHelper::%x(%x->%x)\n",this,iter,_iter);
  //fprintf(stderr,"\tlength=%d\n",length());
}
  
IterHelper::~IterHelper() {
  if(_owned)delete _iter;
}


IterHelper::IterHelper(const IterHelper &x) : _iter(x._iter),_owned(x._owned) {
    const_cast<IterHelper&>(x)._owned = false;
    }

IterHelper & IterHelper::operator =(const IterHelper &x) {
    _iter = x._iter;
    _owned = x._owned;
    const_cast<IterHelper&>(x)._owned = false;
    return *this;
    }

void IterHelper::clone_iter() {
    _iter = _iter->clone();
    _owned = true;
    }
