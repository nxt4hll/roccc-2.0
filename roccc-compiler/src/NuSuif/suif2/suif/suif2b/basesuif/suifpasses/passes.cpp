#include "common/system_specific.h"
#include "passes.h"

#include "suifkernel/suif_env.h"
#include "suifkernel/module.h"

#include "dispatcher.h"

#include "common/suif_vector.h"



Pass::Pass( SuifEnv* suif_env, const LString &name ) :
            Module( suif_env, name ) {
}


Pass::~Pass() {
}

void Pass::initialize() {
  Module::initialize();
  set_interface("Pass", 0);
}



void Pass::execute() {
  FileSetBlock *fsb = _suif_env->get_file_set_block();
  if (fsb == NULL)
    suif_warning("%s: Pass execution attempted without a file set block\n",
		 get_module_name().c_str());
  else
    do_file_set_block( fsb );
}


PipelinablePass::PipelinablePass( SuifEnv* suif_env, const LString &name ) :
           Pass( suif_env, name ) {
}


PipelinablePass::~PipelinablePass() {
}

void PipelinablePass::initialize() {
  Pass::initialize();
  set_interface("PipelinablePass", 0);
}

void PipelinablePass::execute() {
  suif_vector<PipelinablePass*> pipelined_modules;
  pipelined_modules.push_back( this );
  PipelinerDispatchPass dispatch_pass( _suif_env, &pipelined_modules );
  dispatch_pass.execute();
}

void PipelinablePass::do_file_set_block(
                FileSetBlock* file_set_block ) {
}


void PipelinablePass::do_file_block(
                FileBlock* file_block ) {
}


void PipelinablePass::do_procedure_definition(
                ProcedureDefinition* proc_def ) {
}


void PipelinablePass::do_variable_definition(
                VariableDefinition* var_def ) {
}

//
// FrontendPass
//

FrontendPass::FrontendPass( SuifEnv* suif_env, const LString &name ) :
            Module( suif_env, name ) {
}

FrontendPass::~FrontendPass() {
}


void FrontendPass::initialize() {
  Module::initialize();
  set_interface("FrontendPass", 0);
}

void FrontendPass::execute() {
  FileSetBlock *fsb = build_file_set_block();
  get_suif_env()->set_file_set_block( fsb );
}



