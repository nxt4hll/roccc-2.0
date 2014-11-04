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
#include "copy_propagation_pass.h"

using namespace std;

/**************************** Declarations ************************************/

class cpyp_load_variable_expression_walker: public SelectiveWalker {
public:
  cpyp_load_variable_expression_walker(SuifEnv *env)
    :SelectiveWalker(env, LoadVariableExpression::get_class_name()) {}
  Walker::ApplyStatus operator () (SuifObject *x); 
};

extern list<Statement*>* to_be_removed;

/**************************** Implementations ************************************/
CopyPropagationPass::CopyPropagationPass(SuifEnv *pEnv) : PipelinablePass(pEnv, "CopyPropagationPass") {}

void CopyPropagationPass::do_procedure_definition(ProcedureDefinition* proc_def){
  OutputInformation("Copy propagation pass begins") ;
    if (proc_def){
	to_be_removed = new list<Statement*>;

    	cpyp_load_variable_expression_walker walker(get_suif_env());
 	proc_def->walk(walker);

	remove_statements(to_be_removed);

        delete to_be_removed;
    }
    OutputInformation("Copy Propagation pass ends") ;
}

Walker::ApplyStatus cpyp_load_variable_expression_walker::operator () (SuifObject *x) {
	SuifEnv *env = get_env();
	LoadVariableExpression *load_var_expr = to<LoadVariableExpression>(x);

	Statement *load_parent = get_expression_owner(load_var_expr);
	BrickAnnote *in_stmts = to<BrickAnnote>(load_parent->lookup_annote_by_name("in_stmts")); 
	BrickAnnote *reaching_defs = to<BrickAnnote>(load_var_expr->lookup_annote_by_name("reaching_defs")); 

	Iter<SuifBrick*> iter = reaching_defs->get_brick_iterator();
	if(!iter.is_valid()) 
	   return Walker::Continue;

	SuifObjectBrick *sob = to<SuifObjectBrick>(iter.current());
	if(!is_a<StoreVariableStatement>(sob->get_object()))
	   return Walker::Continue;

        StoreVariableStatement *def = to<StoreVariableStatement>(sob->get_object());
	if(!is_a<LoadVariableExpression>(def->get_value()))
	   return Walker::Continue;

	VariableSymbol *replacement_sym = (to<LoadVariableExpression>(def->get_value()))->get_source();
	BrickAnnote *def_in_stmts = to<BrickAnnote>(def->lookup_annote_by_name("in_stmts")); 

	if(!annotes_equal_for_sym(in_stmts, def_in_stmts, def, replacement_sym))
	   return Walker::Continue;

	for(iter.next(); iter.is_valid(); iter.next()){

	    sob = to<SuifObjectBrick>(iter.current());
  	    if(!is_a<StoreVariableStatement>(sob->get_object()))
	       return Walker::Continue;

            def = to<StoreVariableStatement>(sob->get_object());
 	    if(!is_a<LoadVariableExpression>(def->get_value()))
	       return Walker::Continue;

	    if(replacement_sym != (to<LoadVariableExpression>(def->get_value()))->get_source())
	       return Walker::Continue;
	
	    def_in_stmts = to<BrickAnnote>(def->lookup_annote_by_name("in_stmts")); 
	    if(!annotes_equal_for_sym(in_stmts, def_in_stmts, def, replacement_sym))
	       return Walker::Continue;

	}

	list<LoadVariableExpression*>* to_be_removed_store_var_values = new list<LoadVariableExpression*>;
        for(Iter<SuifBrick*> iter = reaching_defs->get_brick_iterator();
            iter.is_valid();iter.next()){

            sob = to<SuifObjectBrick>(iter.current());

            StoreVariableStatement *store_var_stmt = to<StoreVariableStatement>(sob->get_object());
            BrickAnnote *reached_uses = to<BrickAnnote>(store_var_stmt->lookup_annote_by_name("reached_uses"));

            remove_brick(reached_uses, load_var_expr);
            if(reached_uses->get_brick_count() == 0){
	       to_be_removed->push_back(store_var_stmt);
	       to_be_removed_store_var_values->push_back(to<LoadVariableExpression>(store_var_stmt->get_value()));
	    }
        }

        load_var_expr->set_source(replacement_sym);
        empty(reaching_defs);

        sob = to<SuifObjectBrick>(in_stmts->get_brick(0));
        BitVector *bv = (BitVector*)(sob->get_object());
        BitVectorMap *bv_map = bv->get_bit_vector_map();
        list<int>* kill_set_list = bv_map->get_kill_set(replacement_sym);

        if(!kill_set_list)
           return Walker::Continue;

        for(list<int>::iterator iter = kill_set_list->begin();
            iter != kill_set_list->end(); iter++){

            if(bv->is_marked(*iter)){

               SuifObject *so = bv_map->reverse_lookup(*iter);
               reaching_defs->append_brick(create_suif_object_brick(env, so));

               if(is_a<StoreVariableStatement>(so)){

                  StoreVariableStatement *store_var_stmt = to<StoreVariableStatement>(so);
                  BrickAnnote *reached_uses = to<BrickAnnote>(store_var_stmt->lookup_annote_by_name("reached_uses"));
	 	  for(list<LoadVariableExpression*>::iterator iter2 = to_be_removed_store_var_values->begin();
		      iter2 != to_be_removed_store_var_values->end(); iter2++)
		      remove_brick(reached_uses, *iter2);
                  reached_uses->append_brick(create_suif_object_brick(env, load_var_expr));

               }else if(is_a<CallStatement>(so)){

                  CallStatement *call_stmt = to<CallStatement>(so);
                  BrickAnnote *reached_uses = to<BrickAnnote>(call_stmt->lookup_annote_by_name("reached_uses"));
	 	  for(list<LoadVariableExpression*>::iterator iter2 = to_be_removed_store_var_values->begin();
		      iter2 != to_be_removed_store_var_values->end(); iter2++)
		      remove_brick(reached_uses, *iter2);
                  reached_uses->append_brick(create_suif_object_brick(env, load_var_expr));

               }else if(is_a<VariableSymbol>(so)){

                  VariableSymbol *var_sym = to<VariableSymbol>(so);
                  BrickAnnote *reached_uses = to<BrickAnnote>(var_sym->lookup_annote_by_name("reached_uses"));
	 	  for(list<LoadVariableExpression*>::iterator iter2 = to_be_removed_store_var_values->begin();
		      iter2 != to_be_removed_store_var_values->end(); iter2++)
		      remove_brick(reached_uses, *iter2);
                  reached_uses->append_brick(create_suif_object_brick(env, load_var_expr));
               }
            }
        }

	delete to_be_removed_store_var_values;
    	return Walker::Continue;
}

