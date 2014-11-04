#ifndef CLONE_SUIFCLONERMODULE_H_
#define CLONE_SUIFCLONERMODULE_H_

#include "iokernel/clone_stream.h"
#include "basicnodes/basic.h"
#include "suifkernel/module.h"
#include "suifkernel/group_walker.h"

//	Several useful cloners
//	There are no actual commands that can be usefully called
//
//	Instead this module makes available some derived cloning classes
//	that meet useful cases
//
//	See also: StatementCloner in nci/suif/suif2b/utils/cloning_utils.[h|cpp]

class CloneInPlace : public CloneStream {
    public:
	CloneInPlace(SuifEnv *env);
	virtual void finish_orphan_object_cloning(
        	Object *old_object,
        	Object *new_object);
    private:
	SuifEnv *_env;
    };

//	The remote cloner will place all symbols that are cloned but are not 
//	contained in a cloned symbol table, in the provided symbol table.
//	Often, this will be just the symbol table one level below the cloned 
//	code

class CloneRemote : public CloneStream {
    public:
        CloneRemote(SuifEnv *env,SymbolTable *new_table);
        CloneRemote(SuifEnv *env);

        virtual void finish_orphan_object_cloning(
                Object *old_object,
                Object *new_object);

	void set_new_table(SymbolTable *new_table);
	SymbolTable *get_new_table();
    private:
	SymbolTable *_new_table;
	SuifEnv *_env;
    };

//	A dispatch cloner allows you to determine the action to be taken on
//	any object based on its type. In this it is something like a walker.
//
//	A helper class is used to tie in code to be called
class CloneAction {
    public:
	CloneAction(const LString &the_type);
	virtual void object_enquiry(Object *,CloneStreamObjectInstance *,PTR_TYPE ptr_type)=0;
	virtual ~CloneAction(){}
        const LString & get_type() const;
    private:
        const LString type;
	};


class DispatchCloner : public CloneStream{
    public:
       DispatchCloner(SuifEnv *env);
       virtual ~DispatchCloner();
       virtual void finish_orphan_object_cloning(
                Object *old_object,
                Object *new_object);

	void append_selector(CloneAction &x);

	// Objects which are not SuifObjects cannot be dispatched. Instead,
	// the virtual function non-suifobject_enquiry is called
	// The default implementation of this clones defined and owned 
	// pointers and references other pointers

	virtual void non_object_enquiry(Object *,CloneStreamObjectInstance *,PTR_TYPE ptr_type);

    private:
	virtual void object_enquiry(Object *,CloneStreamObjectInstance *,PTR_TYPE ptr_type);
	TypeBasedDispatch<CloneAction*> *_vector;
	};



extern "C" void EXPORT init_suifcloning(SuifEnv* suif);


#endif
