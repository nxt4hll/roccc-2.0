#ifndef SUIFPASSES__DISPATCHER_H
#define SUIFPASSES__DISPATCHER_H

#include "passes.h"

class PipelinerDispatchPass : public PipelinablePass {
public:
  PipelinerDispatchPass( SuifEnv* suif_env, 
                         suif_vector<PipelinablePass*>* modules );

  virtual ~PipelinerDispatchPass();

  virtual Module *clone() const;

  virtual void execute();

  virtual void do_file_set_block( FileSetBlock* file_set_block );

  virtual void do_file_block( FileBlock* file_block );

  virtual void do_definition_block( DefinitionBlock* definition_block );

  virtual void do_procedure_definition( ProcedureDefinition* proc_def );

  virtual void do_variable_definition( VariableDefinition* var_def );
  
private:
  suif_vector<PipelinablePass*>* _modules;
  int _module_count;
};


#endif

