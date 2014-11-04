#ifndef SUIFKERNEL__UTILITIES_H
#define SUIFKERNEL__UTILITIES_H

#include "iokernel/object_factory.h"
#include "suifkernel/suif_object.h"

#include "suifkernel_forwarders.h"
#include "suif_env.h"

#include "iter.h"
/**
 * \file utilities.h
 * 
 * Templates for iterating over subsets of SuifObjects
 */

// --------------- type less accessor functions ------------
sf_owned Iterator* object_iterator_ut( const ObjectWrapper &start_object,
				       const MetaClass* what );

sf_owned Iterator* object_iterator_ut( const ObjectWrapper &start_object,
				       const MetaClass* dont_search_beyond,
				       const MetaClass* what );

sf_owned Iterator* object_ref_iterator_ut( SuifObject *start_object,
					   const MetaClass* what );

/**
 * \class object_iterator utilities.h suifkernel/utilities.h
 * Build an Iterator
 * that will walk over all Owned objects in
 * the ownership tree rooted at the start_object
 * 
 * \par Example Usage
 *
 * \code
 * for (Iter<CallStatement> iter = 
 *      object_iterator<CallStatement>(file_set_block);
 *      iter.is_valid(); iter.next()) 
 *   {
 *      CallStatement *call = &iter.current();
 *      // do something with the call
 *   }
 * \endcode
 *
 * \par WARNING
 * even though this iterator will give you access to a
 * reference to the desired object, it is a VERY BAD idea
 * to try to modify the object during iteration.  Instead
 * consider using the collect_objects<> to build
 * a list of all the objects to operate on, then make
 * transformations
 */

// -------------- typed accessor functions ----------------
template<class T>
Iter<T> object_iterator( const SuifObject* start_object ) {
  //fprintf(stderr,"[JUL] --- utilities.h@object_iterator ---\n");
  Iterator* it = 0;
  
  if ( start_object ) {
    MetaClass* what = start_object->get_object_factory()->
                           find_meta_class( T::get_class_name() );
    it = object_iterator_ut( ObjectWrapper(start_object),
			     what );
  }
  Iter<T> i=Iter<T>( it );
  //fprintf(stderr,"[JUL] utilities.h@object_iterator::%x  %x \n",it,&i);
  return i;
}


/**
 * \class collect_objects utilities.h suifkernel/utilities.h
 * Build a list of all Owned objects in the ownership 
 * tree rooted at the start_object 
 * that subclass from the given type
 * 
 * \par Example Usage
 *
 * \code
 * list<ProcedureSymbol*>* sym_list = collect_object<ProcedureSymbol>(root);
 * for (list<ProcedureSymbol*>::iterator iter = sym_list->begin();
 *      iter != sym_list->end(); iter++)
 *   {
 *     ProcedureSymbol *ps = *iter;
 *     // do something with the procedure symbol
 *   }
 * // clean up
 * delete sym_list;
 * \endcode
 *
 * \par NOTES
 * Use this iterator to collect all of the objects that
 * subclass from the given type. You can then use the
 * and modify the returned objects after collecting the objects.
 */

template<class T>
list<T*>* collect_objects( const SuifObject* start_object ) {
  list<T*>* l = new list<T*>;
  if ( start_object ) {
    MetaClass* what = start_object->get_object_factory()->
                           find_meta_class( T::get_class_name() );

    Iterator* it = object_iterator_ut( ObjectWrapper(start_object),
				       what );
    while ( it->is_valid() ) {
      suif_assert( it->current_meta_class() == what );
      l->push_back( (T*)it->current() );
      it->next();
    }
    delete it;
  }
  return l;
}

/**
 * \class collect_instance_objects utilities.h suifkernel/utilities.h
 * Build an Iterator
 * that will walk over all of the immediate child SuifObjects 
 * that are subtypes of the template type that are OWNED 
 * by this SuifObject.
 * 
 * \par Example Usage
 *
 * \code
 * for (Iter<Expression> iter = 
 *      collect_instance_objects<Expression>(statement);
 *      iter.is_valid(); iter.next()) 
 *   {
 *      Expression *exp = &iter.current();
 *      // do something with the OWNED expression
 *   }
 * \endcode
 *
 * \par NOTES
 * The above example is similar to the
 * Statement::get_expression_iterator() method for Statements.
 * This iterator does not dive into the entire ownership tree
 * rooted at the start_object
 */


  

template<class T>
Iter<T> collect_instance_objects( SuifObject* start_object,
                                   const LString& start_class = emptyLString,
                                   const LString& end_class = emptyLString ) {
  Iterator* it = 0;
  if ( start_object ) {
    ObjectFactory* of = start_object->get_suif_env()->get_object_factory();

    MetaClass* what = of->find_meta_class( T::get_class_name() );
    suif_assert( what );

    MetaClass* dont_search_beyond = of->find_meta_class( SuifObject::get_class_name() );
    suif_assert( dont_search_beyond );

    it = object_iterator_ut( ObjectWrapper(start_object),
			     dont_search_beyond,
			     what );
  }
  return Iter<T>( it );
}

template<class T>
Iter<T> suif_subobject_iterator( SuifObject* start_object,
				 const LString& start_class = emptyLString,
				 const LString& end_class = emptyLString ) {
  return(collect_instance_objects<T>(start_object, start_class, end_class));
}

/**
 * \class suif_object_ref_iterator utilities.h suifkernel/utilities.h
 * Build an Iterator
 * that will walk over all of the immediate child SuifObjects 
 * that are subtypes of the template type that are REFERENCED
 * (not OWNED) by this SuifObject.
 * 
 * \par Example Usage
 *
 * \code
 * for (Iter<Type> iter = 
 *      suif_object_ref_iterator<Type>(expression);
 *      iter.is_valid(); iter.next()) 
 *   {
 *      Type *t = &iter.current();
 *      // do something with the referenced type.
 *   }
 * \endcode
 *
 * \par NOTES
 * This can be used with an iterator over the ownership
 * tree like the object_iterator<>() to find all the
 * SuifObject links in a system
 */

template<class T>
Iter<T> suif_object_ref_iterator( SuifObject* start_object,
				  const LString& start_class = 
				  SuifObject::get_class_name()) {
  // ask a stupid question
  if (!start_object) return(NULL);

  ObjectFactory* of = start_object->get_suif_env()->get_object_factory();
  
  MetaClass* what = of->find_meta_class( T::get_class_name() );
  suif_assert( what );
  // the stop object BETTER be a subclass of suif object.
  suif_assert(is_kind_of_suif_object_meta_class(what));
  Iterator *it = object_ref_iterator_ut( start_object,
					 what );
  return Iter<T>( it );
}


#endif
