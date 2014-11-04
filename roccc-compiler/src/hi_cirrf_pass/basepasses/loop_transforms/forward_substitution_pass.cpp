// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#include "common/system_specific.h"
#include <common/suif_copyright.h>

#include <iostream>
#include <math.h>
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
#include <utils/semantic_helper.h>
#include <cfenodes/cfe.h>
#include <cfenodes/cfe_factory.h>
#include "roccc_utils/print_utils.h"
#include "roccc_utils/list_utils.h"
#include "roccc_utils/loop_utils.h"
#include "roccc_utils/warning_utils.h"
#include "forward_substitution_pass.h"

using namespace std;

/**************************** Declarations ************************************/

class fs_c_for_statement_walker: public SelectiveWalker {
public:
  fs_c_for_statement_walker(SuifEnv *the_env)
    : SelectiveWalker(the_env, CForStatement::get_class_name()) {}
  Walker::ApplyStatus operator () (SuifObject *x);
};

/**************************** Implementations ************************************/
ForwardSubstitutionPass::ForwardSubstitutionPass(SuifEnv *pEnv) : PipelinablePass(pEnv, "ForwardSubstitutionPass") {}

void ForwardSubstitutionPass::do_procedure_definition(ProcedureDefinition* proc_def){
  OutputInformation("Forward Substitution pass begins") ;
    if (proc_def){
        fs_c_for_statement_walker walker(get_suif_env());
        proc_def->walk(walker);
    }
    OutputInformation("Forward Substitution pass ends") ;
}
   
Walker::ApplyStatus fs_c_for_statement_walker::operator () (SuifObject *x) {
    CForStatement *the_c_for = to<CForStatement>(x);  
    Statement *body = to<Statement>(the_c_for->get_body());

    if (body) {
	list<VariableSymbol*>* the_c_for_induction_variables = get_c_for_induction_variables(the_c_for);
	list<VariableSymbol*>* the_c_for_invariants = get_loop_invariants(body, the_c_for_induction_variables);

	VariableSymbol* target_symbol = NULL;
	Expression* substitute_expr = NULL;

       	list<StoreVariableStatement*>* store_var_stmts = collect_objects<StoreVariableStatement>(body);
       	for (list<StoreVariableStatement*>::iterator iter = store_var_stmts->begin();
             iter != store_var_stmts->end(); iter++) {
	     target_symbol = (*iter)->get_destination();
	     substitute_expr = (*iter)->get_value();
	
	     bool found = true;
	     for(SemanticHelper::SrcVarIter src_iter(substitute_expr);
                 src_iter.is_valid(); src_iter.next()) {
	         if(!is_in_list(src_iter.current(), the_c_for_induction_variables) &&
                    !is_in_list(src_iter.current(), the_c_for_invariants)) {
	            found = false;
                    break;
	         }
	     }
	     
	     if(found){
	        BrickAnnote* reached_uses = to<BrickAnnote>((*iter)->lookup_annote_by_name("reached_uses"));
                for(Iter<SuifBrick*> annote_iter = reached_uses->get_brick_iterator();
                    annote_iter.is_valid(); annote_iter.next()){
                    SuifObjectBrick *sob = to<SuifObjectBrick>(annote_iter.current());
                    LoadVariableExpression *target_symbol_use = to<LoadVariableExpression>(sob->get_object());

	            BrickAnnote* reaching_defs = to<BrickAnnote>((target_symbol_use)->lookup_annote_by_name("reaching_defs"));
		    if(reaching_defs->get_brick_count() != 1)
		       continue;

	            if(target_symbol == target_symbol_use->get_source()){ 
		       Expression *replacement_expr = to<Expression>(deep_suif_clone(substitute_expr));
		       target_symbol_use->get_parent()->replace(target_symbol_use, replacement_expr);
		    }else
		    { 
		      OutputWarning("ERROR in Forward substitution pass!") ;
		    }
		}	 	
	     }
	  	
	}
    }

    return Walker::Continue;
}


