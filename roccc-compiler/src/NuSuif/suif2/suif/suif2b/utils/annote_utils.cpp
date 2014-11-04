
#include "annote_utils.h"

#include "basicnodes/basic.h"
#include "trash_utils.h"
#include "suifkernel/utilities.h"


struct annote_trash_pair {
  AnnotableObject *ao;
  Annote *an;
  annote_trash_pair(AnnotableObject *_ao, Annote *_an) : ao(_ao), an(_an) { };
};


void trash_named_annotes(SuifEnv *suif_env,
			 list<LString> &annote_names,
			 SuifObject *root)
{
  // have to be careful, since Annotes are themselves Annoteable objects; don't want to
  // change the tree while we iterate.
  list< annote_trash_pair > to_remove;

  for (Iter<AnnotableObject> obj_it = object_iterator<AnnotableObject>(root);
       obj_it.is_valid();
       obj_it.next()) {
    AnnotableObject *obj = &(obj_it.current());
    for (list<LString>::iterator str_it = annote_names.begin();
	 str_it != annote_names.end();
	 str_it++) {
      LString name = *str_it;
      int num_annotes =  obj->num_annote_of_name(name);
      for (int i = 0; i<num_annotes; i++) {
	Annote *annote = obj->lookup_annote_by_name(name, i);
	to_remove.push_back( annote_trash_pair(obj, annote) );
      }
    }
  }

  while (!to_remove.empty()) {
    AnnotableObject *obj = to_remove.front().ao;
    Annote *an = to_remove.front().an;
    to_remove.pop_front();

    obj->remove_annote(an);
    trash_it(suif_env, an);
  }

}

						   

void trash_named_annotes(SuifEnv *suif_env,
			 const LString annote_name,
			 SuifObject *root)
{
  list<LString> names;

  names.push_front(annote_name);
  trash_named_annotes(suif_env, names, root);
}



