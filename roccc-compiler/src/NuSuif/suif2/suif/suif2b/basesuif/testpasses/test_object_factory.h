#ifndef TESTPASSES__TEST_OBJECT_FACTORY_H
#define TESTPASSES__TEST_OBJECT_FACTORY_H

#include "suifkernel/suifkernel_forwarders.h"
#include "suifkernel/real_object_factory.h"

class TestX;
class TestY;

class TestObjectFactory : public RealObjectFactory {
public:
  TestObjectFactory();

  virtual void init( SuifEnv* suif );

  virtual TestX* createTestX( int i, TestX* x1=0, TestX* x2=0 );
  virtual TestY* createTestY( int i, TestX* x1=0, TestX* x2=0, TestX* x3=0 );

  virtual const LString& getName();

};



#endif













