// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#include <cassert>

#include "roccc_utils/warning_utils.h"
#include "pipelined_unrolling.h"

PipelinedUnrollingPass::PipelinedUnrollingPass(SuifEnv* pEnv) :
  PipelinablePass(pEnv, "PipelinedUnrollingPass")
{
  theEnv = pEnv ;
  procDef = NULL ;
}

void PipelinedUnrollingPass::do_procedure_definition(ProcedureDefinition* proc_def)
{
  procDef = proc_def ;
  assert(procDef != NULL) ;

  OutputInformation("Pipelined unrolling begins") ;

  ProcessArgs() ;

  OutputInformation("Pipelined unrolling ends") ;
}

void PipelinedUnrollingPass::initialize()
{
  PipelinablePass::initialize() ;
  _command_line->set_description("") ;
}
