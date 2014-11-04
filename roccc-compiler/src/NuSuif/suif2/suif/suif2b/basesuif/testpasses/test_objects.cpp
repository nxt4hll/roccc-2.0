#include "common/system_specific.h"
#include "test_objects.h"

#include "iokernel/stl_meta_class.h"
#include "iokernel/aggregate_meta_class.h"
#include "iokernel/helper.h"

#include "common/suif_vector.h"


// #ifdef PGI_BUILD
// #include <new>
// #else
// #include <new.h>
// #endif
#include <new>

using namespace std;

void TestX::setX1( TestX* x1 ) {
  this->x1 = x1;
}


void TestX::setX2( TestX* x2 ) {
  this->x2 = x2;
}


TestX* TestX::getX1() {
  return x1;
}

TestX* TestX::getX2() {
  return x2;
}

void TestX::setInt1( int i ) {
  this->i = i;
}


int TestX::getInt1() {
return i;
}


void TestX::add( TestX* x ) {
  _vector->push_back( x );
}


Iter<TestX*>* TestX::get_vector_iterator() {
 static const LString field_name("_vector");
  FieldDescription* field = get_aggregate_meta_class()->get_field_description(field_name);
  return new Iter<TestX*>( new STLIterator<suif_vector<TestX*> > (_vector,field->metaClass) );
}

void TestX::constructorFunction( void* place ) {
    new(place) TestX();
}

TestX::TestX() {
  x1 = x2 = 0;
  i = 0;
  _vector = new suif_vector<TestX*>;
}

TestX::~TestX() {
  delete x1;
  delete x2;
  delete_list_and_elements( _vector );
}


static const LString testx_class_name( "TestX" );

const LString &TestX::get_class_name() {return testx_class_name;}

// -------------------------------- TestY ----------------------

void  TestY::setX3( TestX* x3 ) {
  this->x3 = x3;
}


TestX* TestY::getX3() {
  return x3;
}


TestY::TestY() {
  x3 = 0;
}

TestY::~TestY() {
  delete x3;
}

void TestY::constructorFunction( void* place ) {
    new(place) TestY();
}


static const LString testy_class_name(Object::get_class_name() + "/TestY");

const LString &TestY::get_class_name() {return testy_class_name;}
