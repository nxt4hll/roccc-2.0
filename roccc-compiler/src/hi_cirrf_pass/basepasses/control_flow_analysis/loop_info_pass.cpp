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
#include "roccc_utils/annote_utils.h"
#include "loop_info_pass.h"

// THIS PASS ONLY BUILDS THE CONTROL FLOW INFORMATION RELATED TO LOOP NESTS.
// JUMP INDIRECT IS NOT SUPPORTED
// FOR STATEMENT IS NOT SUPPORTED
// MULTI WAY BRANCH (SWITCH/CASE) IS NOT SUPPORTED

// DO WHILE STATEMENTS ARE RE-WRITTEN AS WHILE LOOPS IN THE PREPROCESSING STAGE

// FIX ME = ADD WHILE/IF STATEMENT INTRODUCED CONTROL FLOW SUPPORT

/**************************** Declarations ************************************/

class annote_c_for_statements: public SelectiveWalker {
public:
  annote_c_for_statements(SuifEnv *env)
    :SelectiveWalker(env, CForStatement::get_class_name()) {}
  Walker::ApplyStatus operator () (SuifObject *x); 
};

int loop_level;

void solve_li_statement(Statement *s);
void solve_li_if_stmt(Statement *s);
void solve_li_while_stmt(Statement *s);
void solve_li_c_for_stmt(Statement *s);
void solve_li_scope_stmt(Statement *s);
void solve_li_statement_list_stmt(Statement *s);

/**************************** Implementations ************************************/
LoopInfoPass::LoopInfoPass(SuifEnv *pEnv) : PipelinablePass(pEnv, "LoopInfoPass") {}

void LoopInfoPass::do_procedure_definition(ProcedureDefinition* proc_def){
    if (proc_def){
	annote_c_for_statements walker(get_suif_env());
	proc_def->walk(walker);

	loop_level = 0;

	Statement *proc_def_body = to<Statement>(proc_def->get_body());
	solve_li_statement(proc_def_body);
    }
}

Walker::ApplyStatus annote_c_for_statements::operator () (SuifObject *x) {
	SuifEnv *env = get_env();
	CForStatement *c_for_stmt = to<CForStatement>(x);

	BrickAnnote *ba;

	if(c_for_stmt->lookup_annote_by_name("c_for_info"))
	{
	  delete c_for_stmt->remove_annote_by_name("c_for_info");
	}
   
	ba = create_brick_annote(env, "c_for_info");
	c_for_stmt->append_annote(ba);

	if(!(c_for_stmt->lookup_annote_by_name("c_for_label"))){
	   ba = create_brick_annote(env, "c_for_label");
	   c_for_stmt->append_annote(ba);
        }

    	return Walker::Continue;
}

void solve_li_statement(Statement *s){

	if (is_a<IfStatement>(s))
	    solve_li_if_stmt(s);
	else if (is_a<WhileStatement>(s))
	    solve_li_while_stmt(s);
  	else if (is_a<CForStatement>(s))
	    solve_li_c_for_stmt(s);
	else if (is_a<ScopeStatement>(s))
	    solve_li_scope_stmt(s);
	else if (is_a<StatementList>(s))
	    solve_li_statement_list_stmt(s);

}

void solve_li_if_stmt(Statement *s){
	IfStatement *if_stmt = to<IfStatement>(s);

	Statement *then_stmt = if_stmt->get_then_part();  
	solve_li_statement(then_stmt);	

	Statement *else_stmt = if_stmt->get_else_part();  
	if(else_stmt)
	   solve_li_statement(else_stmt);
}

void solve_li_while_stmt(Statement *s){
	WhileStatement *while_stmt = to<WhileStatement>(s);

	Statement *body = while_stmt->get_body();  
	solve_li_statement(body);

}

void solve_li_c_for_stmt(Statement *s){
	SuifEnv *env = s->get_suif_env();
	CForStatement *c_for_stmt = to<CForStatement>(s);

	BrickAnnote *nest_level = to<BrickAnnote>(s->lookup_annote_by_name("c_for_info"));
	if(nest_level->get_brick_count() == 0){
   	   nest_level->insert_brick(0, create_integer_brick(env, IInteger(++loop_level)));
	   BrickAnnote *loop_label = to<BrickAnnote>(s->lookup_annote_by_name("c_for_label"));
	   if(loop_label->get_brick_count() == 0){
	      loop_label->insert_brick(0, create_string_brick(env, String(loop_level)));
  	   }  
	}

	Statement *body = c_for_stmt->get_body();  
	solve_li_statement(body);	

	--loop_level;
}

void solve_li_scope_stmt(Statement *s){
	ScopeStatement *scope_stmt = to<ScopeStatement>(s);

	Statement *body = scope_stmt->get_body();  
	solve_li_statement(body);
}

void solve_li_statement_list_stmt(Statement *s){
     	StatementList *stmt_list = to<StatementList>(s);

     	for (Iter<Statement*> iter = stmt_list->get_child_statement_iterator();
	     iter.is_valid(); iter.next())
	     solve_li_statement(iter.current());

}

