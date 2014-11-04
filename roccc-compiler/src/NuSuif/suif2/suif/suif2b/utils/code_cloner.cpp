#include "code_cloner.h"
#include "suifkernel/suif_exception.h"
#include "suifkernel/suif_env.h"
#include "utils/print_utils.h"
//#include <iostream.h>
#include <iostream>


CodeCloner::CodeCloner(SymbolTable *new_table,
		       suif_map<SymbolTableObject*, SymbolTableObject*>* map) :
  CloneStream(new_table->get_suif_env()->get_object_factory()),
  _new_table(new_table),
  _old_new_map(map)
{
  set_pointer_handling(ENQUIRE_PTR, ENQUIRE_PTR, CLONE_PTR);// (ref, def, own)
};


Object* CodeCloner::clone( const Object* object )
{
  Object* clone = CloneStream::clone(object);
  if (is_kind_of<SuifObject>(clone))
    to<SuifObject>(clone)->set_parent(0);
  return clone;
}

void CodeCloner::finish_orphan_object_cloning(Object *old_object,
					      Object *new_object)
{
  if (is_kind_of<SymbolTableObject>(new_object)) {
    SymbolTableObject *new_sym = to<SymbolTableObject>(new_object);
    new_sym->set_parent(NULL);
    _new_table->add_symbol(new_sym);
    if (_old_new_map != 0)
      _old_new_map->enter_value(to<SymbolTableObject>(old_object), new_sym);
  }
}



/** CodeCloner is set up so that this is called only for reference pointers
  * only.
  */
void CodeCloner::object_enquiry(Object *obj,
				CloneStreamObjectInstance *co,
				PTR_TYPE ptr_type)
{
  if (is_kind_of<SymbolTableObject>(obj)) {
    if (_new_table->has_symbol_table_object_member(to<SymbolTableObject>(obj)))
      set_reference(co);
    else
      set_clone_object(co);
  } else if (is_kind_of<SymbolTable>(obj)) { // ???
    set_replacement(co, _new_table);
  } else {
    set_reference(co);
  }
}
                                                                                
