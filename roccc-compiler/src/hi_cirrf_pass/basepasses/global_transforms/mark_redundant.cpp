// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#include <cassert>

#include <suifkernel/command_line_parsing.h>
#include <basicnodes/basic_factory.h>

#include "roccc_utils/warning_utils.h"
#include "roccc_utils/roccc2.0_utils.h"

#include "mark_redundant.h"

MarkRedundantPass::MarkRedundantPass(SuifEnv* pEnv) : 
  PipelinablePass(pEnv, "MarkRedundantPass")
{
  theEnv = pEnv ;
  procDef = NULL ;
}

void MarkRedundantPass::initialize()
{
  PipelinablePass::initialize() ;
  _command_line->set_description("Marks labels as redundant") ;
  OptionString* optionRedundantLabel = new OptionString("Redundant Label",
							&redundantLabel) ;
  OptionInt* doubleOption = new OptionInt("DoubleOrTriple", &doubleOrTriple) ;
  OptionList* arguments = new OptionList() ;
  arguments->add(optionRedundantLabel) ;
  arguments->add(doubleOption) ;
  _command_line->add(arguments) ;
}

void MarkRedundantPass::do_procedure_definition(ProcedureDefinition* proc_def)
{
  procDef = proc_def ;
  assert(procDef != NULL) ;

  OutputInformation("Marking a redundant label") ;
  list<LabelLocationStatement*>* allLabels = 
    collect_objects<LabelLocationStatement>(procDef->get_body()) ;
  
  list<LabelLocationStatement*>::iterator labelIter = allLabels->begin() ;
  while (labelIter != allLabels->end())
  {
    if ((*labelIter)->get_defined_label()->get_name() == redundantLabel)
    {
      BrickAnnote* labelBrick = create_brick_annote(theEnv, "Redundant") ;
      IntegerBrick* doubleBrick = 
	create_integer_brick(theEnv, IInteger(doubleOrTriple)) ;
      labelBrick->append_brick(doubleBrick) ;
      (*labelIter)->append_annote(labelBrick) ;
    }
    ++labelIter ;
  }

  delete allLabels ;
  OutputInformation("Done marking a redundant label") ;
}
