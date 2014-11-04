#include "common/system_specific.h"
#include "synchronizer.h"
#include "clone_stream.h"
#include "meta_class.h"
#include "object_factory.h"
#include "helper.h"
#include "cast.h"

#include "common/suif_list.h"
#include "common/suif_map.h"
#include "list_meta_class.h"


// #ifdef PGI_BUILD
#include <new>
// #else
// #include <new.h>
// #endif

#ifdef DEBUG
#include <iostream>
using namespace std;
#endif


Synchronizer::Synchronizer(ObjectFactory *factory) :
  CloneStream(factory),
  _system_factory(0), _new_object_factory(0)
{
    set_pointer_handling(ENQUIRE_PTR,ENQUIRE_PTR,CLONE_PTR);
}


Synchronizer::~Synchronizer() {
}

class SynchronizerApplier : public MetaClassApplier {
    public:
	SynchronizerApplier(Synchronizer *syn);
	virtual bool operator () (MetaClass *x);
    private:
	Synchronizer *_syn;
    };

SynchronizerApplier::SynchronizerApplier(Synchronizer *syn) : _syn(syn) {}

bool SynchronizerApplier::operator () (MetaClass *x) {
    const MetaClass *m = ((Object *)x)->get_meta_class();
    // 
    MetaClass *new_class = _syn->get_replacement(x);
    if (new_class == NULL) 
        _syn->push_for_clone(x,m);
    else if (new_class != x)
	_syn->push_for_replacement(x,new_class);
    return true;
    }

class FixupOrderedList : public MetaClassApplier {
    public:
	virtual bool operator () (MetaClass *x);
	FixupOrderedList(Synchronizer *x);
    private:
	Synchronizer *_syn;
	suif_map<MetaClass *,bool> _visited;
    };

FixupOrderedList:: FixupOrderedList(Synchronizer *x) : _syn(x) {
    }

bool FixupOrderedList::operator () (MetaClass *x) {
    if (_syn->get_is_cloned(x)) {
	suif_map<MetaClass *,bool>::iterator iter = _visited.find(x);
	if (iter == _visited.end()) {
	    _visited.enter_value(x,true);
	    x->walk_referenced_meta_classes(this);
	    x->adjust_field_offsets();
	    }
	}
    return true; 
    }

void Synchronizer::synchronize( ObjectFactory* system_factory,
                                ObjectFactory* new_object_factory,
                                InputStream* input_stream ) {
    _system_factory=system_factory;
    _new_object_factory=new_object_factory;
    _input_stream = input_stream;

#ifdef DEBUG
    cerr<<"--- START SYNCHRONIZE -----"<<endl;
    cerr<<"========> NEW:"<<endl;
    _new_object_factory->print_contents();
    cerr<<"========> OLD:"<<endl;
    _system_factory->print_contents();
#endif
    SynchronizerApplier synapp(this);

    open();
    _new_object_factory->apply_to_metaclasses(&synapp);

    perform_cloning();

    // now we need to do some fixup for cloned objects
    // First we need to enter them in the meta class table for the 
    // system factory as these are new objects.
    // Second we need to synchronize the sizes of the objects

    int clone_count = get_clone_count();

    FixupOrderedList ordered(this);

    read_close(); // do this first to initialize objects

    for (int i = 0;i < clone_count; i++) {
	MetaClass *m = (MetaClass*)get_new_address(i);
	const MetaClass *obj_metaclass = get_meta_class(i);
	if (obj_metaclass->object_is_kind_of(MetaClass::get_class_name())
	    && get_is_cloned(i)) {
            _system_factory->enter_meta_class( m);
	    input_stream->remap_address(get_old_address(i),m);
	    m->set_constructor_function(0); // because it is bogus read in value
            ordered(m);
	    }
	else if (!get_is_cloned(i) && (m != NULL) && (ObjectFactory::is_a_list( m ))) {
	    ListMetaClass *lm = to<ListMetaClass>(m);

            MetaClass* element_meta_class = lm->get_element_meta_class();

  	    // The element meta class may not be that in the system table. We
	    // must backpatch in that case.
	    Address new_addr = clone_address(element_meta_class);
	    if (new_addr != 0) {
	        element_meta_class = (MetaClass *)(new_addr);
		ordered(element_meta_class);
		lm->set_element_meta_class(element_meta_class);
                }
	    }
	}
    close();
    }

MetaClass * Synchronizer::get_replacement(Object *x) const {
    MetaClass *m = to<MetaClass>(x);
    MetaClass *new_class = NULL;

    // look for matching node. Special case: if this object is a list we
    // need a generic list instead. We enter that in the system table
    // immediately. Later we will have to patch the element pointer since
    // it can change
    if (ObjectFactory::is_a_list( m )) {
        ListMetaClass *lm = to<ListMetaClass>(m);
        MetaClass* element_meta_class = lm->get_element_meta_class();
	MetaClass *new_element_meta_class =
		_system_factory->lookupMetaClass(
			element_meta_class->get_instance_name() );
	if (new_element_meta_class)
	    element_meta_class = new_element_meta_class;
	size_t old_size = lm->get_size();
        lm = _system_factory->get_list_meta_class( element_meta_class,lm->get_instance_name());
 	if (old_size != lm->get_size()) {
	  if (lm->has_constructed_object()) {
	    printf("List class became %s:%d was of size %d\n",(const char *)lm->get_instance_name(),lm->get_size(),old_size);
	    }
	  }
        new_class = lm;
        }
    else {
        new_class = _system_factory->lookupMetaClass( m->get_instance_name() );
        }
    if (new_class && (new_class != m))
	_input_stream->remap_address(m,new_class);
    return new_class;
    }

void Synchronizer::object_enquiry(Object *x,CloneStreamObjectInstance *o,PTR_TYPE ptr_type) {
    
    MetaClass *new_class = get_replacement(x);
    if (new_class == NULL) {// non found so we clone it 
	set_clone_object(o);
	}
    else if (new_class == x) {
	set_reference(o);
	}
    else {
	set_replacement(o,new_class);
	}
    }

ObjectFactory* Synchronizer::get_object_factory() const{
  return _system_factory;
}

Synchronizer& Synchronizer::operator=(const Synchronizer&) {
  kernel_assert(false);
  return(*this);
}
Synchronizer::Synchronizer(const Synchronizer&)  :
  CloneStream(0),_system_factory(0), _new_object_factory(0)
{
  kernel_assert(false);
}

