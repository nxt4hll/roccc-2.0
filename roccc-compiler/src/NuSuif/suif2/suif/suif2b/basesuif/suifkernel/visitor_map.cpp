#include "common/system_specific.h"
#include "visitor_map.h"
#include "iokernel/iokernel_forwarders.h"
#include "iokernel/object_factory.h"
#include "iokernel/meta_class.h"
#include "suif_env.h"
#include "common/suif_vector.h"

#include "cascading_map.h"

class SuifEnv;

VisitorMap::VisitorMap( SuifEnv* suif_env ) :
    map(new CascadingMap<VisitorEntry>(suif_env, VisitorEntry(0, 0))),
    unknownState(0), unknownMethod(0)
{
}

VisitorMap::~VisitorMap() {
    delete map;
}

void VisitorMap::register_visit_method( Address stateAddress,
					VisitMethod visitMethod, 
					const LString &className ) {
    map->assign(className, VisitorEntry(stateAddress, visitMethod));
}

void VisitorMap::register_visit_method( Address stateAddress, 
					VisitMethod visitMethod, 
					MetaClass* mc ) {
    map->assign(mc, VisitorEntry(stateAddress, visitMethod));
}

void VisitorMap::apply( Object* object ) {
    VisitorEntry entry = map->lookup(object);
    if (entry.visitMethod) {
		entry.visitMethod( entry.stateAddress, object );
    } else {
	if (unknownMethod) 
	    unknownMethod( unknownState, object );
    }
    return;
}

void VisitorMap::apply( Object * object, const MetaClass* metaClass ) {
    VisitorEntry entry = map->lookup(metaClass);
    if (entry.visitMethod) {
	entry.visitMethod( entry.stateAddress, object );
    } else {
	if (unknownMethod) 
	    unknownMethod( unknownState, object );
    }
    return;
}

void VisitorMap::register_unknown_method( Address stateAddress, 
					  VisitMethod visitMethod ) {
    unknownState = stateAddress;
    unknownMethod = visitMethod;
}


