// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#include "common/system_specific.h"
#include <common/suif_copyright.h>

#include <iostream>
#include <iokernel/cast.h>
#include <iokernel/clone_stream.h>
#include <common/i_integer.h>
#include <basicnodes/basic_factory.h>
#include <suifnodes/suif.h>
#include <suifnodes/suif_factory.h>
#include <basicnodes/basic.h>
#include <basicnodes/basic_constants.h>
#include <suifkernel/suifkernel_messages.h>
#include <suifkernel/utilities.h> 
#include <suifkernel/group_walker.h> 
#include "transforms/procedure_walker_utilities.h"
#include <utils/expression_utils.h>
#include <utils/symbol_utils.h>
#include <utils/type_utils.h>
#include <utils/cloning_utils.h>
#include <cfenodes/cfe.h>
#include "roccc_utils/list_utils.h"
#include "roccc_utils/symbol_utils.h"
#include "roccc_utils/warning_utils.h"
#include "scalar_renaming_pass.h"

using namespace std;

// THIS PASS ELIMINATES ALL ANTI AND OUTPUT DEPENDENCIES BETWEEN SCALAR VARIABLES 
// INSIDE THE PROGRAM BY RENAMING THE VARIABLES.

/**************************** Declarations ************************************/

class srp_store_variable_statement_walker: public SelectiveWalker {
public:
  srp_store_variable_statement_walker(SuifEnv *env)
    :SelectiveWalker(env, StoreVariableStatement::get_class_name()) {}
  Walker::ApplyStatus operator () (SuifObject *x); 
};

class srp_call_statement_walker: public SelectiveWalker {
public:
  srp_call_statement_walker(SuifEnv *env)
    :SelectiveWalker(env, CallStatement::get_class_name()) {}
  Walker::ApplyStatus operator () (SuifObject *x); 
};

list<StoreVariableStatement*> renamed_list;
list<CallStatement*> renamed_call_stmt_list;

/**************************** Implementations ************************************/
ScalarRenamingPass::ScalarRenamingPass(SuifEnv *pEnv) : PipelinablePass(pEnv, "ScalarRenamingPass") {}

void ScalarRenamingPass::do_procedure_definition(ProcedureDefinition* proc_def){
  OutputInformation("Scalar Renaming begins") ;
  if (proc_def)
  {
    srp_store_variable_statement_walker walker1(get_suif_env());
    proc_def->walk(walker1);
    srp_call_statement_walker walker2(get_suif_env());
    proc_def->walk(walker2);
  }
  OutputInformation("Scalar Renaming ends") ;
}

Walker::ApplyStatus srp_store_variable_statement_walker::operator () (SuifObject *x) {
	SuifEnv *env = get_env();
	StoreVariableStatement *store_var_stmt = to<StoreVariableStatement>(x);

	assert(store_var_stmt != NULL) ;

	if(is_in_list(store_var_stmt, &renamed_list))
	   return Walker::Continue;

	VariableSymbol *store_var_stmt_destination = store_var_stmt->get_destination();

	VariableSymbol *new_var = new_anonymous_variable(env, 
							 store_var_stmt_destination->get_symbol_table(),
							 store_var_stmt_destination->get_type());
	name_variable(new_var);
	list<SuifObject*>* rename_queue = new list<SuifObject*>;
	rename_queue->push_back(store_var_stmt);

	unsigned int rename_queue_index = 0;

	while(rename_queue_index < rename_queue->size()){

	    list<SuifObject*>::iterator iter = rename_queue->get_nth(rename_queue_index);

	    if(is_a<StoreVariableStatement>(*iter)){
	       StoreVariableStatement *def = to<StoreVariableStatement>(*iter);
	       BrickAnnote *reached_uses = to<BrickAnnote>(def->lookup_annote_by_name("reached_uses")); 
	       for(Iter<SuifBrick*> iter = reached_uses->get_brick_iterator();
	           iter.is_valid(); iter.next()){
		   SuifObjectBrick *sob = to<SuifObjectBrick>(iter.current());
		   if(!is_in_list(sob->get_object(), rename_queue))
		      rename_queue->push_back(sob->get_object());
	       }
	    }else if(is_a<LoadVariableExpression>(*iter)){
	       LoadVariableExpression *use = to<LoadVariableExpression>(*iter);
	       BrickAnnote *reaching_defs = to<BrickAnnote>(use->lookup_annote_by_name("reaching_defs")); 
	       for(Iter<SuifBrick*> iter = reaching_defs->get_brick_iterator();
	           iter.is_valid(); iter.next()){
		   SuifObjectBrick *sob = to<SuifObjectBrick>(iter.current());
		   if(!is_in_list(sob->get_object(), rename_queue))
		      rename_queue->push_back(sob->get_object());
	       }
	    }

	    rename_queue_index++;
	}

	for(list<SuifObject*>::iterator iter = rename_queue->begin();
	    iter != rename_queue->end(); iter++){

	    if(is_a<StoreVariableStatement>(*iter)){
	       StoreVariableStatement *def = to<StoreVariableStatement>(*iter);
	       def->set_destination(new_var);
	       renamed_list.push_back(def);
            }else if(is_a<LoadVariableExpression>(*iter)){
	       LoadVariableExpression *use = to<LoadVariableExpression>(*iter);
	       use->set_source(new_var);
	    }
	}

    	return Walker::Continue;

}

Walker::ApplyStatus srp_call_statement_walker::operator () (SuifObject *x) {
	SuifEnv *env = get_env();
	CallStatement *call_stmt = to<CallStatement>(x);

	if(is_in_list(call_stmt, &renamed_call_stmt_list))
	   return Walker::Continue;

	// ROCCC 2.0 change.  Call statements might not have a destination.
	//  We have to find all of the definitions from the operands.

	list<VariableSymbol*> allDestinations ;
	if (call_stmt->get_destination() != NULL)
	{
	  allDestinations.push_back(call_stmt->get_destination()) ;
	}

	// For all operands, determine if it is an output variable or not.
	for(unsigned int i = 0 ; i < call_stmt->get_argument_count() ; ++i)
	{
	  if (dynamic_cast<SymbolAddressExpression*>(call_stmt->get_argument(i)))
	  {
	    Symbol* nextSym = dynamic_cast<SymbolAddressExpression*>(call_stmt->get_argument(i))->get_addressed_symbol() ;
	    VariableSymbol* nextVarSym = dynamic_cast<VariableSymbol*>(nextSym) ;
	    if (nextVarSym != NULL)
	    {
	      allDestinations.push_back(nextVarSym) ;
	    }
	  }
	}

	list<VariableSymbol*>::iterator symIter ;
	symIter = allDestinations.begin() ;
	while (symIter != allDestinations.end())
	{
	  VariableSymbol* newVar ;
	  newVar = new_unique_variable(env,
				       (*symIter)->get_symbol_table(),
				       (*symIter)->get_type()) ;
	  name_variable(newVar) ;

	  // For each variable we have to rename the next use
	  list<SuifObject*>* renameQueue = new list<SuifObject*>;
	  renameQueue->push_back(call_stmt) ;
	  unsigned int renameQueueIndex = 0 ;

	  while (renameQueueIndex < renameQueue->size())
	  {
	    list<SuifObject*>::iterator renameIter ;
	    renameIter = renameQueue->get_nth(renameQueueIndex) ;
	    if (is_a<CallStatement>(*renameIter))
	    {
	      CallStatement *def = to<CallStatement>(*renameIter);
	      BrickAnnote *reached_uses = 
		to<BrickAnnote>(def->lookup_annote_by_name("reached_uses")); 
	      assert(reached_uses != NULL) ;
	      for(Iter<SuifBrick*> iter = reached_uses->get_brick_iterator();
		  iter.is_valid(); iter.next())
	      {
		SuifObjectBrick *sob = to<SuifObjectBrick>(iter.current());
		if(!is_in_list(sob->get_object(), renameQueue))
		  renameQueue->push_back(sob->get_object());
	      }
	      
	    }
	    else if (is_a<LoadExpression>(*renameIter))
	    {
	      LoadVariableExpression *use = to<LoadVariableExpression>(*renameIter);
	      BrickAnnote *reaching_defs = to<BrickAnnote>(use->lookup_annote_by_name("reaching_defs")); 
	      for(Iter<SuifBrick*> iter = reaching_defs->get_brick_iterator();
		  iter.is_valid(); iter.next())
	      {
		SuifObjectBrick *sob = to<SuifObjectBrick>(iter.current());
		if(!is_in_list(sob->get_object(), renameQueue))
		  renameQueue->push_back(sob->get_object());
	      }

	    }
	    ++renameQueueIndex ;
	  }

	  for(list<SuifObject*>::iterator iter = renameQueue->begin();
	    iter != renameQueue->end(); iter++)
	  {
	    if(is_a<CallStatement>(*iter))
	    {
	      // This can no longer work...
	      CallStatement *def = to<CallStatement>(*iter);
	      def->set_destination(newVar);
	      renamed_call_stmt_list.push_back(def);
            }
	    else if(is_a<LoadVariableExpression>(*iter))
	    {
	      LoadVariableExpression *use = to<LoadVariableExpression>(*iter);
	      use->set_source(newVar);
	    }
	}

	  delete renameQueue ;
	  // Continue on to the next variable
	  ++symIter ;
	}

    	return Walker::Continue;
}


