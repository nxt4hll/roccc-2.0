#include "common/system_specific.h"
#include "module_subsystem.h"

#include "iokernel/helper.h"

#include "module.h"

#include "common/suif_map.h"
#include "common/suif_list.h"
#include <iostream>

#include <assert.h>
#include "suif_exception.h"

#include "command_line_parsing.h"

using namespace std;

class ModuleInterface {
 protected:
  LString interfaceName;
  // These should be maps.
  list<Module *> *listenerModules;
  list<LString> *producerModuleNames;
  list<Module *> *producerModules;
  String _description;
 public:
  ModuleInterface(const LString &interface_name);
  ~ModuleInterface(void);

  bool exists_listener(Module *) const;
  void add_listener(Module *);

  bool exists_producer_name(const LString &producer_module_name) const;
  bool exists_producer(Module *) const;

  void add_producer_name(const LString &producer_module_name);
  void add_producer(Module *);

  void set_description(const String &description);
  String get_description() const;


  void remove_producer(Module *);

  void notify_listener_of_all_producer_names(Module *listening_module);
  void notify_listener_of_all_producers(Module *listening_module);
  void notify_all_listeners_of_producer_name(const LString &producer_name);
  void notify_all_listeners_of_producer(Module *producer_module);
  void notify_all_listeners_of_producer_destruction(Module *producer_module);
};


ModuleSubSystem::ModuleSubSystem( SuifEnv* suif_env ) :
  _suif_env(suif_env),
  listOfRegisteredModules(new suif_map<LString,Module*>),
  moduleInterfaces(new suif_map<LString,ModuleInterface*>)
{
  // Add information here about some of the interfaces already defined
  //
  set_interface_description
    ("print",
     "get_interface(\"print\") will return a function of signature \n"
     "  'PrintDispatch': void(*)(Module*, ostream &, const ObjectWrapper &)\n"
     "  that will print an Object that participates in the MetaClass system."
     );
  set_interface_description
    ("get_object_location",
     "get_interface(\"get_object_location\") will return a function of signature \n"
     "  'LocationDispatch': ObjectLocation ( *)( SuifObject * )\n"
     "  that will return a structure with the source file, line and column information\n"
     "  if available"
     );
  set_interface_description
    ("object_nodes",
     " This module defines a set of objects"
     );
}


ModuleSubSystem::~ModuleSubSystem() {
  delete_map_and_value( listOfRegisteredModules );
  delete_map_and_value( moduleInterfaces );
}


Module* ModuleSubSystem::initialize_module( const LString& moduleName ) {
  Module* module = retrieve_module( moduleName );
  assert( module );
  module->initialize();
  return module;
}


bool ModuleSubSystem::is_initialized( const LString& moduleName ) const {
  Module* module = retrieve_module( moduleName );
  assert( module );
  return module->is_initialized();
}


bool ModuleSubSystem::is_available( const LString& moduleName ) const {
  Module* module = retrieve_module( moduleName );
  return ( module != 0 );
}

void ModuleSubSystem::register_module( Module* module ) {
  assert( module );
  const LString& moduleName = module->get_module_name();
  if ( retrieve_module( moduleName )  ) {
     delete module;
  } else {
    // Why don't we use initialize_module here?
    module->initialize();
    //    (*listOfRegisteredModules)[ moduleName ] = module;
    listOfRegisteredModules->enter_value(moduleName, module);
  }
}


//@purpose To register a module mod.  If not successful, and if delete_if_fail
//         is true, then mod will be deleted.
//@return  True iff successful.
//
bool ModuleSubSystem::test_and_register_module(Module* mod, bool delete_if_fail)
{
  if (!retrieve_module(mod->get_module_name())) {
    register_module(mod);
    return true;
  }
  if (delete_if_fail) {
    delete mod;
  }
  return false;
}



Module* ModuleSubSystem::retrieve_module( const LString& moduleName ) const {
  Module* module = 0;
  suif_map<LString,Module*>::iterator it =
      listOfRegisteredModules->find( moduleName );
  if ( it != listOfRegisteredModules->end() ) {
    module = (*it).second;
  }
  return module;
}

bool ModuleSubSystem::execute( const LString& module_name,
                               TokenStream* command_line ) {
  Module* module = parse_command_line_and_clone( module_name, command_line );
  if ( module ) {
    try {
      module->execute();
    } catch (...) {
      if ( module -> delete_me() ) delete module;
      throw;
    }
    if ( module -> delete_me() ) delete module;
    return true;
  }
  return false;
}

void ModuleSubSystem::print_modules(ostream &output,
				    const String &separator,
				    const LString &interface_filter) const {
  bool comma_needed = false;
  for (suif_map<LString,Module*>::iterator iter =
	 listOfRegisteredModules->begin();
       listOfRegisteredModules->end() != iter;
       iter++ ) {
    Module *module = (*iter).second;
    if (comma_needed) {
      output << separator.c_str();
    }
    if (interface_filter != emptyLString
	&& !module->supports_interface(interface_filter))
      continue;

    comma_needed = true;
    output <<  "'";
    output << module->get_module_name().c_str();
    output << "'";
  }
}

void ModuleSubSystem::get_module_list(const LString &interface_filter,
				      list<LString> &mlist) const {
  for (suif_map<LString,Module*>::iterator iter =
	 listOfRegisteredModules->begin();
       listOfRegisteredModules->end() != iter;
       iter++ ) {
    Module *module = (*iter).second;
    if (interface_filter != emptyLString
	&& !module->supports_interface(interface_filter))
      continue;
    mlist.push_back(module->get_module_name());
  }
}


Module* ModuleSubSystem::parse_command_line_and_clone(
                         const LString& moduleName,
                         TokenStream* command_line ) const {
  Module* module = retrieve_module( moduleName );
  String avail_mods = "";
  if (module == 0) {
    cerr << "Available modules are: ";
    print_modules(cerr, ", ", emptyLString);
    cerr << "\nUse one of these or import another library\n" << endl;
    SUIF_THROW(SuifException(String("Could not load module [") + moduleName
			     + "]"));
  }

  module = module -> clone();
  if (!module->is_initialized() ) {
    module->initialize();
  }
  assert( module );
  if ( !module->parse_command_line( command_line ) ) {
    // module has failed to initialize
    if ( module -> delete_me() ) delete module;
    module = 0;
  }
  // check for the --help flag here
  if (module->_help_flag->is_set()) {
    String usage = module->get_usage();
    cerr << usage;
    SUIF_THROW(SuifException("User requested usage"));
  }
  return module;
}

ModuleInterface *ModuleSubSystem::get_interface(const LString &interface_name) const {
  suif_map<LString,ModuleInterface*>::iterator iter =
    moduleInterfaces->find(interface_name);;
  if (iter == moduleInterfaces->end()) return 0;
  return((*iter).second);
}

ModuleInterface *ModuleSubSystem::retrieve_interface(const LString &interface_name) {
  ModuleInterface *mi = get_interface(interface_name);
  if (!mi) {
    mi = new ModuleInterface(interface_name);
    moduleInterfaces->enter_value(interface_name, mi);
  }
  return(mi);
}

void ModuleSubSystem::register_interface_listener( Module *listening_module,
						   const LString &interface_name) {
  ModuleInterface *mi = retrieve_interface(interface_name);

  if (mi->exists_listener(listening_module)) return;
  mi->add_listener(listening_module);

  mi->notify_listener_of_all_producer_names(listening_module);
  mi->notify_listener_of_all_producers(listening_module);

}


void ModuleSubSystem::
register_interface_producer_name( const LString &module_name,
				  const LString &interface_name ) {
  ModuleInterface *mi = retrieve_interface(interface_name);

  if (mi->exists_producer_name(module_name)) return;
  mi->add_producer_name(module_name);

  mi->notify_all_listeners_of_producer_name(module_name);
}

void ModuleSubSystem::
register_interface_producer( Module *producer_module,
			     const LString &interface_name ) {
  ModuleInterface *mi = retrieve_interface(interface_name);

  LString producer_module_name = producer_module->get_module_name();
  register_interface_producer_name(producer_module_name,
				   interface_name);

  if (mi->exists_producer(producer_module)) return;
  mi->add_producer(producer_module);

  mi->notify_all_listeners_of_producer(producer_module);
}

void ModuleSubSystem::
register_interface_producer_destruction( Module *producer_module,
					 const LString &interface_name ) {
  ModuleInterface *mi = retrieve_interface(interface_name);

  if (!mi->exists_producer(producer_module)) return;
  mi->remove_producer(producer_module);
  mi->notify_all_listeners_of_producer_destruction(producer_module);
}

void ModuleSubSystem::
set_interface_description(const LString &interface_name,
			  const String &description) {
  ModuleInterface *mi = retrieve_interface(interface_name);
  mi->set_description(description);
}
String ModuleSubSystem::
get_interface_description(const LString &interface_name) const {
  ModuleInterface *mi = get_interface(interface_name);
  if (mi == NULL) return(emptyString);
  return(mi->get_description());
}

void ModuleSubSystem::get_interface_list(list<LString> &ilist) const {
  for (suif_map<LString,Module*>::iterator iter =
	 moduleInterfaces->begin();
       moduleInterfaces->end() != iter;
       iter++ ) {
    LString name = (*iter).first;
    ilist.push_back(name);
  }
}

/*
 * ****************************************
 *
 * ModuleInterface Implementation
 *
 * Each interface has a set of
 * Listeners and Producers.
 * The producers are always registered first
 * by name, then my module.
 * They are removed on destruction
 *
 * ****************************************
 */

ModuleInterface::ModuleInterface(const LString &interface_name) :
  interfaceName(interface_name),
  listenerModules(new list<Module*>()),
  producerModuleNames(new list<LString>()),
  producerModules(new list<Module *>()),
  _description(emptyString)
{}

ModuleInterface::~ModuleInterface(void)
{
  delete listenerModules;
  delete producerModuleNames;
  delete producerModules;
}

void ModuleInterface::set_description(const String &description) {
  _description = description;
}
String ModuleInterface::get_description() const {
  return(_description);
}



bool ModuleInterface::exists_listener(Module *listener_module) const {
  for (list<Module *>::iterator iter = listenerModules->begin();
       iter != listenerModules->end();
       iter++) {
    if ((*iter) == listener_module) return true;
  }
  return(false);
}

void ModuleInterface::add_listener(Module *listener_module) {
  suif_assert(!exists_listener(listener_module));
  listenerModules->push_back(listener_module);
}

bool ModuleInterface::exists_producer_name(const LString &producer_module_name) const {
  for (list<LString>::iterator iter = producerModuleNames->begin();
       iter != producerModuleNames->end();
       iter++) {
    if ((*iter) == producer_module_name) return true;
  }
  return(false);
}

void ModuleInterface::add_producer_name(const LString &producer_module_name) {
  suif_assert(!exists_producer_name(producer_module_name));
  producerModuleNames->push_back(producer_module_name);
}


bool ModuleInterface::exists_producer(Module *producer_module) const {
  for (list<Module *>::iterator iter = producerModules->begin();
       iter != producerModules->end();
       iter++) {
    if ((*iter) == producer_module) return true;
  }
  return(false);
}

void ModuleInterface::add_producer(Module *producer_module) {
  suif_assert(!exists_producer(producer_module));
  producerModules->push_back(producer_module);
}

void ModuleInterface::remove_producer(Module *producer_module) {
  if (!exists_producer(producer_module)) return;
  for (list<Module *>::iterator iter = producerModules->begin();
       iter != producerModules->end();
       iter++) {
    if ((*iter) == producer_module) {
      producerModules->erase(iter);
      return;
    }
  }
}


void ModuleInterface::notify_listener_of_all_producer_names(Module *listening_module) {
  for (list<LString>::iterator iter =
	 producerModuleNames->begin();
       iter != producerModuleNames->end();
       iter++) {
    // Here are the call-backs to the listeners
    listening_module->interface_registered(*iter, interfaceName);
  }
}

void ModuleInterface::notify_listener_of_all_producers(Module *listening_module) {
  for (list<Module *>::iterator iter =
	 producerModules->begin();
       iter != producerModules->end();
       iter++) {
    // Here are the call-backs to the listeners
    listening_module->interface_object_created(*iter, interfaceName);
  }
}


void ModuleInterface::notify_all_listeners_of_producer_name(const LString &name) {
  for (list<Module *>::iterator iter =
	 listenerModules->begin();
       iter != listenerModules->end();
       iter++) {
    // Here are the call-backs to the listeners
    (*iter)->interface_registered(name, interfaceName);
  }
}

void ModuleInterface::notify_all_listeners_of_producer(Module *producer) {
  for (list<Module *>::iterator iter =
	 listenerModules->begin();
       iter != listenerModules->end();
       iter++) {
    // Here are the call-backs to the listeners
    (*iter)->interface_object_created(producer, interfaceName);
  }
}

void ModuleInterface::notify_all_listeners_of_producer_destruction(Module *producer) {
  for (list<Module *>::iterator iter =
	 listenerModules->begin();
       iter != listenerModules->end();
       iter++) {
    // Here are the call-backs to the listeners
    (*iter)->interface_object_destructed(producer, interfaceName);
  }
}
