#include "common/system_specific.h"
#include <ctype.h>
#include "cloner.h"
#include "suifkernel/module_subsystem.h"
#include "suifkernel/suif_object.h"
#include "suifkernel/suif_env.h"
#include "basicnodes/basic.h"
#include "iokernel/cast.h"
#include "suifnodes/suif.h"
#include "suifkernel/io_subsystem.h"

class SuifClonerModule : public Module {
  public:
    SuifClonerModule( SuifEnv* suif ) : Module( suif ,"suifcloning") {}
    virtual void initialize() {
      }
    virtual Module *clone() const {
      return((Module*)this);
      }
    };

extern "C" void EXPORT init_suifcloning(SuifEnv* suif)
{
  ModuleSubSystem* mSubSystem = suif->get_module_subsystem();
  if (!mSubSystem->retrieve_module("suifcloning")) {
    mSubSystem -> register_module( new SuifClonerModule(suif) );
    CloneInPlace *deep_cloner = new CloneInPlace(suif);
    deep_cloner->set_deep_clone();
    CloneStream *shallow_cloner = new CloneStream(suif->get_object_factory());
    shallow_cloner->set_shallow_clone();
    CloneSubSystem* css = suif->get_clone_subsystem();
    css->set_deep_clone_stream(deep_cloner);
    css->set_shallow_clone_stream(shallow_cloner);
    }
}

CloneAction::CloneAction(const LString &the_type)
    : type(the_type) 
{ }

const LString &CloneAction::get_type() const {
  return(type);
}

CloneInPlace::CloneInPlace(SuifEnv *env)
        :       CloneStream( env->get_object_factory()),
                _env(env) {}


void CloneInPlace::finish_orphan_object_cloning( 
	Object *old_object, 
	Object *new_object) {
    SuifObject *oo = to<SuifObject>(old_object);
    SymbolTableObject *no = to<SymbolTableObject>(new_object);

    SuifObject *parent = oo->get_parent();
    no->set_parent(0); // because it will assert if we don't
    kernel_assert_message(is_kind_of<SymbolTableObject>(oo),
                                ("Not in a symbol table"));
    SymbolTable *st = to<SymbolTable>(parent);
    st->append_symbol_table_object(no);
    }


CloneRemote::CloneRemote(SuifEnv *env,SymbolTable *new_table) 
	: 	CloneStream( env->get_object_factory()),
		_new_table(new_table),
		_env(env) {}

CloneRemote::CloneRemote(SuifEnv *env) 
        :       CloneStream( env->get_object_factory()),
                _new_table(0), 
                _env(env) {}

void CloneRemote::set_new_table(SymbolTable *new_table) {
    _new_table = new_table;
    }

SymbolTable *CloneRemote::get_new_table() {
    return _new_table;
    }

void CloneRemote::finish_orphan_object_cloning( 
			Object *old_object, 
			Object *new_object) {
    //SuifObject *oo = to<SuifObject>(old_object);
    SymbolTableObject *no = to<SymbolTableObject>(new_object);
    no->set_parent(0);
    get_new_table()->append_symbol_table_object(no);
    }

DispatchCloner::DispatchCloner(SuifEnv *env) : 
  CloneStream( env->get_object_factory()),
  _vector(new TypeBasedDispatch<CloneAction*>)

{}

DispatchCloner::~DispatchCloner()
  {
    delete _vector;
  }

void DispatchCloner::finish_orphan_object_cloning(
                Object *old_object,
                Object *new_object) {}

void DispatchCloner::append_selector(CloneAction &x) {
   const LString &type = x.get_type();
   _vector->set_value(type,&x);
    }

void DispatchCloner::object_enquiry(Object *x,CloneStreamObjectInstance *o,PTR_TYPE ptr_type) {
    if (!is_kind_of<SuifObject>(x)) {
	non_object_enquiry(x,o,ptr_type);
	return;
	}

    SuifObject *so = to<SuifObject>(x);
    CloneAction *w = _vector->lookup_value(so);
    if (!w) 
	set_reference(o);
    else
	w->object_enquiry(x,o,ptr_type);
    }

void DispatchCloner::non_object_enquiry(Object *x,CloneStreamObjectInstance *o,PTR_TYPE ptr_type) {
    switch (ptr_type) {
	case NO_PTR:
	case REF_PTR:
	    set_reference(o);
	    break;
	case DEFINE_PTR:
	case OWN_PTR:
	    set_clone_object(o);
	}
    }



