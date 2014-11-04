
#include <cassert>

#include <basicnodes/basic.h>
#include <basicnodes/basic_factory.h>
#include <suifnodes/suif_factory.h>
#include <suifkernel/command_line_parsing.h>

#include "intrinsicPass.h"

#include "roccc_utils/warning_utils.h"

IntrinsicPass::IntrinsicPass(SuifEnv* pEnv) :
  PipelinablePass(pEnv, "IntrinsicPass")
{
  theEnv = pEnv ;
  procDef = NULL ;
}

void IntrinsicPass::initialize()
{
  PipelinablePass::initialize() ;
  _command_line->set_description("Add intrinsic annotations") ;
  OptionList* intrinsicArgs = new OptionList() ;
  OptionString* intrinsicName = new OptionString("Intrinsic", &name) ;
  intrinsicArgs->add(intrinsicName) ;
  _command_line->add(intrinsicArgs) ;
}

void IntrinsicPass::do_procedure_definition(ProcedureDefinition* proc_def)
{
  procDef = proc_def ;
  assert(procDef != NULL) ;

  OutputInformation("Intrinsic pass begins") ;

  BrickAnnote* intrinsicAnnote = create_brick_annote(theEnv, "Intrinsic") ;

  StringBrick* intrinsicBrick = create_string_brick(theEnv, name) ;
  intrinsicAnnote->append_brick(intrinsicBrick) ;
  procDef->append_annote(intrinsicAnnote) ;

  OutputInformation("Intrinsic pass ends") ;
}
