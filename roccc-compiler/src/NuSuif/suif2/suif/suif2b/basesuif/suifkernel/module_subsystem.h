#ifndef SUIFKERNEL__MODULE_SUBSYSTEM_H
#define SUIFKERNEL__MODULE_SUBSYSTEM_H

#include "suifkernel_forwarders.h"

class ModuleInterface;

class ModuleSubSystem {
public:
  ModuleSubSystem( SuifEnv* _suif_env );
  virtual ~ModuleSubSystem();

  virtual Module* initialize_module( const LString& moduleName );

  virtual bool is_initialized( const LString& moduleName ) const;
  virtual bool is_available( const LString& moduleName ) const;
  virtual void register_module( Module* module );
  virtual bool test_and_register_module( Module* module, bool delete_if_fail );
  virtual Module* retrieve_module( const LString& moduleName ) const;

  virtual bool execute( const LString& moduleName,
                        TokenStream* command_line );

  /**
   * write the list of modules to the mlist
   */
  virtual void get_module_list(const LString &interface_filter,
			       list<LString> &mlist) const;

  // print out a list of the modules that implement the interface
  // If the interface_filter is the emptyLString then print them all out
  virtual void print_modules(std::ostream &output,
			     const String &separator,
			     const LString &interface_filter) const;

  // Make a clone of the prototype module found by name
  //  Then apply the command line to the clone and return it
  virtual Module* parse_command_line_and_clone(
                        const LString& module_name,
                        TokenStream* command_line ) const;

  // Iterface for the SUIF interface support mechanism
  // Each module interested in knowing about the other modules that
  // support a particular interface should use these
  virtual void register_interface_listener( Module *listening_module,
					    const LString &interface_name);
  // any module that supports an interface should register its interface
  // here:
  // These will issue callbacks to listeners.  In the call back
  // code, you should NOT try to add a new listener.
  virtual void register_interface_producer_name(const LString &module_name,
					 const LString &interface_name);
  
  virtual void register_interface_producer(Module *module,
						  const LString &interface_name);
  virtual void register_interface_producer_destruction(Module *module,
						       const LString &interface_name);

  virtual ModuleInterface *get_interface(const LString &interface_name) const;
  // Will build it if it does not exist.
  virtual ModuleInterface *retrieve_interface(const LString &interface_name);

  virtual void set_interface_description(const LString &interface_name,
					 const String &description);
  virtual String get_interface_description(const LString &interface_name) const;
  /**
   * write the list of interfaces to the ilist
   */
  virtual void get_interface_list(list<LString> &ilist) const;

private:
  SuifEnv* _suif_env;
  suif_map<LString,Module*> *listOfRegisteredModules;
  suif_map<LString,ModuleInterface*> *moduleInterfaces;

  ModuleSubSystem(const ModuleSubSystem &);
  ModuleSubSystem& operator=(const ModuleSubSystem &);
};



#endif

