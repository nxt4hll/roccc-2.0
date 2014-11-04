#ifndef IOKERNEL__CASCADINGMAP_H
#define IOKERNEL__CASCADINGMAP_H

#include "iokernel/object.h"
#include "iokernel/iokernel_forwarders.h"
#include "iokernel/object_factory.h"
#include "iokernel/meta_class.h"
#include "suif_env.h"
#include "common/suif_vector.h"

/*!
 * \class CascadingMap
 * \brief lookup table template <class T> mapping SuifObject to data of type T.
 *
 * A lookup table with data of type T, keyed by SuifObject, which
 * selects the most specific entry with a key which is a class or
 * superclass of a given object.
 * 
 * requires: T is a value class, has assignment, !=
 *
 * \par CascadingMap example
 * To use this cascading map:
 *
 * \code
 * // Have some data;
 *
 * \endcode
 * \code
 * // register the data
 * CascadingMap<char> *map = new CascadingMap<char>(suif_env);
 * char *my_info1 ="procedure";
 * char *my_info2 ="statement";
 * map->assign( ProcedureDefinition::get_class_name(), my_info1 );
 * map->assign( Statement::get_class_name(), my_info2 );
 *                           
 *                           
 * \endcode
 * \code
 * // Pick an object:
 * //  SuifObject *so;
 * map->lookup(so);
 * // returns my_info1 for any procedure definition,
 * // my_info2 for any statement (or subclass)
 * // 0 for anything else
 * \endcode
 * */

template < class T >
class CascadingMap {
public:
    CascadingMap( SuifEnv* suif, T defaultvalue );
    ~CascadingMap();
    
    void assign( const LString &className, T data);
			 
    // returns defaultvalue if not found;
    T lookup( const Object* object );
    
    // low level interface
    T lookup( const MetaClass* metaClass );
    void assign( MetaClass* mc, T newdata );
    
private:
    T defaultvalue;
    suif_vector<T>* entries;
    SuifEnv* suif;
    suif_vector<bool>* cached;
    suif_vector<bool>* valid;
    void clear_cache();
    void expand_to(MetaClassId to_id);
private:
    CascadingMap(const CascadingMap &other);
    CascadingMap& operator=(const CascadingMap &other);
};


class SuifEnv;

template < class T >
CascadingMap<T>::CascadingMap( SuifEnv* suif_env, T defaultvalue0 ) :
    defaultvalue(defaultvalue0), entries(0), suif(suif_env), cached(0),
    valid(0)
{
    //  this->suif = suif;
    unsigned num = suif->get_object_factory()->get_last_id();
    entries = new suif_vector<T>( num );
    cached = new suif_vector<bool>( num );
    valid = new suif_vector<bool>( num );
    for (unsigned i = 0; i < num; i++) {
	(*entries)[i] = defaultvalue;
	(*cached)[i] = false;
	(*valid)[i] = false;
    }
}

template< class T >
void CascadingMap<T>::clear_cache() {
    unsigned num = cached->size();
    for (unsigned i = 0; i < num; i++) {
	(*cached)[i] = false;
    }
}

template < class T >
CascadingMap<T>::~CascadingMap() {
    delete entries;
    delete cached;
    delete valid;
}

template < class T >
void CascadingMap<T>::assign( const LString &className,
			      T data ) {
  MetaClass *mc = suif->get_object_factory()->lookupMetaClass( className );
  suif_assert_message( mc, 
		       ("No metaclass registered for %s."
			"Perhaps another dll is required.",
			className.c_str() ));
  assign(mc, data);
}

template < class T >
void CascadingMap<T>::expand_to( MetaClassId id ) {
  while( id >= (MetaClassId)entries->size()){
    entries->push_back(defaultvalue);
    valid->push_back(false);
    cached->push_back(false);
  }
  return;
}


template < class T >
void CascadingMap<T>::assign( MetaClass* mc, T data ) {
    clear_cache();
    suif_assert_message( mc, 
			 ("Cascading Map can not Map the NULL MetaClass\n"));
    MetaClassId id = mc->get_meta_class_id();
    
    expand_to(id);

    (*entries)[id] = data;
    (*valid)[id] = true;
}

template < class T >
T CascadingMap<T>::lookup( const Object* object ) {
    return lookup( object->get_meta_class() );
}

template < class T > 
T CascadingMap<T>::lookup( const MetaClass* mc ) {
    MetaClassId id = mc->get_meta_class_id();
    expand_to(id);
    
    if ((*valid)[id] || (*cached)[id]) {
	return (*entries)[ id ];
    } else {
	MetaClass *parent = mc->get_link_meta_class();
	if (parent) {
	    T res = lookup(parent);
	    (*entries)[id] = res;
	    (*cached)[id] = true;
	    return res;
	} else {
	    (*entries)[id] = defaultvalue;
	    (*cached)[id] = true;
	    return defaultvalue;
	}
    }
}

#endif
