#ifndef SUIFKERNEL__MODULE_H
#define SUIFKERNEL__MODULE_H

#include "suifkernel_forwarders.h"

/**
 * \file module.h
 * Defines the interface of a Module, the basis of IR nodes or passes.
 */

/**
 * \class Module module.h suifkernel/module.h
 * Defines the interface of a module, the basis of IR nodes or passes.
 * 
 * \par From Loading A DLL to executing a module:
 *   When a new DLL is loaded,
 *   <UL>
 *   <LI>
 *   1) init_DLL(SuifEnv)  is called.  This
 *      should build a class for each module
 *      in the DLL. Each of these modules
 *      is placed into the ModuleSubSystem by name.
 *      During ModuleSubSystem::register_module()
 *       the Module::initialize() method will
 *       be called.  Command line parsing
 *       should be built here.
 *   <LI>
 *   2) When a pass is to be execute (because the
 *      execute Module wants to invoke it)
 *      The prototype module is retrieved by name
 *         (the string returned by Module::get_class_name()).
 *         from the module sub system.
 *      The prototype is then cloned (with Module::clone()).
 *      The clone is then initialized (with Module::initialize())
 *        if its is_initialize() returns false.
 *      Then the clone's parse_command_line() method is called.
 *      Finally, the Module::execute() method is invoked.
 *   <LI>
 *   3) The ModuleSubSystem also has a
 *      execute() which
 *      <UL>
 *      <LI>
 *      a) invokes parse_command_line_and_clone() which
 *         builds a returns a clone() of the Module
 *         invokes Module::parse_command_line on
 *           TokenStream command_line
 *      <LI>
 *      b) invokes module::execute() then
 *           deletes the module if delete_me()
 *   </UL>
 *
 */

class Module {
public:
  Module (SuifEnv* suif_env, const LString& moduleName = emptyLString );
  virtual ~Module();

  /**
     Retrieves the name of a module. This name must be unique for every kind of
     Module. (it may be called before calling the initialize method)
  */
  virtual LString get_module_name() const;

  /**
    generates a new copy of the current instance if necessary
    The implementor of a subclass is responsible for overriding
    this method if the implementation has a unique state
    the default implementation returns the current instance
  */
  virtual Module* clone() const = 0;

  /**
     tells the system whether an instance of this Module should be
     deleted after it has been executed</LI>
     the default implementation returns false
  */
  virtual bool delete_me() const;

  /**
     returns true if the method initalize() was already executed
     on this instance
  */
  virtual bool is_initialized() const;

  /**
     initializes the modules. This method must/will be called before
     a call to parse_command_line or execute
  */
  virtual void initialize();

  /**
     parses the input for this Module
  */
  virtual bool parse_command_line( TokenStream* command_line_stream );

  /**
     executes the Module
     parse_command_line should be called before this method is invoked
  */
  virtual void execute();

  virtual SuifEnv *get_suif_env() const;

  /**
     Interface required for the SUIF interface support mechanism.
     
     This can return pointers to anything. Frequently it will be
     a pointer to a function to call
  */
  virtual bool supports_interface( const LString &interface_name ) const;
  virtual Address get_interface( const LString &interface_name ) const;

  /**
   * write the list of interfaces this module supports to the llist
   */
  virtual void get_supported_interface_list(list<LString> &llist) const;

  /**
     These are callbacks that the module system will give when
     a new module registers its support for an interface.
  */
  virtual void interface_registered(const LString &producer_module_name,
				    const LString &interface_name);
  virtual void interface_object_created(Module *producer_module,
					const LString &interface_name);
  virtual void interface_object_destructed(Module *producer_module,
					   const LString &interface_name);

  void import_module(const char* module_name);

  /**
   * get the module description
   */
  virtual String get_description() const;

  virtual String get_usage() const;

protected:
  SuifEnv* _suif_env;
  LString _module_name;

  OptionLiteral* _command_name; // argv[0]
  OptionList* _command_line;

  // For normal options, just create the appropriate Option 
  // object and add it to the _flags.
  // The _flags list is the first to be checked.
  // Then for any required options, add them after the
  friend class ModuleSubSystem;

  OptionSelection* _flags;
  OptionLiteral* _help_flag;
  
  suif_hash_map<LString, Address> *_interfaces;
  virtual void set_interface( const LString &interface_name,
			      Address interface);
private:
  Module(const Module &);
  Module& operator=(const Module &);
};

#endif




