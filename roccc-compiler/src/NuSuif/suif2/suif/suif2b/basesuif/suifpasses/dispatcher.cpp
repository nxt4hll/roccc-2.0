#include "common/system_specific.h"
#include "dispatcher.h"

#include "basicnodes/basic.h"

#include "common/suif_vector.h"

PipelinerDispatchPass::PipelinerDispatchPass( SuifEnv* suif_env, suif_vector<PipelinablePass*>* modules ) :
    PipelinablePass( suif_env, "dispatch-pass" ),
    _modules( modules ),
    _module_count( modules->size() ) {
  _module_name = LString( "dispatch-pass" );
}


PipelinerDispatchPass::~PipelinerDispatchPass() {
}


Module *PipelinerDispatchPass::clone() const {
  return (Module*)this;
}

void PipelinerDispatchPass::execute() {
  Pass::execute();
}

void PipelinerDispatchPass::do_file_set_block(
	    FileSetBlock* file_set_block ) {
            
    {for ( int i = 0 ; i < _module_count ; i++ ) {
      (*_modules)[i]->do_file_set_block( file_set_block );
    }}
    
    for ( Iter<FileBlock*> iter = file_set_block -> get_file_block_iterator();
	  iter.is_valid();
	  iter.next() ) {
      do_file_block( iter.current() );
    }

    {for ( int i = 0 ; i < _module_count ; i++ ) {
      (*_modules)[i]->finalize();
    }}
}


void PipelinerDispatchPass::do_file_block(
            FileBlock* file_block ) {
  for ( int i = 0 ; i < _module_count ; i++ ) {
     (*_modules)[i]->do_file_block( file_block );
  }
  do_definition_block( file_block -> get_definition_block() );
}


void PipelinerDispatchPass::do_definition_block(
            DefinitionBlock* definition_block ) {
{
  for ( Iter<VariableDefinition*> iter = definition_block-> get_variable_definition_iterator();
         iter.is_valid() ;
         iter.next() ) {
     do_variable_definition( iter.current() );
  }
}

  for ( Iter<ProcedureDefinition*> iter = definition_block-> get_procedure_definition_iterator();
        iter.is_valid();
        iter.next() ) {
    do_procedure_definition( iter.current() );
   }
}


void PipelinerDispatchPass::do_procedure_definition(
	    ProcedureDefinition* proc_def ) {
  for ( int i = 0 ; i < _module_count ; i++ ) {
     (*_modules)[i]->do_procedure_definition( proc_def );
  }
  do_definition_block( proc_def -> get_definition_block() );
}


void PipelinerDispatchPass::do_variable_definition(
	    VariableDefinition* var_def ) {
  for ( int i = 0 ; i < _module_count ; i++ ) {
     (*_modules)[i]->do_variable_definition( var_def );
  }
}











