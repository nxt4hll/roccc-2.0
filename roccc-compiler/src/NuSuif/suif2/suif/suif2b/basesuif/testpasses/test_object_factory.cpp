#include "common/system_specific.h"
#include "test_object_factory.h"

#include "iokernel/object_factory.h"
#include "iokernel/object.h"
#include "iokernel/meta_class.h"
#include "iokernel/aggregate_meta_class.h"
#include "iokernel/stl_meta_class.h"
#include "iokernel/pointer_meta_class.h"
#include "suifkernel/suifkernel_forwarders.h"
#include "suifkernel/suif_env.h"

#include "test_objects.h"

#include <assert.h>
#include <iostream.h>

#include "common/suif_vector.h"
#include "common/suif_map.h"


TestObjectFactory::TestObjectFactory() {
  //  testXMetaClass = testYMetaClass = pointerMetaClass = 0;
}


void TestObjectFactory::init( SuifEnv* suif ) {
  _object_factory = suif->get_object_factory();
  ObjectFactory *of = _object_factory;

  MetaClass* intMetaClass = of->lookupMetaClass("int");
  assert( intMetaClass );
  AggregateMetaClass* meta_SuifObject = of->find_aggregate_meta_class(SuifObject::get_class_name());

  AggregateMetaClass* testX = 0;
  testX = of->create_object_meta_class( TestX::get_class_name(),
                                        sizeof( TestX ),
                                        TestX::constructorFunction,
                                        meta_SuifObject );
  PointerMetaClass* owningPointerToTestX = of->get_pointer_meta_class( testX, true );


  testX-> add_field_description( "i", intMetaClass, OFFSETOF(TestX,i) );
  testX-> add_field_description( "x1", owningPointerToTestX, OFFSETOF(TestX,x1) );
  testX-> add_field_description( "x2", owningPointerToTestX, OFFSETOF(TestX,x2) );

  STLMetaClass* meta_TestX_vector =of-> get_stl_meta_class("LIST:suif_vector<TestX*>",
                               new STLDescriptor< suif_vector<TestX*> >( owningPointerToTestX ) );
  PointerMetaClass* meta_TestX_owner = of->get_pointer_meta_class(meta_TestX_vector,true,true);

  testX-> add_field_description( "_vector", meta_TestX_owner, OFFSETOF(TestX,_vector) );


  testX->add_virtual_field( "int", "i" );
  testX->add_virtual_field( "pointers", "x1;x2" );
  testX->add_virtual_field( "TestX", "x1/*;x2/*" );
  testX->add_virtual_field( "int-indirect", "i;x1/i;x2/i" );
  testX->add_virtual_field( "vectorPtr", "_vector" );
  testX->add_virtual_field( "vector", "_vector/*" );
  testX->add_virtual_field( "vectorContents", "_vector/*/*" );

  // ------------ make meta class for TestY -------------
  AggregateMetaClass* testY = 0;

  testY = of->create_object_meta_class( TestY::get_class_name(),
                                        sizeof( TestY ),
                                        TestY::constructorFunction,
                                        testX );
  testY-> add_field_description( "x3", owningPointerToTestX, OFFSETOF(TestY,x3) );


  testY->add_virtual_field( "int", "i" );
  testY->add_virtual_field( "pointers", "^;x3" );
  testY->add_virtual_field( "TestX", "^;x3/*" );

}


TestX* TestObjectFactory::createTestX( int i, TestX* x1, TestX* x2 ) {
       MetaClass* m = _object_factory->lookupMetaClass( TestX::get_class_name() );
       assert( m );
       TestX* x = new TestX();
       x->set_meta_class( m );
       x->setInt1( i );
       x->setX1( x1 );
       x->setX2( x2 );
       return x;
}


TestY* TestObjectFactory::createTestY( int i, TestX* x1, TestX* x2, TestX* x3 ) {
       MetaClass* m = _object_factory->lookupMetaClass( TestY::get_class_name() );
       TestY* y= new TestY();
       y->set_meta_class( m );
       y->setInt1( i );
       y->setX1( x1 );
       y->setX2( x2 );
       y->setX3( x3 );
       return y;
}






static LString testObjectFactoryName("TestObjectFactory");

const LString& TestObjectFactory::getName() {
  return testObjectFactoryName;
}















