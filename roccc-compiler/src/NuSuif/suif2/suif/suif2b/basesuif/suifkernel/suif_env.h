/**
 * \file suif_env.h
 * A SuifEnv represents the global environment of a suif session.
 */



#ifndef SUIFKERNEL__SUIF_ENV_H
#define SUIFKERNEL__SUIF_ENV_H

#include "suifkernel_forwarders.h"

#include <stdarg.h>


/** A global function that creates the suif_env; 
 *  called once only at the beginning of a suif driver 
 */

SuifEnv* create_suif_env();

/**
 * \class SuifEnv suif_env.h suifkernel\suif_env.h
 * A SuifEnv represents the global environment of a suif session.
 * It contains the following submodules 
 * (a get_ and a set_ method is defined for each item)
 * <UL>
 * <LI> an Object Factory </LI>
 * <LI> a FileSetBlock, which contains the program
 *      on which suifpasses are applied. </LI>
 * <LI> a module subsystem</LI>
 * <LI> a dll subsystem</LI>
 * <LI> an error subsystem</LI>
 * <LI> a print subsystem</LI>
 * <LI> a clone subsystem</LI>
 * <LI> a TypeBuilder</LI>
 * </UL>
 */
class SuifEnv {
public:
  SuifEnv();

  virtual void init();

  virtual ~SuifEnv();

  virtual void set_object_factory( ObjectFactory* );
  virtual ObjectFactory* get_object_factory() const;

  virtual ModuleSubSystem* get_module_subsystem() const;
  virtual void set_module_subsystem( ModuleSubSystem* subSystem );

  virtual void set_dll_subsystem( DLLSubSystem* _dll_subsystem );
  virtual DLLSubSystem* get_dll_subsystem() const;

  virtual void set_error_subsystem( ErrorSubSystem* _error_subsystem );
  virtual ErrorSubSystem* get_error_subsystem() const;

  virtual PrintSubSystem* get_print_subsystem() const;
  virtual void set_print_subsystem( PrintSubSystem* subsystem );

  virtual void add_object_factory( RealObjectFactory* of );
  virtual RealObjectFactory* get_object_factory( const LString& name ) const;

  /** Read in a suif file and make it the current FileSetBlock.
     The original FileSetBlock is deleted. */
  virtual void read( const String& inputFileName );

  /** Write the current FileSetBlock into a file named by outputFileName. */
  virtual void write( const String& outputFileName ) const;
  virtual FileSetBlock *read_more( const String& inputFileName ) const;

  virtual CloneSubSystem* get_clone_subsystem() const;

  virtual void set_file_set_block( FileSetBlock* o );
  virtual FileSetBlock* get_file_set_block() const;


  virtual TypeBuilder* get_type_builder() const;
  virtual void set_type_builder( TypeBuilder* );

  /** Send an error message to stderr */
  // use the macro suif_error
  virtual void error(  SuifObject* obj, const char* file_name,
                       int line_number,
                       const char* module_name,
                       const char* description, va_list ap );

  /** Send a warning message to stderr */
  // use the macro suif_warning
  virtual void warning( SuifObject* obj, const char* file_name,
                       int line_number,
                       const char* module_name,
                       const char* description, va_list ap );

  /** Send some information to stderr */
  // use the macro suif_information
  virtual void information( SuifObject* obj, const char* file_name,
                       int line_number,
                       const char* module_name,
                       int verbosity_level,
                       const char* description, va_list ap );

  // use the macro suif_error
  virtual void error(  const char* file_name,
                       int line_number,
                       const char* module_name,
                       const char* description, va_list ap );

  // use the macro suif_warning
  virtual void warning( const char* file_name,
                       int line_number,
                       const char* module_name,
                       const char* description, va_list ap );

  // use the macro suif_information
  virtual void information( const char* file_name,
                       int line_number,
                       const char* module_name,
                       int verbosity_level,
                       const char* description, va_list ap );

  // Purpose: to load and initialize a DLL
  //  Use "require_DLL" instead
  //
  void import_module(const LString &module_name);

  // Purpose: to load and initialize a DLL
  //  if it has not already been loaded
  //  Use "require_DLL" instead
  //
  void require_module(const LString &module_name);

  // Purpose: to load and initialize a DLL
  //  if it has not already been loaded
  //
  void require_DLL(const LString &module_name);

  void register_module(Module *module);

  String get_location(const SuifObject *obj) const;
  String to_string(const LString &style,
		   const SuifObject *obj) const;
  /* Use the default printer */
  String to_string(SuifObject *obj) const;

protected:
  typedef list<RealObjectFactory*> FactoryList;
  InputSubSystem* input_sub_system;
  OutputSubSystem* output_subsystem;
  CloneSubSystem* cloneSubSystem;
  DLLSubSystem* _dll_subsystem;
  ModuleSubSystem* _module_subsystem;
  ErrorSubSystem* _error_subsystem;
  PrintSubSystem* _print_subsystem;
  TypeBuilder* _type_builder;

  ObjectFactory* _object_factory;

  FactoryList* factories;

  FileSetBlock* _file_set_block;
  AddressMap* rudimentaryAddressMap;
private:
  SuifEnv(const SuifEnv &);
  SuifEnv& operator=(const SuifEnv &);
};


#endif

