#include "common/system_specific.h"
#include "meta_class_iter.h"
#include "meta_class.h"
#include "object_wrapper.h"


// #ifdef PGI_BUILD
#include <new>
// #else
// #include <iostream.h>
// #include <new.h>
// #endif
#include <iostream>//jul modif
using namespace std;//jul modif




void Iterator::add( Address object ) {
  kernel_assert_message( false, ("Called illegal operation Iterator::add"  ) );
}

size_t Iterator::length() const {
  size_t count = 0;
  Iterator *n = clone();
  n->set_to( 0 );
  while ( n->is_valid() ) {
    count++;
    n->next();
  }
  delete n;
  return count;
}

void Iterator::set_to( size_t index ) {
  first();
  while ( index-- && is_valid() ) next();
}

ObjectWrapper Iterator::current_object() const {
  ObjectWrapper obj(current(),
		    current_meta_class());
  return(obj);
}

FieldWrapper Iterator::current_field() const {
  FieldWrapper obj(current_object(),
		   current_name());
  return(obj);
}

void Iterator::print_to_default() const {
  if (!is_valid()) {
    cerr << "invalid";
    return;
  }
  Address address = current();
  const MetaClass *mc = current_meta_class();
  LString name = current_name();
  LString mclass_name = mc->get_instance_name();

  cerr << name.c_str() << " = " <<
    mclass_name.c_str() << " @" << address << "'\n";
}

Iterator::~Iterator() {
}


SingleElementIterator::SingleElementIterator( Address address, const MetaClass* metaClass ) :
  _is_valid(true), _object(ObjectWrapper(address, metaClass)) {
}

SingleElementIterator::SingleElementIterator( const ObjectWrapper &obj ) :
  _is_valid(true), _object(obj)
{
}

const MetaClass* SingleElementIterator::current_meta_class() const {
  return _is_valid ? _object.get_meta_class() : 0;
}

const LString& SingleElementIterator::current_name() const {
  return emptyLString;
}


Address SingleElementIterator::current() const {
  return _object.get_address();
}



bool SingleElementIterator::is_valid() const {
  return _is_valid;
}


void SingleElementIterator::next() {
  _is_valid = false;
}

void SingleElementIterator::previous() {
  _is_valid = false;
}

void SingleElementIterator::first() {
  _is_valid = true;
}

size_t SingleElementIterator::length() const {
  return 1;
}

SingleElementIterator& SingleElementIterator::operator=(const SingleElementIterator&) {
  kernel_assert(false);
  return(*this);
}
SingleElementIterator::SingleElementIterator(const SingleElementIterator&other) :
  _is_valid(other._is_valid),
  _object(other._object)
{
}

Iterator *SingleElementIterator::clone() const
    {
    return new SingleElementIterator(*this);
    }

/*
 * Empty Iterator implementation.
 * always returns invalid
 */

EmptyIterator::EmptyIterator() {}

const MetaClass* EmptyIterator::current_meta_class() const {
  return(0);
}

const LString& EmptyIterator::current_name() const {
  return emptyLString;
}


Address EmptyIterator::current() const {
  kernel_assert_message(0, ("Someone called current() on an Empty Iterator"));
  return(0);
}



bool EmptyIterator::is_valid() const {
  return(false);
}


void EmptyIterator::next() {
}

void EmptyIterator::previous() {
}

void EmptyIterator::first() {
}

size_t EmptyIterator::length() const {
  return 0;
}

EmptyIterator& EmptyIterator::operator=(const EmptyIterator&) {
  // no effect
  return(*this);
}
EmptyIterator::EmptyIterator(const EmptyIterator&other) 
{}

Iterator *EmptyIterator::clone() const
{
  return new EmptyIterator();
}




