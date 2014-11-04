#ifndef TESTPASSES__TESTMODULES_H
#define TESTPASSES__TESTMODULES_H

#include "suifkernel/module.h"
#include "suifkernel/suifkernel_forwarders.h"

//	Cleaned up by David Moore, late May 99
//	Get rid of all users of bogus TestX and TestY classes. 
//	The usefulness of these has passed and their implementations
//	have not being kept up with the generated classes

class TestCloneModule : public Module {
public:
  TestCloneModule( SuifEnv* suif );
  virtual LString get_module_name() const;

  virtual void execute();

  virtual Module *clone() const;

  static LString ClassName;
};

class NodeCount : public Module {
public:
  NodeCount( SuifEnv* suif );

  virtual void execute();

  virtual Module *clone() const;
};

class VirtualTests : public Module {
public:
  VirtualTests( SuifEnv* suif );

  virtual void execute();

  virtual Module *clone() const;
};


class TestWalkerModule : public Module {
public:
  TestWalkerModule(  SuifEnv* suif );
  virtual LString get_module_name() const;

  virtual void execute();

  virtual Module *clone() const;

  static LString ClassName;
};




#endif

