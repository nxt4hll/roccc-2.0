#include "common/system_specific.h"
#include "walking_maps.h"

#include "visitor_map.h"

#include "suif_object.h"
#include "utilities.h"
#include "common/suif_list.h"


// Here are the default
// functions for SuifObjects

static void handle_static_process_suif_object(WalkingMaps *walk,
				SuifObject *obj) {
  if (walk->is_done()) return;
  walk->get_pre_map()->apply(obj);
  // On skip, don't continue on this object
  if (walk->is_skip()) {
    walk->set_skip(false);
    return;
  }

  if (walk->is_done()) return;
  walk->get_children_map()->apply(obj);
  if (walk->is_done()) return;
  walk->get_post_map()->apply(obj);
  if (walk->is_done()) return;
}


static void handle_static_pre_suif_object(WalkingMaps *walk,
			    SuifObject *obj) {
  return;
}
static void handle_static_children_suif_object(WalkingMaps *walk,
					SuifObject *obj) {
  // use the object iterator to find all suiffobjects.
  // check to see that the parent is ME
  for (Iter<SuifObject> iter =
	 collect_instance_objects<SuifObject>(obj);
       iter.is_valid(); iter.next()) {
    //  list<SuifObject *>::iterator iter = the_list->begin();
    //  for (; iter != the_list->end(); iter++) {
    SuifObject *child = &iter.current();

    if (child == 0 ) continue;
    suif_assert(child->get_parent() == obj);
    walk->get_process_map()->apply(child);
    if (walk->is_done()) return;
  }
}

static void handle_static_post_suif_object(WalkingMaps *walk,
					   SuifObject *obj) {
  return;
}



WalkingMaps::WalkingMaps(
			 SuifEnv *suif_env,
			 const LString &walking_maps_name,
			 //VisitorMap *pre_map,
			 //VisitorMap *children_map,
			 //VisitorMap *post_map,
			 //VisitorMap *process_map,
			 Address user_state
			 ) :
  _walking_maps_name(walking_maps_name),
  _done(false),
  _skip(false),
  _pre_map(new VisitorMap(suif_env)),
  _children_map(new VisitorMap(suif_env)),
  _post_map(new VisitorMap(suif_env)),
  _process_map(new VisitorMap(suif_env)),
  _user_state(user_state) {};

WalkingMaps::~WalkingMaps(){
  delete _pre_map;
  delete _children_map;
  delete _post_map;
  delete _process_map;
}

LString WalkingMaps::get_walking_maps_name() const { return _walking_maps_name; }

void WalkingMaps::set_user_state(Address user_state) {
  _user_state = user_state;
}
Address WalkingMaps::get_user_state() const {
  return(_user_state);
}

void WalkingMaps::init_suif_object() {
  _pre_map->register_visit_method((Address) this,
				  (VisitMethod)handle_static_pre_suif_object,
				  SuifObject::get_class_name());
  _children_map->register_visit_method((Address) this,
				       (VisitMethod)handle_static_children_suif_object,
				       SuifObject::get_class_name());
  _post_map->register_visit_method((Address) this,
				   (VisitMethod)handle_static_post_suif_object,
				   SuifObject::get_class_name());
  _process_map->register_visit_method((Address) this,
				      (VisitMethod)handle_static_process_suif_object,
				      SuifObject::get_class_name());
}

void WalkingMaps::register_pre_visit_method(VisitMethod func,
					    const LString &name) {
  _pre_map->register_visit_method((Address) this, func, name);
}

void WalkingMaps::register_children_visit_method(VisitMethod func,
					    const LString &name) {
  _children_map->register_visit_method((Address) this, func, name);
}

void WalkingMaps::register_post_visit_method(VisitMethod func,
					    const LString &name) {
  _post_map->register_visit_method((Address) this, func, name);
}

void WalkingMaps::register_process_visit_method(VisitMethod func,
					    const LString &name) {
  _process_map->register_visit_method((Address) this, func, name);
}

void WalkingMaps::process_a_suif_object(SuifObject *so) {
  get_process_map()->apply(so);
}

VisitorMap *WalkingMaps::get_pre_map() const { return _pre_map; }
VisitorMap *WalkingMaps::get_children_map() const { return _children_map; }
VisitorMap *WalkingMaps::get_post_map() const { return _post_map; }
VisitorMap *WalkingMaps::get_process_map() const { return _process_map; }

void WalkingMaps::set_done(bool val) {_done = val; }
bool WalkingMaps::is_done() const { return(_done); }

void WalkingMaps::set_skip(bool val) {_skip = val; }
bool WalkingMaps::is_skip() const { return(_skip); }


WalkingMaps::WalkingMaps(const WalkingMaps &other) :
  _done(false), _skip(false), _pre_map(0),
  _children_map(0), _post_map(0), _process_map(0),
  _user_state(0)
{
  suif_assert(false);
}
WalkingMaps& WalkingMaps::operator=(const WalkingMaps &other) {
  suif_assert(false); return(*this);
}
