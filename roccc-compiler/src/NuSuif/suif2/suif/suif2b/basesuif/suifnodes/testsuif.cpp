#include "common/system_specific.h"
#include "Forwarders.h"
#include "ModuleSubSystem.h"
#include "Module.h"
#include "SuifStore.h"
#include "assert.h"
#include "SuifObject.h"
#include "suif.h"

class TestSuifModule : public Module {
public:
  TestSuifModule( Suif* suif ) : Module( suif ) {
     moduleName = LString("testsuif");
  }

  virtual void execute( suif_vector<String>* commandLine ) {
    cerr<<"EXECUTE TEST SUIF"<<endl;
    SuifObjectFactory* sf = (SuifObjectFactory*)suif->getObjectFactory("suif");
    assert( sf );
    suif->setRootNode( sf->createIfStatement( 0, 0, 0 ) );
  }

};



extern "C" void EXPORT init_testsuif( Suif* suif ) {
  ModuleSubSystem* mSubSystem = suif->getModuleSubSystem();
  if (!mSubSystem->retrieve_module("suif")) {
    mSubSystem -> register_module( new TestSuifModule( suif ) );
    }

}
