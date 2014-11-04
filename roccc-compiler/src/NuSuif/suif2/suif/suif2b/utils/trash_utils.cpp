#include "common/system_specific.h"
#include "common/suif_hash_map.h"
#include "trash_utils.h"
#include "suifkernel/utilities.h"
#include "basicnodes/basic.h"

#include "basicnodes/basic_factory.h"

void trash_it(SuifObject *obj) {
  if (obj == NULL) return;
  trash_it(obj->get_suif_env(), obj);
}

void trash_it(SuifEnv *s, SuifObject *obj) {
  // It MUST be unowned to trash it
  assert(obj->get_parent() == NULL);
  static LString k_trash = "trash";
  FileSetBlock *fsb = s->get_file_set_block();
  suif_assert(fsb != NULL);

  Annote *an = fsb->peek_annote(k_trash);
  BrickAnnote *b_an = NULL;
  if (is_kind_of<BrickAnnote>(an)) {
    b_an = to<BrickAnnote>(an);
  }
  if (b_an == NULL) {
    suif_assert_message(an == NULL, 
	("Non-brick annotation 'trash' on the file set block"));
    b_an = create_brick_annote(s, k_trash);
    fsb->append_annote(b_an);
  } 
  // In order to allow more passes to work when this happens,
  // we mark that these are only referenced. here.  Thus they are NOT
  // owned.
  b_an->append_brick(create_suif_object_brick(s, obj));
}



void take_out_trash(SuifEnv *s) {
  static LString k_trash = "trash";
  FileSetBlock *fsb = s->get_file_set_block();
  if (fsb == NULL) return;

  // remove the trash annote.  we'll replace it later
  Annote *an = fsb->take_annote(k_trash);
  if (!is_kind_of<BrickAnnote>(an)) {
    return;
  }
  BrickAnnote *b_an = to<BrickAnnote>(an);
  // create a map of the objects in the trash
  // that links to their "parents"
  suif_hash_map<SuifObject*,SuifObject*> obj_map;
  suif_hash_map<SuifObject*,int> referenced_map;

  // keep our own list of the objects because of the
  // horrible performance of the Object list interface.
  list<SuifObject*> obj_list;
  
  for (Iter<SuifBrick*> iter = b_an->get_brick_iterator();
       iter.is_valid(); iter.next()) {
    SuifBrick *br = iter.current();
    if (!is_kind_of<SuifObjectBrick>(br)) {
      suif_warning(br, "Non SuifObjectBrick in the trash");
      continue;
    }
    SuifObjectBrick *o_br = to<SuifObjectBrick>(br);
    SuifObject *base_obj = o_br->get_object();
    if (base_obj) {
      obj_list.push_back(base_obj);

      //obj_map[base_obj] = base_obj;
      obj_map.enter_value(base_obj, base_obj);
      for (Iter<SuifObject> sub_iter = object_iterator<SuifObject>(base_obj);
	   sub_iter.is_valid(); sub_iter.next()) {
	SuifObject *sub_obj = &sub_iter.current();
	// obj_map[sub_obj] = base_obj;
	obj_map.enter_value(sub_obj, base_obj);
      }
    }
  }

  // We've got all the information in the maps.
  delete b_an;
  b_an = create_brick_annote(s, k_trash);
  
  // Now we have a map of Object->parent object
  // We need another one for object -> referenced
  for (Iter<SuifObject> all_iter1 = object_iterator<SuifObject>(fsb);
       all_iter1.is_valid(); all_iter1.next()) {
    SuifObject *obj = &all_iter1.current();
    
    for (Iter<SuifObject> ref_iter = 
	   suif_object_ref_iterator<SuifObject>(obj, SuifObject::get_class_name());
	 ref_iter.is_valid(); ref_iter.next()) {
      SuifObject *obj_ref = &ref_iter.current();
      
      suif_hash_map<SuifObject*,SuifObject*>::iterator find =
	obj_map.find(obj_ref);
      if (find == obj_map.end()) continue;
      // mark the parent as referenced
      SuifObject *par = (*find).second;
      //referenced_map[par] = 1;
      referenced_map.enter_value(par, 1);
    }
  }
  // walk over all of the reference
  // and delete or put back
  for (list<SuifObject *>::iterator obj_iter = 
	 obj_list.begin();
       obj_iter != obj_list.end(); obj_iter++) {
    SuifObject *parent = *obj_iter;
    if (referenced_map.find(parent) == referenced_map.end()) {
      // delete it
      delete parent;
    } else {
      // or put it back
      b_an->append_brick(create_suif_object_brick(s, parent));
    }
  }
  fsb->append_annote(b_an);
  // All done;
}
    
static bool is_owned_by_file_set_block(SuifObject *obj,
				       FileSetBlock *fsb) {
  if (obj == NULL) return(false);
  if (obj == fsb) return(true);
  SuifObject *parent = obj->get_parent();
  if (parent == NULL) return(false);
  return(is_owned_by_file_set_block(parent, fsb));
}

//
size_t validate_file_set_block_ownership(FileSetBlock *fsb) {
  static LString k_trash = "trash";
  int errors = 0;
  if (fsb == NULL) return (false);

  // remove the trash annote because it's special.
  // The items in the trash will NOT be appear to be owned
  // by this file set block.
  Annote *an = fsb->take_annote(k_trash);

  suif_hash_map<SuifObject*,bool> dangling_objects;

  // Now we have a map of Object->parent object
  // We need another one for object -> referenced

  // Once to check all "owned" objects.
  for (Iter<SuifObject> all_iter1 = object_iterator<SuifObject>(fsb);
       all_iter1.is_valid(); all_iter1.next()) 
    {
      SuifObject *obj = &all_iter1.current();
      if (!is_owned_by_file_set_block(obj, fsb)) 
	{
	  if (dangling_objects.find(obj) == dangling_objects.end()) {
	    suif_warning(obj, "Object %s not owned by file set",
			 obj->getClassName().c_str());
	    // dangling_objects[obj] = true;
	    dangling_objects.enter_value(obj, true);
	    errors++;
	  }
	}
    }

  // Once again to check for references
  for (Iter<SuifObject> all_iter = object_iterator<SuifObject>(fsb);
       all_iter.is_valid(); all_iter.next()) 
    {
      SuifObject *obj = &all_iter.current();
      if (dangling_objects.find(obj) != dangling_objects.end()) continue;
      for (Iter<SuifObject> ref_iter = 
	     suif_object_ref_iterator<SuifObject>(obj, 
						  SuifObject::get_class_name());
	   ref_iter.is_valid(); ref_iter.next()) {
	SuifObject *obj_ref = &ref_iter.current();
	if (dangling_objects.find(obj) != dangling_objects.end()) continue;

	if (!is_owned_by_file_set_block(obj_ref, fsb)) {
	  suif_warning(obj_ref, "Referenced Object %s not owned by file set",
		       obj_ref->getClassName().c_str());
	  // dangling_objects[obj] = true;
	  dangling_objects.enter_value(obj, true);
	  errors++;
	}
      }
    }
  if (an != NULL)
    fsb->append_annote(an);
  
  return(errors);
}

