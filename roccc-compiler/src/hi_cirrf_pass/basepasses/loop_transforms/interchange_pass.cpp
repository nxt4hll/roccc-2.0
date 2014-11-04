// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

#include <cassert>
#include <sstream>

#include <suifkernel/command_line_parsing.h>
#include <suifkernel/utilities.h>

#include "roccc_utils/warning_utils.h"

#include "interchange_pass.h"

InterchangePass::InterchangePass(SuifEnv* pEnv) :
  PipelinablePass(pEnv, "InterchangePass")
{
  theEnv = pEnv ;
  procDef = NULL ;
}

InterchangePass::~InterchangePass()
{
  ; // Nothing to delete 
}

void InterchangePass::initialize()
{
  PipelinablePass::initialize() ;
  _command_line->set_description("Interchanges two loop indicies") ;
  OptionString* loopLabel1 = new OptionString("Loop label 1",
					      &loopOne) ;
  OptionString* loopLabel2 = new OptionString("Loop label 2",
					      &loopTwo) ;
  OptionList* args = new OptionList() ;
  args->add(loopLabel1) ;
  args->add(loopLabel2) ;
  _command_line->add(args) ;
}

void InterchangePass::do_procedure_definition(ProcedureDefinition* p)
{
  procDef = p ;
  assert(procDef != NULL) ;
  
  OutputInformation("Loop interchange pass begins") ;

  CForStatement* forLoopOne = NULL ;
  CForStatement* forLoopTwo = NULL ;

  list<CForStatement*>* allFors = 
    collect_objects<CForStatement>(procDef->get_body()) ;
  list<CForStatement*>::iterator forIter = allFors->begin() ;
  while (forIter != allFors->end())
  {
    BrickAnnote* labelAnnote = 
      dynamic_cast<BrickAnnote*>((*forIter)->lookup_annote_by_name("c_for_label")) ;
    if (labelAnnote != NULL)
    {
      StringBrick* currentStringBrick = 
	dynamic_cast<StringBrick*>(labelAnnote->get_brick(0)) ;
      assert(currentStringBrick != NULL) ;
      if (currentStringBrick->get_value() == loopOne)
      {
	forLoopOne = (*forIter) ;
      }
      if (currentStringBrick->get_value() == loopTwo)
      {
	forLoopTwo = (*forIter) ;
      }
    }
    ++forIter ;
  }
  delete allFors ;

  if (forLoopOne == NULL || forLoopTwo == NULL)
  {
    std::stringstream infoStream ;
    infoStream << "Could not locate both loops: " 
	       << loopOne 
	       << " and "
	       << loopTwo << std::endl ;
    OutputError(infoStream.str().c_str()) ;
  }
  else
  {
    Interchange(forLoopOne, forLoopTwo) ;
  }

  OutputInformation("Loop interchange pass ends") ;
}

// This code is based on the original interchange code and may need to be
//  changed.
void InterchangePass::Interchange(CForStatement* one, CForStatement* two)
{
  assert(one != NULL) ;
  assert(two != NULL) ;

  // Swap the before statements, the tests, and the steps

  Statement* firstBefore = one->get_before() ;
  Statement* secondBefore = two->get_before() ;

  one->set_before(NULL) ;
  two->set_before(NULL) ;
  
  one->set_before(secondBefore) ;
  two->set_before(firstBefore) ;

  Expression* firstTest = one->get_test() ;
  Expression* secondTest = two->get_test() ;

  one->set_test(NULL) ;
  two->set_test(NULL) ;
  
  one->set_test(secondTest) ;
  two->set_test(firstTest) ;

  Statement* firstStep = one->get_step() ;
  Statement* secondStep = two->get_step() ;
  
  one->set_step(NULL) ;
  two->set_step(NULL) ;
  
  one->set_step(secondStep) ;
  two->set_step(firstStep) ;
}
