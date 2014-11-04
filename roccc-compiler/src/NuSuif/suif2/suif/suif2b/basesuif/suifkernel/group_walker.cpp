#include "common/system_specific.h"
#include "group_walker.h"
#include "iokernel/meta_class.h"
#include <stdio.h>

#ifndef NULL
#define NULL 0
#endif

//	These walkers allow you to select what objects are to be visited.
//
//

SelectiveWalker::SelectiveWalker(SuifEnv *the_env,const LString &the_type)
    : SuifWalker(the_env),type(the_type) {}

bool SelectiveWalker::is_visitable(Address address,
				   const MetaClass *_meta) const {
    while ((_meta != NULL) && (_meta->get_instance_name() != type))
        {
        _meta = _meta->get_link_meta_class();
        }
    return (_meta != NULL);
    }

const LString & SelectiveWalker::get_type() const {
    return type;
    }

GroupWalker::GroupWalker(SuifEnv *the_env)
    : SuifWalker(the_env), _vector(new TypeBasedDispatch<SelectiveWalker*>) {}

GroupWalker::~GroupWalker()
{
  delete _vector;
}

void GroupWalker::append_walker(SelectiveWalker &x) {
    const LString &type = x.get_type();
    _vector->set_value(type,&x);
    }

TypeBasedDispatchHelper::TypeBasedDispatchHelper() :
  _locked(false)
{
}

void TypeBasedDispatchHelper::set_value(const LString &id, void *val) {
  suif_assert_message(!_locked, ("TypeBasedDispatch must be completely"
		      " initialized before calling lookup_value()."));
    _vector[id.get_ordinal()] = val;
    }

void *TypeBasedDispatchHelper::lookup_value(SuifObject *x) {
    const MetaClass* meta = x->get_meta_class();
    void *w = NULL;
    while (meta) {
	const LString &type = meta->get_instance_name();
	int index = type.get_ordinal();
	if (index <= 0)
	    return 0;
	w = _vector[index];
	if (w != NULL)
	    break;
	meta = meta->get_link_meta_class();
	}

    if (w) {
 	// record this type and its parents in the vector to
	// reduce future searching
	meta = x->get_meta_class();
	while (meta != NULL) {
	    const LString &type = meta->get_instance_name();
            int index = type.get_ordinal();
            if (index <= 0)
	        break;
	    if (_vector[index])
	        break;
	    _vector[index] = w;
	    meta = meta->get_link_meta_class();
            }
	}
    return w;
    }

Walker::ApplyStatus GroupWalker::operator () (SuifObject *x) {
    SelectiveWalker *w = _vector->lookup_value(x);
    if (!w)
        return Walker::Continue;
    Walker::ApplyStatus status = (*w)(x);
    if (status == Replaced)
	set_address(w->get_address());
    return status;
    }

ReplacingWalker::ReplacingWalker(SuifEnv *the_env)
    : GroupWalker(the_env) {}

Walker::ApplyStatus ReplacingWalker::operator () (SuifObject *x) {
    Walker::ApplyStatus status = GroupWalker::operator()(x);
    if (status == Replaced) {
	SuifObject *y = (SuifObject *)get_address();
	_map.enter_value(x,y);
	SuifObject *parent = (SuifObject *)get_parent();
	if (parent) {
	    parent->replace(x,y);
	    }
	}
    return status;
    }

bool ReplacingWalker::is_walkable(Address address, bool is_owned,const MetaClass *_meta) /* const */{
    suif_hash_map<Address,Object *>::iterator iter = _map.find(address);
    if (iter == _map.end()) {
	_map.enter_value(address,0);
	return true;
	}
    if ((*iter).second) {
	SuifObject *parent = (SuifObject *)get_parent();
        if (parent) {
            parent->replace((SuifObject *)address,(SuifObject *)(*iter).second);
            }
	return false;
	}
    return false;
    }

bool ReplacingWalker::is_changed(Address addr) const {
    suif_hash_map<Address,Object *>::iterator iter = _map.find(addr);
    if (iter == _map.end()) {
        return false;
        }
    if ((*iter).second == 0)
	return false;
    // substitute
    SuifObject *parent = (SuifObject *)get_parent();
    if (parent) {
        parent->replace((SuifObject *)addr,(SuifObject *)(*iter).second);
        }
    return true;
    }
	
Object *ReplacingWalker::get_replacement(Object *x) const {
    suif_hash_map<Address,Object *>::const_iterator iter = _map.find(x);
    if (iter == _map.end())
        return 0;
    return (*iter).second;
    }

void ReplacingWalker::add_replacement(Object *from,Object *to) {
    _map.enter_value(from,to);
    }

bool ReplacingWalker::get_makes_changes() const {
    return true;
    }
