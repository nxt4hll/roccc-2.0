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
#include "roccc_utils/IR_utils.h"
#include "roccc_utils/list_utils.h"
#include "roccc_utils/symbol_utils.h"
#include "roccc_utils/annote_utils.h"
#include "roccc_utils/control_flow_utils.h"
#include "roccc_utils/bit_vector_data_flow_utils.h"
#include "roccc_utils/warning_utils.h"
#include "code_hoisting_pass.h"

// Run FlattenScopeStatementsPass prior to running this pass
// Run ScalarRenamingPass prior to running this pass
// Run InvariantCodeMotionPass prior to running this pass

// FIX ME: ADD ARRAY REF EXPRESSION PROCESSING such as. COLLECT DEFINED AND USED ARRAY NAMES AT EVERY *ITER

/**************************** Declarations ************************************/

void form_worklist(Statement *s);
void process_worklist();

list<IfStatement*>* worklist;

/**************************** Implementations ************************************/
CodeHoistingPass::CodeHoistingPass(SuifEnv *pEnv) : PipelinablePass(pEnv, "CodeHoistingPass") {}

void CodeHoistingPass::do_procedure_definition(ProcedureDefinition* proc_def)
{
  OutputInformation("Code hoisting pass begins") ;
  if (proc_def)
  {
    worklist = new list<IfStatement*>;
    form_worklist(to<Statement>(proc_def->get_body()));
    process_worklist();
  }
  OutputInformation("Code hoisting pass ends") ;
}

void form_worklist(Statement *s){

    if(!s) return;

    if(is_a<IfStatement>(s)){
       form_worklist((to<IfStatement>(s))->get_then_part());
       form_worklist((to<IfStatement>(s))->get_else_part());
       worklist->push_back(to<IfStatement>(s));
    }else if(is_a<CForStatement>(s))
       form_worklist((to<CForStatement>(s))->get_body());	
    else if(is_a<WhileStatement>(s))
       form_worklist((to<WhileStatement>(s))->get_body());	
    else if(is_a<ScopeStatement>(s))
       form_worklist((to<ScopeStatement>(s))->get_body());	
    else if(is_a<StatementList>(s))
       for(Iter<Statement*> iter = (to<StatementList>(s))->get_statement_iterator();
	   iter.is_valid(); iter.next())
	   form_worklist(iter.current());
    
}

list<Statement*>* form_child_stmt_list(Statement *s, list<Statement*>* result_list = 0){

    if(!result_list)
       result_list = new list<Statement*>;

    if(!s) 
       return result_list;

    if(is_a<StatementList>(s))
       for(Iter<Statement*> iter = (to<StatementList>(s))->get_statement_iterator();
	   iter.is_valid(); iter.next())
	   form_child_stmt_list(iter.current(), result_list);
    else result_list->push_back(s);

    return result_list;

}

void process_worklist(){

    while(!worklist->empty()){
	IfStatement *if_stmt = worklist->front();
	list<VariableSymbol*>* used_vars_in_if_stmt_condition_expr = collect_used_symbols(if_stmt->get_condition());

	list<Statement*>* then_stmt_list = form_child_stmt_list(if_stmt->get_then_part());
	list<Statement*>* else_stmt_list = form_child_stmt_list(if_stmt->get_else_part());

	list<VariableSymbol*>* modified_vars_in_then_stmt = new list<VariableSymbol*>;
	list<VariableSymbol*>* used_vars_in_then_stmt = new list<VariableSymbol*>;

	for(list<Statement*>::iterator iter = then_stmt_list->begin();
	    iter != then_stmt_list->end(); iter++){

            Iter<ArrayReferenceExpression> array_ref_iter = object_iterator<ArrayReferenceExpression>(*iter);
            if(array_ref_iter.is_valid()){
   	       push_back_items_into_list(collect_used_symbols(*iter), used_vars_in_then_stmt);
	       push_back_items_into_list(collect_defined_symbols(*iter), modified_vars_in_then_stmt);
	       continue;
	    }

	    if(is_items_in_list(collect_used_symbols(*iter), modified_vars_in_then_stmt)){
   	       push_back_items_into_list(collect_used_symbols(*iter), used_vars_in_then_stmt);
	       push_back_items_into_list(collect_defined_symbols(*iter), modified_vars_in_then_stmt);
	       continue;
	    }

	    if(is_items_in_list(collect_defined_symbols(*iter), used_vars_in_then_stmt)){
   	       push_back_items_into_list(collect_used_symbols(*iter), used_vars_in_then_stmt);
	       push_back_items_into_list(collect_defined_symbols(*iter), modified_vars_in_then_stmt);
	       continue;
            }

	    if(is_items_in_list(collect_defined_symbols(*iter), modified_vars_in_then_stmt)){
   	       push_back_items_into_list(collect_used_symbols(*iter), used_vars_in_then_stmt);
	       push_back_items_into_list(collect_defined_symbols(*iter), modified_vars_in_then_stmt);
	       continue;
            }

	    if(is_items_in_list(collect_defined_symbols(*iter), used_vars_in_if_stmt_condition_expr)){
   	       push_back_items_into_list(collect_used_symbols(*iter), used_vars_in_then_stmt);
	       push_back_items_into_list(collect_defined_symbols(*iter), modified_vars_in_then_stmt);
	       continue;
	    }
	    
  	    list<VariableSymbol*>* modified_vars_in_else_stmt = new list<VariableSymbol*>;
	    list<VariableSymbol*>* used_vars_in_else_stmt = new list<VariableSymbol*>;

	    for(list<Statement*>::iterator iter2 = else_stmt_list->begin();
	        iter2 != else_stmt_list->end(); iter2++){
 
                Iter<ArrayReferenceExpression> array_ref_iter = object_iterator<ArrayReferenceExpression>(*iter2);
                if(array_ref_iter.is_valid()){
   	           push_back_items_into_list(collect_used_symbols(*iter2), used_vars_in_else_stmt);
	           push_back_items_into_list(collect_defined_symbols(*iter2), modified_vars_in_else_stmt);
	           continue;
	        }

   	        if(is_items_in_list(collect_used_symbols(*iter2), modified_vars_in_else_stmt)){
   	           push_back_items_into_list(collect_used_symbols(*iter2), used_vars_in_else_stmt);
	           push_back_items_into_list(collect_defined_symbols(*iter2), modified_vars_in_else_stmt);
	           continue;
		}
	    
	        if(is_items_in_list(collect_defined_symbols(*iter2), used_vars_in_else_stmt)){
   	           push_back_items_into_list(collect_used_symbols(*iter2), used_vars_in_else_stmt);
	           push_back_items_into_list(collect_defined_symbols(*iter2), modified_vars_in_else_stmt);
	           continue;
		}
	    
	        if(is_items_in_list(collect_defined_symbols(*iter2), modified_vars_in_else_stmt)){
   	           push_back_items_into_list(collect_used_symbols(*iter2), used_vars_in_else_stmt);
	           push_back_items_into_list(collect_defined_symbols(*iter2), modified_vars_in_else_stmt);
	           continue;
		}
	    
  	        if(is_items_in_list(collect_defined_symbols(*iter2), used_vars_in_if_stmt_condition_expr)){
   	           push_back_items_into_list(collect_used_symbols(*iter2), used_vars_in_else_stmt);
	           push_back_items_into_list(collect_defined_symbols(*iter2), modified_vars_in_else_stmt);
	           continue;
		}

	        if(is_equal(*iter, *iter2)){
		
		   Statement *replacement = to<Statement>(deep_suif_clone(*iter));
		   insert_statement_before(if_stmt, replacement);

		   remove_statement(*iter);
		   remove_statement(*iter2);	   

		   remove_from_list(*iter2, else_stmt_list);
		   break;
		}

   	        push_back_items_into_list(collect_used_symbols(*iter2), used_vars_in_else_stmt);
	        push_back_items_into_list(collect_defined_symbols(*iter2), modified_vars_in_else_stmt);
	    }

   	    push_back_items_into_list(collect_used_symbols(*iter), used_vars_in_then_stmt);
	    push_back_items_into_list(collect_defined_symbols(*iter), modified_vars_in_then_stmt);

	}

	worklist->pop_front();
    }

}
