
#include <cassert>

#include <basicnodes/basic.h>
#include <basicnodes/basic_factory.h>
#include <suifnodes/suif_factory.h>
#include <suifkernel/command_line_parsing.h>

#include "preferencePass.h"

#include "roccc_utils/warning_utils.h"

PreferencePass::PreferencePass(SuifEnv* pEnv) : 
  PipelinablePass(pEnv, "PreferencePass")
{
  theEnv = pEnv ;
  procDef = NULL ;
}

void PreferencePass::initialize()
{
  PipelinablePass::initialize() ;
  _command_line->set_description("Process preferences and add annotations");
  OptionList* preferenceArgs = new OptionList() ;
  OptionString* preferenceVersion = 
    new OptionString("Version", &versionString) ;

  preferenceArgs->add(preferenceVersion) ;
  _command_line->add(preferenceArgs) ;
}

void PreferencePass::do_procedure_definition(ProcedureDefinition* proc_def)
{
  procDef = proc_def ;
  assert(procDef != NULL) ;

  OutputInformation("Preference pass begins") ;

  BrickAnnote* versionAnnote = create_brick_annote(theEnv, "CompilerVersion") ;
  StringBrick* versionBrick = create_string_brick(theEnv, versionString) ;
  versionAnnote->append_brick(versionBrick) ;
  procDef->append_annote(versionAnnote) ;

  OutputInformation("Preference pass ends") ;
}
