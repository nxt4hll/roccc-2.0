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
#include "roccc_extra_types/bit_vector.h"
#include "roccc_utils/annote_utils.h"
#include "roccc_utils/IR_utils.h"
#include "roccc_utils/list_utils.h"
#include "roccc_utils/bit_vector_annote_utils.h"
#include "roccc_utils/bit_vector_data_flow_utils.h"
#include "roccc_utils/warning_utils.h"
#include "reverse_copy_propagation_pass.h"

using namespace std;

/**************************** Declarations ************************************/

class rcp_store_variable_statement_walker: public SelectiveWalker {
public:
  rcp_store_variable_statement_walker(SuifEnv *env)
    :SelectiveWalker(env, StoreVariableStatement::get_class_name()) {}
  Walker::ApplyStatus operator () (SuifObject *x); 
};

class rcp_call_statement_walker: public SelectiveWalker {
public:
  rcp_call_statement_walker(SuifEnv *env)
    :SelectiveWalker(env, CallStatement::get_class_name()) {}
  Walker::ApplyStatus operator () (SuifObject *x); 
};

/**************************** Implementations ************************************/
ReverseCopyPropagationPass::ReverseCopyPropagationPass(SuifEnv *pEnv) : PipelinablePass(pEnv, "ReverseCopyPropagationPass") {}

void ReverseCopyPropagationPass::do_procedure_definition(ProcedureDefinition* proc_def){
  OutputInformation("Reverse copy propagation pass begins") ;
  if (proc_def)
  {
    rcp_store_variable_statement_walker walker2(get_suif_env());
    proc_def->walk(walker2);
    rcp_call_statement_walker walker3(get_suif_env());
    proc_def->walk(walker3);
  }
  OutputInformation("Reverse copy propagation pass ends") ;
}

Walker::ApplyStatus rcp_store_variable_statement_walker::operator () (SuifObject *x) {
	SuifEnv *env = get_env();
	StoreVariableStatement *store_var_stmt = to<StoreVariableStatement>(x);

	BrickAnnote *reached_uses = to<BrickAnnote>(store_var_stmt->lookup_annote_by_name("reached_uses")); 
	Iter<SuifBrick*> iter = reached_uses->get_brick_iterator();
	if(!iter.is_valid()) 
	   return Walker::Continue;

	for( ; iter.is_valid(); iter.next()){

	    SuifObjectBrick *sob = to<SuifObjectBrick>(iter.current());
            LoadVariableExpression *use = to<LoadVariableExpression>(sob->get_object());

	    BrickAnnote *reaching_defs = to<BrickAnnote>(use->lookup_annote_by_name("reaching_defs")); 
	    if(reaching_defs->get_brick_count() != 1)
	       return Walker::Continue;
	}

	list<StoreVariableStatement*> candidate_store_vars; 

	for(Iter<SuifBrick*> iter = reached_uses->get_brick_iterator();
	    iter.is_valid(); iter.next()){

	    SuifObjectBrick *sob = to<SuifObjectBrick>(iter.current());
            LoadVariableExpression *use = to<LoadVariableExpression>(sob->get_object());

            Statement *use_parent = get_expression_owner(use);
  	    if(!is_a<StoreVariableStatement>(use_parent))
	       continue;

            StoreVariableStatement *def = to<StoreVariableStatement>(use_parent);
   	    if(!is_a<LoadVariableExpression>(def->get_value()))
	       continue;
	    else candidate_store_vars.push_back(def);

	}

	if(candidate_store_vars.empty())
	   return Walker::Continue;

	BrickAnnote *in_stmts = to<BrickAnnote>(store_var_stmt->lookup_annote_by_name("in_stmts")); 
        SuifObjectBrick *sob = to<SuifObjectBrick>(in_stmts->get_brick(0));
        BitVector *bv = (BitVector*)(sob->get_object());
        BitVectorMap *bv_map = bv->get_bit_vector_map();

	VariableSymbol *replacement_sym = NULL;	
	for(list<StoreVariableStatement*>::iterator iter = candidate_store_vars.begin();
	    iter != candidate_store_vars.end(); ){

	    StoreVariableStatement *def = *iter;

	    BrickAnnote *def_in_stmts = to<BrickAnnote>(def->lookup_annote_by_name("in_stmts")); 
	    if(!annotes_equal_for_sym(in_stmts, def_in_stmts, NULL, def->get_destination())){
	       iter = candidate_store_vars.erase(iter);
	       continue;
	    }else iter++;

            list<int>* kill_set_list = bv_map->get_kill_set(def->get_destination());

            list<int>::iterator iter2 = kill_set_list->begin();
            for( ; iter2 != kill_set_list->end(); iter2++){

                if(bv->is_marked(*iter2)){

                   AnnotableObject *ao = to<AnnotableObject>(bv_map->reverse_lookup(*iter2));

		   if(ao->get_parent() == NULL)
		      continue;

                   BrickAnnote *reached_uses = to<BrickAnnote>(ao->lookup_annote_by_name("reached_uses"));
   	           Iter<SuifBrick*> iter3 = reached_uses->get_brick_iterator();
   	           for( ; iter3.is_valid(); iter3.next()){

 	               sob = to<SuifObjectBrick>(iter3.current());
                       LoadVariableExpression *use = to<LoadVariableExpression>(sob->get_object());
		       if(is_textually_preceeding(get_expression_owner(use), def, NULL) &&
			  is_textually_preceeding(store_var_stmt, get_expression_owner(use), NULL))
	    	          break;
		   }

		   if(iter3.is_valid())
		      break;
                }
            }

	    if(iter2 != kill_set_list->end())
	       continue;

	    replacement_sym = def->get_destination();
	}

 	if(replacement_sym == NULL || replacement_sym == store_var_stmt->get_destination())
	   return Walker::Continue;

        store_var_stmt->set_destination(replacement_sym);

	for(Iter<SuifBrick*> iter = reached_uses->get_brick_iterator();
	    iter.is_valid(); iter.next()){

	    sob = to<SuifObjectBrick>(iter.current());
            LoadVariableExpression *use = to<LoadVariableExpression>(sob->get_object());
	    use->set_source(replacement_sym);
	}

    	return Walker::Continue;
}


Walker::ApplyStatus rcp_call_statement_walker::operator () (SuifObject *x) {
	SuifEnv *env = get_env();
	CallStatement *call_stmt = to<CallStatement>(x);

	BrickAnnote *reached_uses = to<BrickAnnote>(call_stmt->lookup_annote_by_name("reached_uses")); 
	Iter<SuifBrick*> iter = reached_uses->get_brick_iterator();
	if(!iter.is_valid()) 
	   return Walker::Continue;

	for( ; iter.is_valid(); iter.next()){

	    SuifObjectBrick *sob = to<SuifObjectBrick>(iter.current());
            LoadVariableExpression *use = to<LoadVariableExpression>(sob->get_object());

	    BrickAnnote *reaching_defs = to<BrickAnnote>(use->lookup_annote_by_name("reaching_defs")); 
	    if(reaching_defs->get_brick_count() != 1)
	       return Walker::Continue;
	}

	list<StoreVariableStatement*> candidate_store_vars; 

	for(Iter<SuifBrick*> iter = reached_uses->get_brick_iterator();
	    iter.is_valid(); iter.next()){

	    SuifObjectBrick *sob = to<SuifObjectBrick>(iter.current());
            LoadVariableExpression *use = to<LoadVariableExpression>(sob->get_object());

            Statement *use_parent = get_expression_owner(use);
  	    if(!is_a<StoreVariableStatement>(use_parent))
	       continue;

            StoreVariableStatement *def = to<StoreVariableStatement>(use_parent);
   	    if(!is_a<LoadVariableExpression>(def->get_value()))
	       continue;
	    else candidate_store_vars.push_back(def);

	}

	if(candidate_store_vars.empty())
	   return Walker::Continue;

	BrickAnnote *in_stmts = to<BrickAnnote>(call_stmt->lookup_annote_by_name("in_stmts")); 
        SuifObjectBrick *sob = to<SuifObjectBrick>(in_stmts->get_brick(0));
        BitVector *bv = (BitVector*)(sob->get_object());
        BitVectorMap *bv_map = bv->get_bit_vector_map();

	VariableSymbol *replacement_sym = NULL;	
	for(list<StoreVariableStatement*>::iterator iter = candidate_store_vars.begin();
	    iter != candidate_store_vars.end(); ){

	    StoreVariableStatement *def = *iter;

	    BrickAnnote *def_in_stmts = to<BrickAnnote>(def->lookup_annote_by_name("in_stmts")); 
	    if(!annotes_equal_for_sym(in_stmts, def_in_stmts, NULL, def->get_destination())){
	       iter = candidate_store_vars.erase(iter);
	       continue;
	    }else iter++;

            list<int>* kill_set_list = bv_map->get_kill_set(def->get_destination());

            list<int>::iterator iter2 = kill_set_list->begin();
            for( ; iter2 != kill_set_list->end(); iter2++){

                if(bv->is_marked(*iter2)){

                   AnnotableObject *ao = to<AnnotableObject>(bv_map->reverse_lookup(*iter2));

		   if(ao->get_parent() == NULL)
		      continue;

                   BrickAnnote *reached_uses = to<BrickAnnote>(ao->lookup_annote_by_name("reached_uses"));
                   Iter<SuifBrick*> iter3 = reached_uses->get_brick_iterator();
                   for( ; iter3.is_valid(); iter3.next()){

                       sob = to<SuifObjectBrick>(iter3.current());
                       LoadVariableExpression *use = to<LoadVariableExpression>(sob->get_object());
    	               if(is_textually_preceeding(get_expression_owner(use), def, NULL) &&
			  is_textually_preceeding(call_stmt, get_expression_owner(use), NULL))
	                  break;
	           }

		   if(iter3.is_valid())
		      break;
                }
            }

	    if(iter2 != kill_set_list->end())
	       continue;

	    replacement_sym = def->get_destination();
	}

 	if(replacement_sym == NULL || replacement_sym == call_stmt->get_destination())
	   return Walker::Continue;
        
	call_stmt->set_destination(replacement_sym);

	for(Iter<SuifBrick*> iter = reached_uses->get_brick_iterator();
	    iter.is_valid(); iter.next()){

	    sob = to<SuifObjectBrick>(iter.current());
            LoadVariableExpression *use = to<LoadVariableExpression>(sob->get_object());
	    use->set_source(replacement_sym);
	}

    	return Walker::Continue;
}



