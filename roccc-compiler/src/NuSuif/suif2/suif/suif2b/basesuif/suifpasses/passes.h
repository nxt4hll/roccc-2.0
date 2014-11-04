#ifndef SUIFPASSES__PASSES_H
#define SUIFPASSES__PASSES_H

#include "suifkernel/module.h"
#include "suifkernel/suifkernel_forwarders.h"

/** 
 * \file passes.h
 * Defines the definition of a Pass, a PipelinablePass and a FrontendPass
 *
 */

/**
 * \class Pass passes.h suifpasses/passes.h
 * A Pass is a Module to be applied to the current FileSetBlock
 * in the suif_env
 *
 */

// ABSTRACT CLASS
class Pass : public Module {
protected:
  Pass( SuifEnv* suif_env, const LString &name );
  virtual ~Pass();

public:
  /** Initialize the pass */
  virtual void initialize();

  /** the default implementation invokes the method do_file_set_block */
  virtual void execute();
  /** May return "this" if the pass has no state. */
  virtual Module* clone() const = 0;

  /** your analysis or optimization goes here */
  virtual void do_file_set_block( FileSetBlock* file_set_block ) = 0;
};


/**
 * \class PipelinablePass passes.h suifpasses/passes.h
 * A Pass is Pipelinable if its computation can be applied 
 * to each procedure independently.  In the pipelined execution mode,
 * the system will apply all the computations from the PipelinablePasses
 * to a procedure before moving to another.  This execution order 
 * should have a better data locality. 
 */

class PipelinablePass : public Pass {
public:
   PipelinablePass( SuifEnv* suif_env, const LString &name );
   virtual ~PipelinablePass() = 0;

   /** Initialize the pass */
   virtual void initialize();

   /** will execute the Pass as if it is not pipelined */
   virtual void execute();
 
   /** May return "this" if the pass has no state. */
   virtual Module* clone() const = 0;

  /** 
   * Override this if computation is to be applied to a file_set_block.
   * The default is empty.
   */
      
   virtual void do_file_set_block( FileSetBlock* file_set_block );
  /** 
   * Override this if computation is to be applied to a file_block.
   * The default is empty.
   */

   virtual void do_file_block( FileBlock* file_block );

  /** 
   * Override this if computation is to be applied to a procedure_definition.
   * The default is empty.
   */
   virtual void do_procedure_definition( ProcedureDefinition* proc_def );
  /** 
   * Override this if computation is to be applied to a variable_definition.
   * The default is empty.
   */
   virtual void do_variable_definition( VariableDefinition* var_def );
  /** 
   * Override this if computation is to be executed after all processing is done.
   * The default is empty.
   */
   virtual void finalize(){};
};

/**
 * \class FrontendPass passes.h suifpasses/passes.h
 * A pass that reads in a SUIF IR representation
 */

// ABSTRACT CLASS
class FrontendPass : public Module {
protected:
  FrontendPass( SuifEnv* suif_env, const LString &name );
  virtual ~FrontendPass();

public:
  virtual void initialize();
  /**
   * The default implementation invokes build_file_set_block
   * and stores the returned pointer.
  */
  virtual void execute();

  // your stuff goes here
  virtual FileSetBlock *build_file_set_block() = 0;
};


#endif
