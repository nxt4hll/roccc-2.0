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
#include <typebuilder/type_builder.h>
#include <suifkernel/suifkernel_messages.h>
#include <suifkernel/utilities.h> 
#include <suifkernel/group_walker.h> 
#include <cfenodes/cfe.h>
#include "utils/expression_utils.h"
#include "utils/cloning_utils.h"
#include "roccc_utils/IR_utils.h"
#include "roccc_utils/print_utils.h"
#include "roccc_utils/warning_utils.h"
#include "switch_case_to_if_stmts_pass.h"

using namespace std;

/**************************** Declarations ************************************/

class sctis_multi_way_branch_statement_walker: public SelectiveWalker {
public:
  sctis_multi_way_branch_statement_walker(SuifEnv *env)
    :SelectiveWalker(env, MultiWayBranchStatement::get_class_name()) {}
  Walker::ApplyStatus operator () (SuifObject *x); 
};

list<CodeLabelSymbol*>* code_label_sym_list;
list<Statement*>* to_be_removed;

/**************************** Implementations ************************************/
SwitchCaseToIfStmtsPass::SwitchCaseToIfStmtsPass(SuifEnv *pEnv) : PipelinablePass(pEnv, "SwitchCaseToIfStmtsPass") {}

void SwitchCaseToIfStmtsPass::do_procedure_definition(ProcedureDefinition* proc_def){
  OutputInformation("Case to if statement pass begins");
  if (proc_def)
  {
    sctis_multi_way_branch_statement_walker walker(get_suif_env());
    proc_def->walk(walker);
  }
  OutputInformation("Case to if statement pass ends") ;
}

StatementList* get_statement_block_btw_labels(ProcedureDefinition *proc_def, CodeLabelSymbol *start_label_sym, 
									     CodeLabelSymbol *end_label_sym){

	StatementList *stmt_block = create_statement_list(proc_def->get_suif_env());  

	for(Iter<LabelLocationStatement> iter = object_iterator<LabelLocationStatement>(proc_def->get_body());
	    iter.is_valid(); iter.next()){
	    
 	    LabelLocationStatement *start_label_loc_stmt = &iter.current();

	    if(start_label_loc_stmt->get_defined_label() == start_label_sym){

	       Statement *next_stmt = get_next_statement(start_label_loc_stmt);
	       while(next_stmt != NULL){
	          if(is_a<LabelLocationStatement>(next_stmt)){
		     if((to<LabelLocationStatement>(next_stmt))->get_defined_label() == end_label_sym)
		        return stmt_block;
	          }else if(is_a<JumpStatement>(next_stmt)){
		     if((to<JumpStatement>(next_stmt))->get_target() == end_label_sym)
		        return stmt_block;
                  }
		  stmt_block->append_statement(to<Statement>(deep_suif_clone(next_stmt)));
		  next_stmt = get_next_statement(next_stmt);
 	       }
	       delete stmt_block;
	       return NULL;
	    }
	}
	delete stmt_block;
	return NULL;
}

Walker::ApplyStatus sctis_multi_way_branch_statement_walker::operator () (SuifObject *x) {
	SuifEnv *env = get_env();
	MultiWayBranchStatement *multi_way_stmt = to<MultiWayBranchStatement>(x);
	ProcedureDefinition *proc_def = get_procedure_definition(multi_way_stmt);
	TypeBuilder *tb = get_type_builder(env);

        BrickAnnote *end_label_annote = to<BrickAnnote>(multi_way_stmt->lookup_annote_by_name("end_label"));
        SuifObjectBrick *sob = to<SuifObjectBrick>(end_label_annote->get_brick(0));
        CodeLabelSymbol *end_label_sym = to<CodeLabelSymbol>(sob->get_object());
        
	Expression *decision_expr = multi_way_stmt->get_decision_operand();

	CodeLabelSymbol *default_label_sym = multi_way_stmt->get_default_target();

	Statement *replacement = get_statement_block_btw_labels(proc_def, default_label_sym, end_label_sym);

	cout << "ADDED " << print_statement(replacement);

	for(Iter<indexed_list<IInteger, CodeLabelSymbol*>::pair> iter = multi_way_stmt->get_case_iterator();
	    iter.is_valid(); iter.next()){

	    indexed_list<IInteger, CodeLabelSymbol*>::pair case_pair = iter.current();	    	    
	
	    Expression *condition_expr = create_binary_expression(env,
							tb->get_boolean_type(),
							String("is_equal_to"),
							to<Expression>(deep_suif_clone(decision_expr)),
							create_int_constant(env, 
								decision_expr->get_result_type(),
								case_pair.first));

	    Statement *then_stmt = get_statement_block_btw_labels(proc_def, case_pair.second, end_label_sym);
	    print_statement(then_stmt);

	    replacement = create_if_statement(env, condition_expr, then_stmt, replacement);
	}

	Statement *next_stmt = get_next_statement(multi_way_stmt);
	bool is_removal_complete = 0;
	while(!is_removal_complete && next_stmt != NULL){
	   if(is_a<LabelLocationStatement>(next_stmt)){
	      if((to<LabelLocationStatement>(next_stmt))->get_defined_label() == end_label_sym)
		 is_removal_complete = 1;
           }
	   remove_statement(next_stmt);
	   next_stmt = get_next_statement(multi_way_stmt);
        }

	multi_way_stmt->get_parent()->replace(multi_way_stmt, replacement);

	set_address(replacement);
        return Walker::Replaced;
}

