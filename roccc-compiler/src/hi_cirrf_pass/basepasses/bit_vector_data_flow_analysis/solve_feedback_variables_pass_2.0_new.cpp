// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

/*

  Variables are identified as feedback if there is a load before a store
   inside the innermost loop.
 
*/

#include <cassert>
#include <map>

#include "solve_feedback_variables_pass_2.0_new.h"
#include <suifnodes/suif.h>
#include <suifnodes/suif_factory.h>
#include <basicnodes/basic.h>
#include <basicnodes/basic_factory.h>
#include <cfenodes/cfe.h>
#include <suifkernel/command_line_parsing.h>
#include "suifkernel/utilities.h"
#include <basicnodes/basic_factory.h>
#include "roccc_utils/warning_utils.h"
#include "roccc_utils/roccc2.0_utils.h"

SolveFeedbackVariablesPass3::SolveFeedbackVariablesPass3(SuifEnv *pEnv) : 
  PipelinablePass(pEnv, "SolveFeedbackVariablesPass3") 
{
  procDef = NULL ;
  theEnv = pEnv ;

  innermost = NULL ;
  isSystolic = false ;
}

void SolveFeedbackVariablesPass3::do_procedure_definition(ProcedureDefinition* proc_def)
{
  procDef = proc_def ;
  assert(procDef != NULL) ;

  OutputInformation("Solve feedback variables 2.0 pass begins") ;

  // Feedback variables should only exist in loops, and I am currently
  //  assuming one loop nest.  I need to find the innermost for loop
  //  and base all of my 

  innermost = InnermostLoop(procDef) ;

  if (innermost == NULL)
  {
    OutputInformation("Solve feedback variables 2.0 pass ends") ;
    return ;
  }

  DetermineNewFeedbacks() ;

  SetupAnnotations() ;

  OutputInformation("Solve feedback variables 2.0 pass ends") ;
}

// This is a vastly simpler way to setup annotations that doesn't worry
//  or assume that all feedbacks come from a single store variable statement
void SolveFeedbackVariablesPass3::SetupAnnotations()
{
  assert(theEnv != NULL) ;
  // Go through the list of all feedback instances we have discovered.
  list<PossibleFeedbackPair>::iterator feedbackIter = actualFeedbacks.begin();
  while (feedbackIter != actualFeedbacks.end())
  {
    // For every feedback, we need to create a two variables
    //  The first one is a replacement for the original use.
    //  The second one is the variable used for feedback.

    LString replacementName = (*feedbackIter).varSym->get_name() ;
    replacementName = replacementName + "_replacementTmp" ;

    VariableSymbol* replacementVariable = 
      create_variable_symbol(theEnv,
			     (*feedbackIter).varSym->get_type(),
			     TempName(replacementName)) ;
    procDef->get_symbol_table()->append_symbol_table_object(replacementVariable);

    VariableSymbol* feedbackVariable =
	create_variable_symbol(theEnv,
			       (*feedbackIter).varSym->get_type(),
			       TempName(LString("feedbackTmp"))) ;
    procDef->get_symbol_table()->append_symbol_table_object(feedbackVariable) ;
    
    // Replace the use with this new feedback variable
    (*feedbackIter).use->set_source(replacementVariable) ;

    // I am looking for the variable that originally defined me and 
    //  the variable that will replace me.
    Statement* definition = (*feedbackIter).definition ;
    VariableSymbol* definitionVariable = 
      GetDefinedVariable(definition, (*feedbackIter).definitionLocation) ;
    assert(definitionVariable != NULL) ;

    // Finally, annotate the feedback variable
    SetupAnnotations(feedbackVariable,		     
		     definitionVariable,
		     replacementVariable) ;
    
    replacementVariable->append_annote(create_brick_annote(theEnv,
							   "NonInputScalar"));

    ++feedbackIter ;
  }  
}

void SolveFeedbackVariablesPass3::SetupAnnotations(VariableSymbol* toAnnote,
						   VariableSymbol* source,
						   VariableSymbol* destination)
{

  // Clean up any annotation that previously existed
  if (toAnnote->lookup_annote_by_name("FeedbackVariable") != NULL)
  {
    // Do I need to remove the bricks associated with this as well, or
    //  does the destructor handle that?
    delete toAnnote->remove_annote_by_name("FeedbackVariable") ;
  }

  // Now create the annotation we will use
  BrickAnnote* feedbackAnnote = 
    to<BrickAnnote>(create_brick_annote(theEnv, "FeedbackVariable")) ;

  // Create the bricks and add them to the annotation
  feedbackAnnote->append_brick(create_suif_object_brick(theEnv, destination)) ;
  feedbackAnnote->append_brick(create_suif_object_brick(theEnv, source)) ;

  // And finally attach the annotation to the variable
  toAnnote->append_annote(feedbackAnnote) ;  
  
  // Mark this as non systolic if appropriate
  if (!isSystolic)
  {
    toAnnote->append_annote(create_brick_annote(theEnv,"NonSystolic")) ;
  }
}

void SolveFeedbackVariablesPass3::initialize()
{
  PipelinablePass::initialize() ;
  _command_line->set_description("Solves feedback variables") ;
  OptionInt* optionIsSystolic = new OptionInt("IsSystolic", &isSystolic) ;
  OptionList* feedbackArguments = new OptionList() ;
  feedbackArguments->add(optionIsSystolic) ;
  _command_line->add(feedbackArguments) ;
}

// Modified to handle cases where a variable is defined for the first time
//  in a call and used in a store variable.
void SolveFeedbackVariablesPass3::DetermineNewFeedbacks()
{
  assert(innermost != NULL) ;
  assert(procDef != NULL) ;
  assert(theEnv != NULL) ;

  StatementList* bodyList = 
    dynamic_cast<StatementList*>(innermost->get_body()) ;
  assert(bodyList != NULL) ;

  std::map<VariableSymbol*, Statement*> lastDefinitions ;
  std::map<VariableSymbol*, int> lastLocations ;

  // Go backwards through the list and determine the last definition
  //  for each variable.
  for (int i = bodyList->get_statement_count() - 1 ; i >= 0 ; --i)
  {
    list<VariableSymbol*> definedVariables = 
      AllDefinedVariables(bodyList->get_statement(i)) ;
    
    list<VariableSymbol*>::iterator defIter = definedVariables.begin() ;
    int j = 0 ;
    while (defIter != definedVariables.end())
    {
      if (lastDefinitions[(*defIter)] == NULL)
      {
	lastDefinitions[(*defIter)] = bodyList->get_statement(i) ;
	lastLocations[(*defIter)] = j ;
      }
      ++defIter ;
      ++j ;
    }
  }

  std::map<VariableSymbol*, int> killedVariables ;

  // Now go from the front and create actual feedbacks for each
  //  live variable.
  for (int i = 0 ; i < bodyList->get_statement_count() ; ++i)
  {
    list<LoadVariableExpression*>* allLoads = 
      collect_objects<LoadVariableExpression>(bodyList->get_statement(i)) ;
    list<LoadVariableExpression*>::iterator loadIter = allLoads->begin() ;
    while(loadIter != allLoads->end())
    {
      if (killedVariables[(*loadIter)->get_source()] == 0 &&
	  lastDefinitions[(*loadIter)->get_source()] != NULL)
      {
	// Create a feedback here
	PossibleFeedbackPair toAdd ;
	toAdd.varSym = (*loadIter)->get_source() ;
	toAdd.use = (*loadIter) ;
	toAdd.definition = lastDefinitions[(*loadIter)->get_source()] ;
	toAdd.definitionLocation = lastLocations[(*loadIter)->get_source()] ;
	actualFeedbacks.push_back(toAdd) ;
      }
      ++loadIter ;
    }
    delete allLoads ;

    // Add all the definitions to the list of killed variables.
    list<VariableSymbol*> definedVariables = 
      AllDefinedVariables(bodyList->get_statement(i)) ;
    list<VariableSymbol*>::iterator defIter = definedVariables.begin() ;
    while (defIter != definedVariables.end())
    {
      killedVariables[(*defIter)] = 1 ;
      ++defIter ;
    }
  }

  
  
}
