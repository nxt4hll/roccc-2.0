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
#include "roccc_utils/control_flow_utils.h"
#include "roccc_utils/bit_vector_data_flow_utils.h"
#include "roccc_utils/warning_utils.h"
#include "solve_available_expressions_pass.h"

using namespace std;

/**************************** Declarations ************************************/

class annote_ae_statements: public SelectiveWalker {
public:
  annote_ae_statements(SuifEnv *env)
    :SelectiveWalker(env, Statement::get_class_name()) {}
  Walker::ApplyStatus operator () (SuifObject *x);
};

void solve_ae_statement(Statement *s, BrickAnnote *extra);
void solve_ae_eval_stmt(Statement *s, BrickAnnote *extra);
void solve_ae_if_stmt(Statement *s, BrickAnnote *extra);
void solve_ae_while_stmt(Statement *s, BrickAnnote *extra);
void solve_ae_c_for_stmt(Statement *s, BrickAnnote *extra);
void solve_ae_scope_stmt(Statement *s, BrickAnnote *extra);
void solve_ae_store_stmt(Statement *s, BrickAnnote *extra);
void solve_ae_store_variable_stmt(Statement *s, BrickAnnote *extra);
void solve_ae_call_stmt(Statement *s, BrickAnnote *extra);
void solve_ae_jump_stmt(Statement *s, BrickAnnote *extra);
void solve_ae_branch_stmt(Statement *s, BrickAnnote *extra);
void solve_ae_statement_list_stmt(Statement *s, BrickAnnote *extra);
void solve_ae_mark_stmt(Statement *s, BrickAnnote *extra);
void solve_ae_return_stmt(Statement *s, BrickAnnote *extra);
void solve_ae_label_location_stmt(Statement *s, BrickAnnote *extra);

/**************************** Implementations ************************************/
SolveAvailableExpressionsPass::SolveAvailableExpressionsPass(SuifEnv *pEnv) : 
					PipelinablePass(pEnv, "SolveAvailableExpressionsPass") {}

void SolveAvailableExpressionsPass::do_procedure_definition(ProcedureDefinition* proc_def)
{
  OutputInformation("Solve available expressions pass begins") ;
  if (proc_def)
  {
    annote_ae_statements walker(get_suif_env());
    proc_def->walk(walker);
    
    Statement *stmt = to<Statement>(proc_def->get_body());
    solve_ae_statement(stmt, NULL);
  }
  OutputInformation("Solve available expressions pass ends") ;
}
       
bool has_array_ref_expr_parent(BinaryExpression *bexpr){

        Statement *parent_stmt = get_expression_owner(bexpr);
        ExecutionObject *parent_exec_obj = to<ExecutionObject>(bexpr->get_parent());

        while(parent_exec_obj != parent_stmt)
           if(is_a<ArrayReferenceExpression>(parent_exec_obj))
              return 1;
           else parent_exec_obj = to<ExecutionObject>(parent_exec_obj->get_parent());
        
        return 0;
}

Walker::ApplyStatus annote_ae_statements::operator () (SuifObject *x) {
        SuifEnv *env = get_env();
        Statement *stmt = to<Statement>(x);
        
        if(stmt->lookup_annote_by_name("in_available_exprs"))
           delete (stmt->remove_annote_by_name("in_available_exprs"));

        if(stmt->lookup_annote_by_name("out_available_exprs"))
           delete (stmt->remove_annote_by_name("out_available_exprs"));

        BrickAnnote *ba = create_brick_annote(env, "in_available_exprs");
        stmt->append_annote(ba);
         
        ba = create_brick_annote(env, "out_available_exprs");
        stmt->append_annote(ba);

        return Walker::Continue;
}

void solve_ae_statement(Statement *s, BrickAnnote *extra){

	if(is_a<EvalStatement>(s))
           solve_ae_eval_stmt(s, extra);
        if(is_a<IfStatement>(s))
           solve_ae_if_stmt(s, extra);
        if(is_a<WhileStatement>(s))
           solve_ae_while_stmt(s, extra);
        if(is_a<CForStatement>(s))
           solve_ae_c_for_stmt(s, extra);
        if(is_a<ScopeStatement>(s))
           solve_ae_scope_stmt(s, extra);
        if(is_a<StoreStatement>(s))
           solve_ae_store_stmt(s, extra);
        if(is_a<StoreVariableStatement>(s))
           solve_ae_store_variable_stmt(s, extra);
        if(is_a<CallStatement>(s))
           solve_ae_call_stmt(s, extra);
        if(is_a<JumpStatement>(s))
           solve_ae_jump_stmt(s, extra);
        if(is_a<BranchStatement>(s))
           solve_ae_branch_stmt(s, extra);
        if(is_a<StatementList>(s))
           solve_ae_statement_list_stmt(s, extra);
        if(is_a<MarkStatement>(s))
           solve_ae_mark_stmt(s, extra);
        if(is_a<ReturnStatement>(s))
           solve_ae_return_stmt(s, extra);
        if(is_a<LabelLocationStatement>(s))
           solve_ae_label_location_stmt(s, extra);
}

void solve_ae_eval_stmt(Statement *s, BrickAnnote *extra){
        SuifEnv *env = s->get_suif_env();

        BrickAnnote *in = to<BrickAnnote>(s->lookup_annote_by_name("in_available_exprs"));
        empty(in);

        BrickAnnote *predecessors = to<BrickAnnote>(s->lookup_annote_by_name("predecessors"));
        intersect_predecessor_annotes(in, predecessors);

        if(extra)
           union_annotes(in, extra);

        BrickAnnote *out = to<BrickAnnote>(s->lookup_annote_by_name("out_available_exprs"));
        empty(out);

        list<BinaryExpression*>* expr_list = collect_objects<BinaryExpression>(s);
        for(list<BinaryExpression*>::iterator iter = expr_list->begin();
            iter != expr_list->end(); iter++)
            if(!has_array_ref_expr_parent(*iter))
               out->append_brick(create_suif_object_brick(env, *iter));
        delete expr_list;

        union_annotes(out, in);
}

void solve_ae_if_stmt(Statement *s, BrickAnnote *extra){
        SuifEnv *env = s->get_suif_env();
        IfStatement *if_stmt = to<IfStatement>(s);

        BrickAnnote *in = to<BrickAnnote>(s->lookup_annote_by_name("in_available_exprs"));
        empty(in);

        BrickAnnote *predecessors = to<BrickAnnote>(s->lookup_annote_by_name("predecessors"));
        intersect_predecessor_annotes(in, predecessors);

        if(extra)
           union_annotes(in, extra);

        BrickAnnote *ba = create_brick_annote(env, "extra");
        if(extra)
           union_annotes(ba, extra);
        list<BinaryExpression*>* expr_list = collect_objects<BinaryExpression>(if_stmt->get_condition());
        for(list<BinaryExpression*>::iterator iter = expr_list->begin();
            iter != expr_list->end(); iter++)
            if(!has_array_ref_expr_parent(*iter))
               ba->append_brick(create_suif_object_brick(env, *iter));
        delete expr_list;

        Statement *then_stmt = if_stmt->get_then_part();
        solve_ae_statement(then_stmt, ba);
        BrickAnnote *t_out = to<BrickAnnote>(then_stmt->lookup_annote_by_name("out_available_exprs"));

        BrickAnnote *e_out = NULL;
        Statement *else_stmt = if_stmt->get_else_part();
        if(else_stmt){
           solve_ae_statement(else_stmt, ba);
           e_out = to<BrickAnnote>(else_stmt->lookup_annote_by_name("out_available_exprs"));
        }

        BrickAnnote *out = to<BrickAnnote>(s->lookup_annote_by_name("out_available_exprs"));
        empty(out);
        union_annotes(out, t_out);
        if(e_out)
           intersect_annotes(out, e_out);
}

void solve_ae_while_stmt(Statement *s, BrickAnnote *extra){
        SuifEnv *env = s->get_suif_env();
        WhileStatement *while_stmt = to<WhileStatement>(s);

        BrickAnnote *in = to<BrickAnnote>(s->lookup_annote_by_name("in_available_exprs"));
        empty(in);

        BrickAnnote *predecessors = to<BrickAnnote>(s->lookup_annote_by_name("predecessors"));
        intersect_predecessor_annotes(in, predecessors);
        if(extra)
           union_annotes(in, extra);

        BrickAnnote *ba = create_brick_annote(env, "extra");
        if(extra)
           union_annotes(ba, extra);

        list<BinaryExpression*>* expr_list = collect_objects<BinaryExpression>(while_stmt->get_condition());
        for(list<BinaryExpression*>::iterator iter = expr_list->begin();
            iter != expr_list->end(); iter++)
            if(!has_array_ref_expr_parent(*iter))
               ba->append_brick(create_suif_object_brick(env, *iter));
        delete expr_list;

        Statement *body = while_stmt->get_body();
        solve_ae_statement(body, ba);
        solve_ae_statement(body, ba);
        BrickAnnote *b_out = to<BrickAnnote>(body->lookup_annote_by_name("out_available_exprs"));

        BrickAnnote *out = to<BrickAnnote>(s->lookup_annote_by_name("out_available_exprs"));
        empty(out);
        union_annotes(out, in);
        intersect_annotes(out, b_out);
}

void solve_ae_c_for_stmt(Statement *s, BrickAnnote *extra){
        SuifEnv *env = s->get_suif_env();
        CForStatement *c_for_stmt = to<CForStatement>(s);

        Statement *before = c_for_stmt->get_before();
        Statement *body = c_for_stmt->get_body();
        Statement *step = c_for_stmt->get_step();

        solve_ae_statement(before, extra);

        BrickAnnote *ba = create_brick_annote(env, "extra");
        list<BinaryExpression*>* expr_list = collect_objects<BinaryExpression>(c_for_stmt->get_test());
        for(list<BinaryExpression*>::iterator iter = expr_list->begin();
            iter != expr_list->end(); iter++)
            if(!has_array_ref_expr_parent(*iter))
               ba->append_brick(create_suif_object_brick(env, *iter));
        delete expr_list;

        solve_ae_statement(body, ba);
        solve_ae_statement(step, NULL);
        solve_ae_statement(body, ba);
        solve_ae_statement(step, NULL);

        BrickAnnote *in = to<BrickAnnote>(s->lookup_annote_by_name("in_available_exprs"));
        empty(in);
        BrickAnnote *predecessors = to<BrickAnnote>(s->lookup_annote_by_name("predecessors"));
        intersect_predecessor_annotes(in, predecessors);

        BrickAnnote *out = to<BrickAnnote>(s->lookup_annote_by_name("out_available_exprs"));
        empty(out);
        union_annotes(out, in);
        union_annotes(out, ba);
}

void solve_ae_scope_stmt(Statement *s, BrickAnnote *extra){
        ScopeStatement *scope_stmt = to<ScopeStatement>(s);

        BrickAnnote *in = to<BrickAnnote>(s->lookup_annote_by_name("in_available_exprs"));
        empty(in);

        BrickAnnote *predecessors = to<BrickAnnote>(s->lookup_annote_by_name("predecessors"));
        intersect_predecessor_annotes(in, predecessors);
        if(extra)
           union_annotes(in, extra);

        Statement *body = scope_stmt->get_body();
        solve_ae_statement(body, extra);
        BrickAnnote *b_out = to<BrickAnnote>(body->lookup_annote_by_name("out_available_exprs"));

        BrickAnnote *out = to<BrickAnnote>(s->lookup_annote_by_name("out_available_exprs"));
        empty(out);
        union_annotes(out, b_out);
}

void solve_ae_store_stmt(Statement *s, BrickAnnote *extra){
        SuifEnv *env = s->get_suif_env();
        StoreStatement *store_stmt = to<StoreStatement>(s);

        BrickAnnote *in = to<BrickAnnote>(s->lookup_annote_by_name("in_available_exprs"));
        empty(in);

        BrickAnnote *predecessors = to<BrickAnnote>(s->lookup_annote_by_name("predecessors"));
        intersect_predecessor_annotes(in, predecessors);
        if(extra)
           union_annotes(in, extra);

        BrickAnnote *out = to<BrickAnnote>(s->lookup_annote_by_name("out_available_exprs"));
        empty(out);
        union_annotes(out, in);

        list<BinaryExpression*>* expr_list = collect_objects<BinaryExpression>(store_stmt->get_value());
        for(list<BinaryExpression*>::iterator iter = expr_list->begin();
            iter != expr_list->end(); iter++)
            if(!has_array_ref_expr_parent(*iter))
               out->append_brick(create_suif_object_brick(env, *iter));
        delete expr_list;
}

void solve_ae_store_variable_stmt(Statement *s, BrickAnnote *extra){
        SuifEnv *env = s->get_suif_env();
        StoreVariableStatement *store_var_stmt = to<StoreVariableStatement>(s);

        BrickAnnote *in = to<BrickAnnote>(s->lookup_annote_by_name("in_available_exprs"));
        empty(in);

        BrickAnnote *predecessors = to<BrickAnnote>(s->lookup_annote_by_name("predecessors"));
        intersect_predecessor_annotes(in, predecessors);
        if(extra)
           union_annotes(in, extra);

        BrickAnnote *out = to<BrickAnnote>(s->lookup_annote_by_name("out_available_exprs"));
        empty(out);
        union_annotes(out, in);

        list<BinaryExpression*>* expr_list = collect_objects<BinaryExpression>(store_var_stmt->get_value());
        for(list<BinaryExpression*>::iterator iter = expr_list->begin();
            iter != expr_list->end(); iter++)
            if(!has_array_ref_expr_parent(*iter))
               out->append_brick(create_suif_object_brick(env, *iter));
        delete expr_list;

        kill_ae_annotes(out, store_var_stmt);
}

void solve_ae_call_stmt(Statement *s, BrickAnnote *extra){
        SuifEnv *env = s->get_suif_env();
        CallStatement *call_stmt = to<CallStatement>(s);

        BrickAnnote *in = to<BrickAnnote>(s->lookup_annote_by_name("in_available_exprs"));
        empty(in);

        BrickAnnote *predecessors = to<BrickAnnote>(s->lookup_annote_by_name("predecessors"));
        intersect_predecessor_annotes(in, predecessors);
        if(extra)
           union_annotes(in, extra);

        BrickAnnote *out = to<BrickAnnote>(s->lookup_annote_by_name("out_available_exprs"));
        empty(out);
        union_annotes(out, in);

        list<BinaryExpression*>* expr_list = collect_objects<BinaryExpression>(call_stmt);
        for(list<BinaryExpression*>::iterator iter = expr_list->begin();
            iter != expr_list->end(); iter++)
            if(!has_array_ref_expr_parent(*iter))
               out->append_brick(create_suif_object_brick(env, *iter));
        delete expr_list;

        kill_ae_annotes(out, call_stmt);
}

void solve_ae_jump_stmt(Statement *s, BrickAnnote *extra){

        BrickAnnote *in = to<BrickAnnote>(s->lookup_annote_by_name("in_available_exprs"));
        empty(in);

        BrickAnnote *predecessors = to<BrickAnnote>(s->lookup_annote_by_name("predecessors"));
        intersect_predecessor_annotes(in, predecessors);
        if(extra)
           union_annotes(in, extra);

        BrickAnnote *out = to<BrickAnnote>(s->lookup_annote_by_name("out_available_exprs"));
        empty(out);
        union_annotes(out, in);
}

void solve_ae_branch_stmt(Statement *s, BrickAnnote *extra){
        SuifEnv *env = s->get_suif_env();
        BranchStatement *branch_stmt = to<BranchStatement>(s);

        BrickAnnote *in = to<BrickAnnote>(s->lookup_annote_by_name("in_available_exprs"));
        empty(in);

        BrickAnnote *predecessors = to<BrickAnnote>(s->lookup_annote_by_name("predecessors"));
        intersect_predecessor_annotes(in, predecessors);
        if(extra)
           union_annotes(in, extra);

        BrickAnnote *out = to<BrickAnnote>(s->lookup_annote_by_name("out_available_exprs"));
        empty(out);
        union_annotes(out, in);

        list<BinaryExpression*>* expr_list = collect_objects<BinaryExpression>(branch_stmt->get_decision_operand());
        for(list<BinaryExpression*>::iterator iter = expr_list->begin();
            iter != expr_list->end(); iter++)
            if(!has_array_ref_expr_parent(*iter))
               out->append_brick(create_suif_object_brick(env, *iter));
        delete expr_list;
}

void solve_ae_statement_list_stmt(Statement *s, BrickAnnote *extra){
        StatementList *stmt_list = to<StatementList>(s);

        BrickAnnote *in = to<BrickAnnote>(s->lookup_annote_by_name("in_available_exprs"));
        empty(in);

        BrickAnnote *predecessors = to<BrickAnnote>(s->lookup_annote_by_name("predecessors"));
        intersect_predecessor_annotes(in, predecessors);
        if(extra)
           union_annotes(in, extra);

        Statement *pred = NULL;

        for(Iter<Statement*> iter = stmt_list->get_child_statement_iterator();
            iter.is_valid(); iter.next()){
            Statement *stmt = iter.current();
	    solve_ae_statement(stmt, extra);
            pred = stmt;
            extra = NULL;
        }

        BrickAnnote *out = to<BrickAnnote>(s->lookup_annote_by_name("out_available_exprs"));
        empty(out);

        if(pred){
           BrickAnnote *pred_out = to<BrickAnnote>(pred->lookup_annote_by_name("out_available_exprs"));
           union_annotes(out, pred_out);
        }else union_annotes(out, in);
}

void solve_ae_mark_stmt(Statement *s, BrickAnnote *extra){

        BrickAnnote *in = to<BrickAnnote>(s->lookup_annote_by_name("in_available_exprs"));
        empty(in);

        BrickAnnote *predecessors = to<BrickAnnote>(s->lookup_annote_by_name("predecessors"));
        intersect_predecessor_annotes(in, predecessors);
        if(extra)
           union_annotes(in, extra);

        BrickAnnote *out = to<BrickAnnote>(s->lookup_annote_by_name("out_available_exprs"));
        empty(out);
        union_annotes(out, in);
}

void solve_ae_return_stmt(Statement *s, BrickAnnote *extra){
        SuifEnv *env = s->get_suif_env();
        ReturnStatement *return_stmt = to<ReturnStatement>(s);

        BrickAnnote *in = to<BrickAnnote>(s->lookup_annote_by_name("in_available_exprs"));
        empty(in);

        BrickAnnote *predecessors = to<BrickAnnote>(s->lookup_annote_by_name("predecessors"));
        intersect_predecessor_annotes(in, predecessors);
        if(extra)
           union_annotes(in, extra);

        BrickAnnote *out = to<BrickAnnote>(s->lookup_annote_by_name("out_available_exprs"));
        empty(out);
        union_annotes(out, in);

        list<BinaryExpression*>* expr_list = collect_objects<BinaryExpression>(return_stmt->get_return_value());
        for(list<BinaryExpression*>::iterator iter = expr_list->begin();
            iter != expr_list->end(); iter++)
            if(!has_array_ref_expr_parent(*iter))
               out->append_brick(create_suif_object_brick(env, *iter));
        delete expr_list;
}

void solve_ae_label_location_stmt(Statement *s, BrickAnnote *extra){

        BrickAnnote *in = to<BrickAnnote>(s->lookup_annote_by_name("in_available_exprs"));
        empty(in);

        BrickAnnote *predecessors = to<BrickAnnote>(s->lookup_annote_by_name("predecessors"));
        intersect_predecessor_annotes(in, predecessors);
        if(extra)
           union_annotes(in, extra);

        BrickAnnote *out = to<BrickAnnote>(s->lookup_annote_by_name("out_available_exprs"));
        empty(out);
        union_annotes(out, in);
}



