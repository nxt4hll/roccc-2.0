#ifndef _UTILS_ANNOTE_UTILS_H_
#define _UTILS_ANNOTE_UTILS_H_

#include "common/lstring.h"
#include "basicnodes/basic.h"
#include "basicnodes/basic_factory.h"
#include "small_set.h"
#include "print_utils.h"



/*
 * Create an annote node that references (not owns) a set of SuifObjects.
 * T must be a subtype of SuifObject.
 */
template<class T>
class ObjectSetAnnote {
 private:
  LString _annote_name;
 public:
  ObjectSetAnnote(LString name) :
    _annote_name(name) {
  };

  BrickAnnote* get_annote(const AnnotableObject* host) const {
    Annote* ann = host->peek_annote(_annote_name);
    if (ann == 0) return 0;
    suif_assert_message(is_kind_of<BrickAnnote>(ann),
			("Annotation [%s] with name [%s] of [%s] is not a BrickAnnote.",
			 to_id_string(ann).c_str(),
			 _annote_name.c_str(),
			 to_id_string(host).c_str()));
    return to<BrickAnnote>(ann);
  }
  
  /* It will remove the annote from \a host.  The removed annote will be deleted.
   */
  void remove_annote(AnnotableObject* host) const {
    if (get_annote(host) == 0) return;
    Annote* ann = host->take_annote(_annote_name);
    delete ann;
  }


  void write(AnnotableObject* host,
	     const small_set<T>* src) const {
    SuifEnv* senv = host->get_suif_env();
    BrickAnnote* ann = create_brick_annote(senv, _annote_name);
    for (small_set<T>::const_iterator iter = src->begin();
	 iter != src->end();
	 iter++) {
      SuifObjectBrick* brick = create_suif_object_brick(senv, *iter);
      ann->append_brick(brick);
    }
    host->append_annote(ann);
  }
  
  unsigned read(const AnnotableObject* host,
		small_set<T>* dest) const {
    BrickAnnote* bann = get_annote(host);
    if (bann == 0) return 0;
    unsigned cnt = 0;
    for (Iter<SuifBrick*> iter = bann->get_brick_iterator();
	 iter.is_valid();
	 iter.next()) {
      SuifBrick* brick = iter.current();
      suif_assert_message(is_kind_of<SuifObjectBrick>(brick),
			  ("Expecting a SuifObjectBrick of [%s] from annote [%s].",
			   to_id_string(brick).c_str(),
			   to_id_string(bann).c_str()));
      SuifObject* brickobj = to<SuifObjectBrick>(brick)->get_object();
      dest->add(dynamic_cast<T>(brickobj));
      cnt++;
    }
    return cnt;
  }
  
  bool is_member(T obj, const AnnotableObject* host) const {
    Annote* ann = host->peek_annote(_annote_name);
    if (ann == 0) return false;
    suif_assert_message(is_kind_of<BrickAnnote>(ann),
			("Annotation [%s] with name [%s] of [%s] is not a BrickAnnote.",
			 to_id_string(ann).c_str(),
			 _annote_name.c_str(),
			 to_id_string(host).c_str()));
    BrickAnnote* bann = to<BrickAnnote>(ann);
    for (Iter<SuifBrick*> iter = bann->get_brick_iterator();
	 iter.is_valid();
	 iter.next()) {
      SuifBrick* brick = iter.current();
      suif_assert_message(is_kind_of<SuifObjectBrick>(brick),
			  ("Expecting a SuifObjectBrick of [%s] from annote [%s].",
			   to_id_string(brick).c_str(),
			   to_id_string(bann).c_str()));
      SuifObject* brickobj = to<SuifObjectBrick>(brick)->get_object();
      if ((SuifObject*)obj == brickobj) return true;
    }
    return false;
  }
  
};

#endif /* _UTILS_ANNOTE_UTILS_H_ */
