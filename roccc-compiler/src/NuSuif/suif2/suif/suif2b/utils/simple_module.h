#ifndef _SIMPLE_MODULE_H_
#define _SIMPLE_MODULE_H_

/**
  * @file
  * This file contains SimpleModule, a simplified abstract subclass of Module.
  */


#include <stdio.h>
#include "suifkernel/module.h"


#include "common/suif_vector.h"
#include "suifkernel/suif_env.h"

/** A simplified version of the Module class.
  *
  * Example on how to use SimpleModule:
  *\code
  *  class MyPass : public SimpleModule {
  *    public:
  *      MyPass(SuifEnv* senv) : SimpleModule("my_pass", senv) {};
  *      virtual void execute(suif_vector<LString>*);
  *      virtual String get_description(void) const;
  * }
  *
  * extern "C" void EXPORT init_my_pass(SuifEnv* suif_env) {
  *   ModuleSubSystem* module_subsystem = suif_env->get_module_subsystem();
  *   module_subsystem->test_and_register_module(new MyPass(suif_env));
  * }
  *\endcode
  *
  * The SimpleModule is designed so that it is reused in every invocation.
  * I.e. it will not be cloned or deleted by the suifdriver.
  *
  * The execute() method of Module is replace by SimpleModule::execute(),
  * which takes a vector of strings as command line arguments.  The subclass
  * of SimpleModule is expected to define this method.  This method is
  * responsible for checking and interpreting the command line arguments.
  *
  * If the command line argument consists of only "-h", or "-help", or
  * "-?", then a string from get_description() will be printed on cerr.
  * And the execute() method will not be called.
  */
class SimpleModule : public Module {
 private:
  suif_vector<LString> _arguments;

 public:
  /** Constructor.
    * @param suif_env a SuifEnv.
    * @param name the command name.
    */
  SimpleModule(SuifEnv* suif_env, LString name);

  /** This method should implement the function for this module.
    * @param args command line arguments.
    *
    * @see check_file_set_block
    * @see check_arg_count
    * @see get_file_set_block
    */
  virtual void execute(suif_vector<LString>* args) =0;

  /** Get a human readable help string.
    * @return a help string.
    *
    * deprecated: replaced by get_description().
    */
  virtual String get_help_string(void) const;

  /** Get a human readable help string.
    */
  virtual String get_description(void) const;

  /** Destructor.
    */
  virtual ~SimpleModule();

  /** @internal
    */
  virtual Module* clone() const;

  /** @internal
    */
  virtual void execute();

  /** @internal
    */
  virtual bool delete_me() const;

  /** @internal
    */
  virtual bool parse_command_line( TokenStream* command_line_stream );

  /** Get the FileSetBlock in current SuifEnv.
    */
  virtual FileSetBlock* get_file_set_block(void);
  
  /** Check that the current SuifEnv has a FileSetBlock.
    * @exception SuifException  if a FileSetBlock is not loaded in SuifEnv.
    *
    * This method is designed to be called inside execute().
    */
  void check_file_set_block(void);

  /** Check number of arguments.
    * @param args  the vector of arguments.
    * @param cnt   the right number of arguments.
    * @exception SuifException  if the number of arguments in \a args is not
    *            exactly \a cnt.
    *
    * This method is designed to be called inside execute().
    */
  void check_arg_count(suif_vector<LString>* args, unsigned cnt);
};

#endif /* _SIMPLE_MODULE_H_ */
