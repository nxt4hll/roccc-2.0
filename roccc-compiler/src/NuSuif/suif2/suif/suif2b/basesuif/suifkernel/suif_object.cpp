#include "common/system_specific.h"
#include "suif_object.h"


#include "iokernel/aggregate_meta_class.h"
#include "iokernel/pointer_meta_class.h"
#include "iokernel/object_factory.h"
#include "iokernel/clone_stream.h"

#include "suif_env.h"
#include "io_subsystem.h"
#include "utilities.h"
#include "print_subsystem.h"

#include "common/formatted.h"

using namespace std;

SuifObject::SuifObject() :
  parent(0)
{}

void SuifObject::constructorFunction( Address instance ) {
     new (instance) SuifObject;
}


SuifEnv* SuifObject::get_suif_env() const {
  return get_object_factory()->get_suif_env();
}


SuifObject* SuifObject::deep_clone( SuifEnv* suif_env ) const {
  if ( !suif_env ) {
    suif_env = get_suif_env();
  }
  SuifObject* cloned_object = suif_env->get_clone_subsystem()->deep_clone( this );
  cloned_object -> set_parent( 0 );
  return cloned_object;
}


SuifObject* SuifObject::shallow_clone( SuifEnv* suif_env ) const {
  if ( !suif_env ) {
    suif_env = get_suif_env();
  }
  SuifObject* cloned_object = suif_env->get_clone_subsystem()->shallow_clone( this );
  cloned_object->set_parent( 0 );
  return cloned_object;
}

SuifObject* SuifObject::get_parent() const {
  return parent;
}

void SuifObject::set_parent( SuifObject* object ) {
  suif_assert_message((object == NULL) || (parent == object) || (parent == NULL),
		      ("Attempt to set parent of an object (%s) to a "
		       "(%s) that has not "
		       "already been disconnected from a (%s).\n",
		       getClassName().c_str(),
		       object->getClassName().c_str(),
		       parent->getClassName().c_str()
		       ));
  parent = object;
}

ObjectFactory* SuifObject::get_object_factory() const {
  return get_meta_class()->get_owning_factory();
}


void SuifObject::print_to_default() const {
  SuifEnv* suif_env = get_object_factory()->get_suif_env();
  suif_env->get_print_subsystem()->print( this );
}

String SuifObject::print_to_string() const {
  SuifEnv* suif_env = get_object_factory()->get_suif_env();
  return(suif_env->get_print_subsystem()->print_to_string( this ));
}

void SuifObject::print( ostream& output ) const {
  SuifEnv* suif_env = get_object_factory()->get_suif_env();
  suif_env->get_print_subsystem()->print( output, this );

}

void SuifObject::verify_invariants( ErrorSubSystem* message_destinations ) {

}


int SuifObject::replace( SuifObject* original,
			 SuifObject* new_object,
			 bool fuse_if_possible ) {
  int number_replacements = 0;
  ObjectFactory* of = get_object_factory();

  // get the 'basetype' of the objects that are investigated for replacement
  // in reality we are actually iterating over the pointers
  MetaClass* suif_mc = of->find_meta_class( SuifObject::get_class_name() );


  // ------- owned
  // now get the 'owned' pointer from the 'basetype'
  MetaClass* what = of->get_pointer_meta_class( suif_mc, true );

  // obtain an iterator iterating over all owned pointers
  Iterator* it = object_iterator_ut( ObjectWrapper(this), suif_mc, what );
  while ( it->is_valid() ) {
    if ( ((*(SuifObject**)it->current())) == original ) {
      // found the object
       ((*(SuifObject**)it->current())) = new_object;
	if (new_object && (new_object->get_parent() != this))
	    new_object->set_parent(this);
       number_replacements++;
    }
    it->next();
  }
  delete it;

  // if we have replaced any owned pointers, the original was owned by
  // this parent, so we must clear the parent field
  if ((number_replacements > 0) && original)
	original->set_parent(0);

  // ----- referenced
  what = of->get_pointer_meta_class( suif_mc, false );

  it = object_iterator_ut( ObjectWrapper(this), suif_mc, what );
  while ( it->is_valid() ) {
    if ( ((*(SuifObject**)it->current())) == original ) {
      // found the object
       ((*(SuifObject**)it->current())) = new_object;
       number_replacements++;
    }
    it->next();
  }
  delete it;

  return number_replacements;
}

void SuifObject::print(FormattedText &x) const {
  // Print something about an unregistered object.
  String name = "{Unregistered ";
  name += getClassName().c_str();
  name += "}";
  x.start_block(name);
  for (Iter<SuifObject> iter = collect_instance_objects<SuifObject>((SuifObject*)this);
       iter.is_valid(); iter.next()) 
    {
      SuifObject *child = &iter.current();
      child->print(x);
    }
  x.end_block();
}


static const LString suif_object_class_name = "SuifObject";

const LString &SuifObject::get_class_name() {return suif_object_class_name;}


SuifObject::SuifObject(const SuifObject &other) :
  parent(0)
{
  suif_assert(0);
}
SuifObject& SuifObject::operator=(const SuifObject &other) {
  parent = 0;
  suif_assert(0); return(*this);
}

Walker::ApplyStatus SuifObject::walk(Walker &w) {
     const MetaClass *_meta = get_meta_class();
     return _meta->walk(this,w);
     }

#include <stdio.h>
int printobj(SuifObject *obj) {
    FormattedText fd;
    obj->print(fd);
    printf("%s\n",(const char *)fd.get_value());
    return 0;
    }


bool is_kind_of_suif_object_meta_class(const MetaClass *meta) {
  while (meta && 
	 (meta->get_instance_name() != SuifObject::get_class_name()))
    {
      meta = meta->get_link_meta_class();
    } 
  return(meta != NULL);
}

