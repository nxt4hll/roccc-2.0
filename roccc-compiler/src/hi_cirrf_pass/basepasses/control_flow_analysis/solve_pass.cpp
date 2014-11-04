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
#include "roccc_utils/annote_utils.h"
#include "roccc_utils/warning_utils.h"
#include "solve_pass.h"

using namespace std;

// THIS PASS ONLY BUILDS THE CONTROL FLOW INFORMATION.
// JUMP INDIRECT IS NOT SUPPORTED
// FOR STATEMENT IS NOT SUPPORTED
// MULTI WAY BRANCH (SWITCH/CASE) IS NOT SUPPORTED

// DO WHILE STATEMENTS ARE RE-WRITTEN AS WHILE LOOPS IN THE PREPROCESSING STAGE

/**************************** Declarations ************************************/

class annote_statements: public SelectiveWalker {
public:
  annote_statements(SuifEnv *env)
    :SelectiveWalker(env, Statement::get_class_name()) {}
  Walker::ApplyStatus operator () (SuifObject *x); 
};

Statement *proc_def_body;
list<LabelLocationStatement*> *label_stmt_list;

void solve_statement(Statement *s, Statement *p);
void solve_eval_stmt(Statement *s, Statement *p);
void solve_if_stmt(Statement *s, Statement *p);
void solve_while_stmt(Statement *s, Statement *p);
void solve_c_for_stmt(Statement *s, Statement *p);
void solve_scope_stmt(Statement *s, Statement *p);
void solve_store_stmt(Statement *s, Statement *p);
void solve_store_variable_stmt(Statement *s, Statement *p);
void solve_call_stmt(Statement *s, Statement *p);
void solve_jump_stmt(Statement *s, Statement *p);
void solve_branch_stmt(Statement *s, Statement *p);
void solve_statement_list_stmt(Statement *s, Statement *p);
void solve_mark_stmt(Statement *s, Statement *p);
void solve_return_stmt(Statement *s, Statement *p);
void solve_label_location_stmt(Statement *s, Statement *p);

/**************************** Implementations ************************************/
ControlFlowSolvePass::ControlFlowSolvePass(SuifEnv *pEnv) : PipelinablePass(pEnv, "ControlFlowSolvePass") {}

void ControlFlowSolvePass::do_procedure_definition(ProcedureDefinition* proc_def){

  OutputInformation("Control flow pass begins") ;

  if (proc_def)
  {
    annote_statements walker(get_suif_env());
    proc_def->walk(walker);

    proc_def_body = to<Statement>(proc_def->get_body());
    label_stmt_list = collect_objects<LabelLocationStatement>(proc_def_body);

    solve_statement(proc_def_body, NULL);
    
    delete label_stmt_list;
  }

  OutputInformation("Control flow pass ends") ;
}

Walker::ApplyStatus annote_statements::operator () (SuifObject *x) {
	SuifEnv *env = get_env();
	Statement *stmt = to<Statement>(x);

	BrickAnnote *ba;

	if(stmt->lookup_annote_by_name("predecessors"))
	   delete (stmt->remove_annote_by_name("predecessors"));
	if(stmt->lookup_annote_by_name("successors"))
	   delete (stmt->remove_annote_by_name("successors"));
   
	ba = create_brick_annote(env, "predecessors");	// useful for building control flow information
	stmt->append_annote(ba);
	ba = create_brick_annote(env, "successors");
	stmt->append_annote(ba);

    	return Walker::Continue;
}

void solve_statement(Statement *s, Statement *p)
{
  // Adjusted because sometimes we put NULL statements in 
  if (s == NULL)
  {
    
    return ;
  }

	if (is_a<EvalStatement>(s))
	    solve_eval_stmt(s, p);
	else if (is_a<IfStatement>(s))
            solve_if_stmt(s, p);
	else if (is_a<WhileStatement>(s))
	    solve_while_stmt(s, p);
  	else if (is_a<CForStatement>(s))
	    solve_c_for_stmt(s, p);
	else if (is_a<ScopeStatement>(s))
	    solve_scope_stmt(s, p);
	else if (is_a<StoreStatement>(s))
	    solve_store_stmt(s, p);
	else if (is_a<StoreVariableStatement>(s))
	    solve_store_variable_stmt(s, p);
	else if (is_a<CallStatement>(s))
	    solve_call_stmt(s, p);
	else if (is_a<JumpStatement>(s))
	    solve_jump_stmt(s, p);
	else if (is_a<BranchStatement>(s))
	    solve_branch_stmt(s, p);
	else if (is_a<StatementList>(s))
	    solve_statement_list_stmt(s, p);
	else if (is_a<MarkStatement>(s))
	    solve_mark_stmt(s, p);
	else if (is_a<ReturnStatement>(s))
	    solve_return_stmt(s, p);
	else if (is_a<LabelLocationStatement>(s))
	    solve_label_location_stmt(s, p);
	else cout << "ERROR" << String(s->getClassName())  << endl;

}


void solve_eval_stmt(Statement *s, Statement *p){
	SuifEnv *env = s->get_suif_env();

	BrickAnnote *predecessors = to<BrickAnnote>(s->lookup_annote_by_name("predecessors"));
	bool found;
	if(p){
	   found = search(predecessors, p);
	   if(!found)
	      predecessors->append_brick(create_suif_object_brick(env, p));
	}
}

void solve_if_stmt(Statement *s, Statement *p){
	SuifEnv *env = s->get_suif_env();
	IfStatement *if_stmt = to<IfStatement>(s);

        BrickAnnote *predecessors = to<BrickAnnote>(s->lookup_annote_by_name("predecessors"));
	bool found;
	if(p){ 
	   found = search(predecessors, p);
	   if(!found)
	      predecessors->append_brick(create_suif_object_brick(env, p));
        }

	Statement *then_stmt = if_stmt->get_then_part();  
	solve_statement(then_stmt, p);	

	Statement *else_stmt = if_stmt->get_else_part();  
	if(else_stmt)
	   solve_statement(else_stmt, p);
}

void solve_while_stmt(Statement *s, Statement *p){
	SuifEnv *env = s->get_suif_env();
	WhileStatement *while_stmt = to<WhileStatement>(s);

  	BrickAnnote *predecessors = to<BrickAnnote>(s->lookup_annote_by_name("predecessors"));
	bool found;
	if(p){
	   found = search(predecessors, p);
	   if(!found)
	      predecessors->append_brick(create_suif_object_brick(env, p));
    	}

	Statement *body = while_stmt->get_body();  
	solve_statement(body, p);
	solve_statement(body, body);

	found = search(predecessors, body);
	if(!found)
	   predecessors->append_brick(create_suif_object_brick(env, body));
}

void solve_c_for_stmt(Statement *s, Statement *p){
	SuifEnv *env = s->get_suif_env();
	CForStatement *c_for_stmt = to<CForStatement>(s);

	Statement *before = c_for_stmt->get_before();  
	Statement *body = c_for_stmt->get_body();  
	Statement *step = c_for_stmt->get_step();  

	solve_statement(before, p);
	solve_statement(body, before);	
	solve_statement(step, body);	
	solve_statement(body, step);	

	BrickAnnote *predecessors = to<BrickAnnote>(s->lookup_annote_by_name("predecessors"));
	bool found = search(predecessors, before);
	if(!found)
	   predecessors->append_brick(create_suif_object_brick(env, before));
	found = search(predecessors, step);
	if(!found)
	   predecessors->append_brick(create_suif_object_brick(env, step));
}

void solve_scope_stmt(Statement *s, Statement *p){
	SuifEnv *env = s->get_suif_env();
	ScopeStatement *scope_stmt = to<ScopeStatement>(s);

	BrickAnnote *predecessors = to<BrickAnnote>(s->lookup_annote_by_name("predecessors"));
	bool found;
	if(p){
	   found = search(predecessors, p);
	   if(!found)
	      predecessors->append_brick(create_suif_object_brick(env, p));
	}

	Statement *body = scope_stmt->get_body();  
	solve_statement(body, p);		
}

void solve_store_stmt(Statement *s, Statement *p){
	SuifEnv *env = s->get_suif_env();

	BrickAnnote *predecessors = to<BrickAnnote>(s->lookup_annote_by_name("predecessors"));
	bool found;
	if(p){
	   found = search(predecessors, p);
	   if(!found)
	      predecessors->append_brick(create_suif_object_brick(env, p));
	}
}	

void solve_store_variable_stmt(Statement *s, Statement *p){
	SuifEnv *env = s->get_suif_env();

 	BrickAnnote *predecessors = to<BrickAnnote>(s->lookup_annote_by_name("predecessors"));
	bool found;
	if(p){
	   found = search(predecessors, p);
	   if(!found)
	      predecessors->append_brick(create_suif_object_brick(env, p));
	}
}

void solve_call_stmt(Statement *s, Statement *p){
	SuifEnv *env = s->get_suif_env();

 	BrickAnnote *predecessors = to<BrickAnnote>(s->lookup_annote_by_name("predecessors"));
	bool found;
	if(p){
	   found = search(predecessors, p);
	   if(!found)
	      predecessors->append_brick(create_suif_object_brick(env, p));
	}
}

void solve_jump_stmt(Statement *s, Statement *p){
	SuifEnv *env = s->get_suif_env();
	JumpStatement *jump_stmt = to<JumpStatement>(s);

	BrickAnnote *predecessors = to<BrickAnnote>(s->lookup_annote_by_name("predecessors"));
	bool found;
	if(p){
	   found = search(predecessors, p);
	   if(!found)
	      predecessors->append_brick(create_suif_object_brick(env, p));
	}

	CodeLabelSymbol *target = jump_stmt->get_target();

	for (list<LabelLocationStatement*>::iterator iter = label_stmt_list->begin();
             iter != label_stmt_list->end(); iter++){
	     
	     LabelLocationStatement *label_stmt = *iter; 
	     
	     if(target == label_stmt->get_defined_label())
		solve_label_location_stmt(label_stmt, s);
	}
}

void solve_branch_stmt(Statement *s, Statement *p){
	SuifEnv *env = s->get_suif_env();
	BranchStatement *branch_stmt = to<BranchStatement>(s);

	BrickAnnote *predecessors = to<BrickAnnote>(s->lookup_annote_by_name("predecessors"));
	bool found;
	if(p){
	   found = search(predecessors, p);
	   if(!found)
	      predecessors->append_brick(create_suif_object_brick(env, p));
	}

	CodeLabelSymbol *target = branch_stmt->get_target();

	for (list<LabelLocationStatement*>::iterator iter = label_stmt_list->begin();
             iter != label_stmt_list->end(); iter++){
	     
	     LabelLocationStatement *label_stmt = *iter; 
	     
	     if(target == label_stmt->get_defined_label())
		solve_label_location_stmt(label_stmt, s);
	}
}

void solve_statement_list_stmt(Statement *s, Statement *p){
	SuifEnv *env = s->get_suif_env();
     	StatementList *stmt_list = to<StatementList>(s);

	BrickAnnote *predecessors = to<BrickAnnote>(s->lookup_annote_by_name("predecessors"));
	bool found;
	if(p){
	   found = search(predecessors, p);
	   if(!found)
	      predecessors->append_brick(create_suif_object_brick(env, p));
        }

	Statement *pred = p;
     	for (Iter<Statement*> iter = stmt_list->get_child_statement_iterator();
	     iter.is_valid(); iter.next()){

	     Statement *stmt = iter.current(); 
	     solve_statement(stmt, pred);
             pred = stmt;
     	}	
}

void solve_mark_stmt(Statement *s, Statement *p){
	SuifEnv *env = s->get_suif_env();

	BrickAnnote *predecessors = to<BrickAnnote>(s->lookup_annote_by_name("predecessors"));
	bool found;
	if(p){
	   found = search(predecessors, p);
	   if(!found)
	      predecessors->append_brick(create_suif_object_brick(env, p));
	}
}

void solve_return_stmt(Statement *s, Statement *p){
	SuifEnv *env = s->get_suif_env();

	BrickAnnote *predecessors = to<BrickAnnote>(s->lookup_annote_by_name("predecessors"));
	bool found;
	if(p){
	   found = search(predecessors, p);
	   if(!found)
	      predecessors->append_brick(create_suif_object_brick(env, p));
	}
}	

void solve_label_location_stmt(Statement *s, Statement *p){
	SuifEnv *env = s->get_suif_env();

	BrickAnnote *predecessors = to<BrickAnnote>(s->lookup_annote_by_name("predecessors"));
	bool found;
	if(p){
	   found = search(predecessors, p);
	   if(!found)
	      predecessors->append_brick(create_suif_object_brick(env, p));
	}
}


