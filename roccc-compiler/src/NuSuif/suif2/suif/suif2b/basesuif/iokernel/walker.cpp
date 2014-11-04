#include "common/system_specific.h"
#include "walker.h"
#include "cast.h"

Walker::Walker(SuifEnv *the_env)
	: _env(the_env),_is_pre_order(true),_address(0),_parent(0) {}

void Walker::set_post_order() {
    _is_pre_order = false;
    }

void Walker::set_pre_order() {
    _is_pre_order = true;
    }

bool Walker::get_is_pre_order() const {
    return _is_pre_order;
    }

bool Walker::is_walkable(Address address,bool is_owned,
			 const MetaClass *_meta) {
    return is_owned;
    }

bool Walker::is_visitable(Address address,const MetaClass *_meta) const
    {
    return true;
    }

void Walker::set_address(Address new_address) {
    _address = new_address;
    }

Address Walker::get_address() const {
    return _address;
    }

SuifEnv *Walker::get_env() const {
    return _env;
    }

void Walker::set_parent(Address addr) {
    _parent = addr;
    }

Address Walker::get_parent() const {
    return _parent;
    }

bool Walker::get_makes_changes() const {
    return false;
    }

bool Walker::is_changed(Address addr) const {
    return false;
    }

