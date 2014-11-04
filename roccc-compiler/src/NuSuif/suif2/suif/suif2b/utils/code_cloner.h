#ifndef _CODE_CLONER_H_
#define _CODE_CLONER_H_

#include "iokernel/clone_stream.h"
#include "basicnodes/basic.h"
#include "common/suif_map.h"


/** @file
  * Defines CodeCloner, designed to clone executable objects.
  */

/** @class CodeCloner
  * This object is designed to clone executable objects like Statements and
  * Expressions.
  * Caller must supply a SymbolTable and optionally a map.
  * The SymbolTable represents the environment of the cloned object.
  * The map will contains a mapping from the old symbols to the new cloned
  *   symbols.
  *
  * \par
  * This cloner clones all owning objects.
  * For referencing and defining pointers, it depends on the type of the
  *  object it points to:
  * \par
  * For SymbolTableObject, if it can be found in the new environment
  *  (SymbolTable), then the pointer will be copied.  Otherwise, a new
  *  symbol will be cloned.
  * For SymbolTable, it will be replaced with a pointer to the new environment.
  *
  */
class CodeCloner : public CloneStream {
public:
  CodeCloner(SymbolTable *new_table,
	     suif_map<SymbolTableObject*, SymbolTableObject*> *old_new_map
	       = NULL);

  /**
    * Modified s.t. the cloned object has no parent.
    */
  virtual Object* clone( const Object* object );
  
  virtual void finish_orphan_object_cloning(Object *old_object,
					    Object *new_object);

 private:
  virtual void object_enquiry(Object *,
			      CloneStreamObjectInstance *,
			      PTR_TYPE ptr_type);

  SymbolTable                  *_new_table;
  suif_map<SymbolTableObject*, SymbolTableObject*>   *_old_new_map;
};



#endif // _CODE_CLONER_H_
