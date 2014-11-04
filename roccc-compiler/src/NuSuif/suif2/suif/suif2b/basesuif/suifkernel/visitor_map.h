#ifndef IOKERNEL__VISITORMAP_H
#define IOKERNEL__VISITORMAP_H

#include "iokernel/object.h"
#include "iokernel/iokernel_forwarders.h"

/*!
 * \class VisitorMap
 * \brief dynamically-dispatched visitor for SuifObject objects.
 *
 * A dynamically-constructed visitor for SuifObjects, which 
 * dispatches on the most specific 
 * 
 * \par VisitorMap example
 * To use this visitor map:
 *
 * \code
 * // Write your visitor state class
 *
 * class MyVisitorInfo {
 *   static void do_static_procedure_definition( MyVisitorInfo *info,
 *                                               ProcedureDefinition *obj ) {
 *       info->do_procedure_definition(obj);
 *   }
 *   void do_procedure_definition(ProcedureDefinition *proc_def) {
 *     ...do_something...
 *   }
 * }
 * \endcode
 * \code
 * // register the visitor functions
 * VisitorMap *map = new VisitorMap(suif_env);
 * MyVisitorClass my_info;
 * map->registerVisitMethod( &myinfo, 
 *                           &MyVisitorClass::do_static_procedure_definition,
 *                           ProcedureDefinition::get_class_name() );
 * \endcode
 * \code
 * // Pick an object:
 * //  SuifObject *so;
 * map->apply(so);
 * \endcode
 *
 */

/**
 * \class VisitorMap visitor_map.h suifkernel/visitor_map.h
 * The VisitorMap calls functions with the VisitMethod
 * prototype.  The first argument is usually a state class
 * The second argument is the Object that was dispatched
 */
typedef void ( * VisitMethod )( Address state, Object * object );


/**
 * \class VisitorEntry
 * \internal
 * Container for VisitMethod entries.
 */

class VisitorEntry {
public:
    Address     stateAddress;
    VisitMethod visitMethod;
public:
    VisitorEntry(Address address, VisitMethod method) :
	stateAddress(address), visitMethod(method) {}
    VisitorEntry() :
	stateAddress(0), visitMethod(0) {}
    VisitorEntry(const VisitorEntry &other) :
	stateAddress(other.stateAddress), visitMethod(other.visitMethod) {}
    VisitorEntry& operator=(const VisitorEntry &other) {
	stateAddress = other.stateAddress;
	visitMethod = other.visitMethod;
	return(*this);
    }
    bool operator==(const VisitorEntry &other) {
	return ((visitMethod == other.visitMethod) &&
		(stateAddress == other.stateAddress));
    }
    bool operator!=(const VisitorEntry &other) {
	return ((visitMethod != other.visitMethod) ||
		(stateAddress != other.stateAddress));
    }
};

/*
 * \par VisitorMap example
 * To use this visitor map:
 *
 * \code
 * // Write your visitor state class
 *
 * class MyVisitorInfo {
 *   static void do_static_procedure_definition( MyVisitorInfo *info,
 *                                               ProcedureDefinition *obj ) {
 *       info->do_procedure_definition(obj);
 *   }
 *   void do_procedure_definition(ProcedureDefinition *proc_def) {
 *     ...do_something...
 *   }
 * }
 * \endcode
 * \code
 * // register the visitor functions
 * VisitorMap *map = new VisitorMap(suif_env);
 * MyVisitorClass my_info;
 * map->registerVisitMethod( &myinfo, 
 *                           &MyVisitorClass::do_procedure_definition,
 *                           ProcedureDefinition::get_class_name() );
 * \endcode
 * \code
 * // Pick an object:
 * //  SuifObject *so;
 * map->apply(so);
 * \endcode
 *
 */

template <class T> class CascadingMap;

class VisitorMap {
public:
    VisitorMap( SuifEnv* suif );
    ~VisitorMap();

    void register_visit_method( Address state, 
				VisitMethod visitMethod, 
				const LString &className );

    void register_unknown_method( Address state, 
				  VisitMethod visitMethod );
    
    void apply( Object* object );

    // more low level
    void apply( Object* object, const MetaClass* metaClass );
    void register_visit_method( Address state, 
				VisitMethod visitMethod, 
				MetaClass* mc );

private:
    CascadingMap<VisitorEntry> *map;
    Address unknownState;
    VisitMethod unknownMethod;
private:
    VisitorMap(const VisitorMap &other);
    VisitorMap& operator=(const VisitorMap &other);
};

#endif
