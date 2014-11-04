#ifndef SUIFKERNEL__WALKING_MAPS_H
#define SUIFKERNEL__WALKING_MAPS_H

#include "iokernel/iokernel_forwarders.h"
#include "visitor_map.h"


class VisitorMap;

/** A combination of 4 VisitorMaps 
 *
 * <UL>
 *   <LI>
 *   "process" : decide whether or not to walk over a SuifObject
 *   <LI>
 *   "pre" : dispatch before walking over the children of a SuifObject
 *   <LI>
 *   "children" : choose the order of walking over the children
 *   <LI>
 *   "post" : dispatch after walking over the children of a SuifObject
 * </UL>
 *
 * The default implementation of these will walk over the
 * whole rooted tree.
 *
 * This allows the user to have total control over 
 * the process of walking over a tree of SuifObjects
 * It is used by some flow insensitive analyses that
 * need to control their walking order
 * These can be registered in the WalkingMaps subsystem so that
 * users who create new node types can extend the appropriate
 *  WalkingMap to do any state tranformations that their nodes
 * need without recompiling the original analysis.
 */

  
class WalkingMaps {
public:
  WalkingMaps(SuifEnv *suif_env, const LString &walking_maps_name,
  	      Address user_state);
  ~WalkingMaps();
  LString get_walking_maps_name() const;
  void set_user_state(Address user_state);
  Address get_user_state() const;

  // Call this to initialize a default
  // implementation for the SuifObject
  //
  void init_suif_object();

  // Use this to begin a walk.
  void process_a_suif_object(SuifObject *so);

  VisitorMap *get_pre_map() const;
  VisitorMap *get_children_map() const;
  VisitorMap *get_post_map() const;
  VisitorMap *get_process_map() const;

  void register_pre_visit_method(VisitMethod,
				 const LString &name);
  void register_children_visit_method(VisitMethod,
				 const LString &name);
  void register_post_visit_method(VisitMethod,
				 const LString &name);
  void register_process_visit_method(VisitMethod,
				 const LString &name);
  // Use this function to force an early exit.
  void set_done(bool val);
  bool is_done() const;

  // called from the pre() to skip processing and post
  // processing of this node
  void set_skip(bool val);
  bool is_skip() const;

private:
  LString _walking_maps_name;
  bool _done;
  bool _skip;

  VisitorMap *_pre_map;
  VisitorMap *_children_map;
  VisitorMap *_post_map;
  VisitorMap *_process_map;

  Address _user_state;
private:
  WalkingMaps(const WalkingMaps &);
  WalkingMaps& operator=(const WalkingMaps &);
};

template<class T>
T *get_walking_state(WalkingMaps *m) { return((T*)m->get_user_state()); }


#endif /* WALKING_MAPS_H */
