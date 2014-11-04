
#include <cassert>

#include <basicnodes/basic.h>
#include <basicnodes/basic_factory.h>

#include "markSystemToSystemPass.h"

#include "roccc_utils/identificationUtilities.h"
#include "roccc_utils/warning_utils.h"

MarkSystemToSystemPass::MarkSystemToSystemPass(SuifEnv* pEnv) :
  PipelinablePass(pEnv, "MarkSystemToSystemPass")
{
  theEnv = pEnv ;
  procDef = NULL ;
}

MarkSystemToSystemPass::~MarkSystemToSystemPass()
{
  ; // Nothing to delete
}

void MarkSystemToSystemPass::do_procedure_definition(ProcedureDefinition* p)
{
  procDef = p ;
  assert(procDef != NULL) ;
  OutputInformation("Mark system to system pass begins") ;
  if (isComposedSystem(procDef))
  {
    procDef->append_annote(create_brick_annote(theEnv, "ComposedSystem")) ;
  }
  OutputInformation("Mark system to system pass ends") ;
}
