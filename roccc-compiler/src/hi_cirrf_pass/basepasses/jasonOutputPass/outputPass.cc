// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

/*

  This file contains the SUIF pass that will convert a function with
   a struct as input and output parameters into a Hi-CIRRF file as specified
   in ROCCC 2.0.

  Before this pass is called, one must make sure that the VerifyPass is called.
   That pass makes sure we are dealing with a ROCCC 2.0 function and 
   may put annotations that this pass is looking for.

  The output pass will work without the annotations, but will not work if 
   the function is not ROCCC 2.0 compliant.

*/

#include <cassert>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cfenodes/cfe.h>
#include <basicnodes/basic_factory.h>
#include <suifkernel/command_line_parsing.h>

#include "suifkernel/utilities.h"
#include "roccc_extra_types/array_info.h"
#include "roccc_utils/warning_utils.h"
#include "roccc_utils/roccc2.0_utils.h"
#include "outputPass.h"

#include "systemOutput.h"
#include "moduleOutput.h"
#include "composedOutput.h"

// When this pass is constructed, open up both of our output files.  These
//  files are called "hi_cirrf.c" and "roccc.h"

OutputPass::OutputPass(SuifEnv* pEnv) : PipelinablePass(pEnv, "OutputPass")
{
  theEnv = pEnv ;
  procDef = NULL ;
}

OutputPass::~OutputPass()
{
}

void OutputPass::do_procedure_definition(ProcedureDefinition* proc_def)
{
  procDef = proc_def ;
  assert(procDef != NULL) ;

  // Look up if this is a module or system.  This is stored as the ROCCC
  //  function type created in the verify pass.
  BrickAnnote* rocccType = 
    dynamic_cast<BrickAnnote*>(procDef->lookup_annote_by_name("FunctionType"));

  assert(rocccType != NULL) ;
  IntegerBrick* valueBrick = 
    dynamic_cast<IntegerBrick*>(rocccType->get_brick(0)) ;
  assert(valueBrick != 0) ;

  int functionType = valueBrick->get_value().c_int() ;

  if (functionType == 1)
  {
    theGenerator = new ModuleGenerator(theEnv, procDef) ;
  }
  else if (functionType == 2)
  {
    theGenerator = new SystemGenerator(theEnv, procDef, streamFileName) ;
  }
  else if (functionType == 3)
  {
    theGenerator = new ComposedGenerator(theEnv, procDef, streamFileName) ;
  }
  else
  {
    OutputError("Unknown ROCCC function type!") ;
    assert(0) ;
  }

  theGenerator->Setup() ;
  theGenerator->Output() ;
}

void OutputPass::initialize()
{
  PipelinablePass::initialize() ;
  _command_line->set_description("Output Hi-CIRRF after all transformations") ;
  OptionString* optionFilename = new OptionString("Stream Info File",
						  &streamFileName) ;
  OptionList* outputPassArgs = new OptionList() ;
  outputPassArgs->add(optionFilename) ;
  _command_line->add(outputPassArgs) ;  
}
