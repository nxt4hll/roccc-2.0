#ifndef TESTPASSES__TESTOBJECTS_H
#define TESTPASSES__TESTOBJECTS_H

#include "suifkernel/suif_object.h"
#include "suifkernel/suifkernel_forwarders.h"
#include "suifkernel/iter.h"

class TestX : public SuifObject {
   friend class TestObjectFactory;
public:
  virtual ~TestX();

  virtual void setX1( TestX* x1 );
  virtual void setX2( TestX* x2 );
  virtual TestX* getX1();
  virtual TestX* getX2();
  virtual void setInt1( int i );
  virtual int getInt1();
  virtual void add( TestX* x );
  virtual Iter<TestX*>* get_vector_iterator();

  static const LString &get_class_name();

protected:

  static void constructorFunction( void* place );

  TestX();


private:
  int i;
  TestX* x1;
  TestX* x2;
  suif_vector<TestX*>* _vector;
};


// -------------------------------- TestY ----------------------

class TestY : public TestX {
   friend class TestObjectFactory;
public:
  virtual void setX3( TestX* x3 );
  virtual TestX* getX3();

  virtual ~TestY();

  static const LString &get_class_name();
protected:
  TestY();

  static void constructorFunction( void* place );

private:
  TestX* x3;
};

#endif



