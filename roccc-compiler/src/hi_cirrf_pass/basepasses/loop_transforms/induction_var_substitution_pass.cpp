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
#include "roccc_utils/list_utils.h"
#include "roccc_utils/annote_utils.h"
#include "roccc_utils/loop_utils.h"
#include "roccc_utils/symbol_utils.h"
#include "roccc_utils/IR_utils.h"
#include "roccc_utils/warning_utils.h"
#include "induction_var_substitution_pass.h"

// Loop peeling and loop normalization passes should be executed prior to running this pass.

/**************************** Declarations ************************************/

class ivs_c_for_statement_walker: public SelectiveWalker {
public:
  ivs_c_for_statement_walker(SuifEnv *the_env)
    : SelectiveWalker(the_env, CForStatement::get_class_name()) {}
  Walker::ApplyStatus operator () (SuifObject *x);
};

class ivs_array_reference_expression_walker: public SelectiveWalker {
public:
  ivs_array_reference_expression_walker(SuifEnv *the_env, StoreVariableStatement *aux_ivs) 
    : SelectiveWalker(the_env, ArrayReferenceExpression::get_class_name()), aux_induction_var_stmt(aux_ivs) {}
  Walker::ApplyStatus operator () (SuifObject *x);

private:
  StoreVariableStatement *aux_induction_var_stmt;
};

class ivs_load_variable_expression_walker: public SelectiveWalker {
public:
  ivs_load_variable_expression_walker(SuifEnv *the_env, StoreVariableStatement *aux_ivs)
    : SelectiveWalker(the_env, LoadVariableExpression::get_class_name()), aux_induction_var_stmt(aux_ivs) {}
  Walker::ApplyStatus operator () (SuifObject *x);

private:
  StoreVariableStatement *aux_induction_var_stmt;
};

void solve_ivs_statement(Statement *s);
void solve_ivs_store_variable_stmt(Statement *s);
bool is_a_qualified_load_var_expr(Expression *e);
bool is_a_qualified_constant_expr(Expression *e);
void find_aux_induction_variable_init_exprs();

void identify_aux_induction_variables();
void reduce_aux_induction_variables();

list<StoreVariableStatement*>* aux_induction_var_stmts;

list<VariableSymbol*>* induction_vars;
list<VariableSymbol*>* aux_induction_vars;
list<VariableSymbol*>* non_aux_induction_vars;
list<VariableSymbol*>* loop_invariants;

CForStatement *current_c_for_stmt = NULL;

/**************************** Implementations ************************************/
InductionVariableSubstitutionPass::InductionVariableSubstitutionPass(SuifEnv *pEnv) : 
					PipelinablePass(pEnv, "InductionVariableSubstitutionPass") {}

void InductionVariableSubstitutionPass::do_procedure_definition(ProcedureDefinition* proc_def){
  OutputInformation("Induction variable substitution pass begins") ;
    if (proc_def){
        ivs_c_for_statement_walker walker(get_suif_env());
        proc_def->walk(walker);
    }
  OutputInformation("Induction variable substitution pass ends") ;
}


Walker::ApplyStatus ivs_c_for_statement_walker::operator () (SuifObject *x) {
    SuifEnv *env = get_env();
    CForStatement *c_for_stmt = to<CForStatement>(x);  
    Statement *body = c_for_stmt->get_body();

    if (body) {

	current_c_for_stmt = c_for_stmt;
 
	aux_induction_var_stmts = new list<StoreVariableStatement*>;

	induction_vars = get_c_for_induction_variables(c_for_stmt);
	loop_invariants = get_loop_invariants(body, induction_vars);

	aux_induction_vars = new list<VariableSymbol*>;
	non_aux_induction_vars = new list<VariableSymbol*>;

        identify_aux_induction_variables();
        reduce_aux_induction_variables();
        find_aux_induction_variable_init_exprs();

        for (list<StoreVariableStatement*>::iterator iter = aux_induction_var_stmts->begin();
             iter != aux_induction_var_stmts->end(); iter++) {

             ivs_array_reference_expression_walker walker(env, *iter);
             body->walk(walker);
        }

    }

    return Walker::Continue;
}


Walker::ApplyStatus ivs_array_reference_expression_walker::operator () (SuifObject *x) {
    SuifEnv *env = get_env();
    ArrayReferenceExpression *array_ref_expr = to<ArrayReferenceExpression>(x);
     
    Expression *array_index_expr = array_ref_expr->get_index(); 

    ivs_load_variable_expression_walker walker(env, aux_induction_var_stmt);
    array_index_expr->walk(walker);

    return Walker::Continue;
}

Walker::ApplyStatus ivs_load_variable_expression_walker::operator () (SuifObject *x) {
    SuifEnv *env = get_env();
    LoadVariableExpression *load_var_expr = to<LoadVariableExpression>(x);

    if(load_var_expr->get_source() != aux_induction_var_stmt->get_destination())
       return Walker::Continue;

    VariableSymbol *new_induction_sym = induction_vars->front();		

    BrickAnnote *inc_expr_annote = to<BrickAnnote>(aux_induction_var_stmt->lookup_annote_by_name("aux_inc_expr")); 
    BrickAnnote *init_expr_annote = to<BrickAnnote>(aux_induction_var_stmt->lookup_annote_by_name("aux_init_expr")); 

    Expression *inc_expr = to<Expression>((to<SuifObjectBrick>(inc_expr_annote->get_brick(0)))->get_object()); 
    Expression *init_expr = to<Expression>((to<SuifObjectBrick>(init_expr_annote->get_brick(0)))->get_object());

    Statement *load_var_expr_parent_stmt = get_expression_owner(load_var_expr);

    int adjustment = 0;
    if(is_textually_preceeding(load_var_expr_parent_stmt, aux_induction_var_stmt))
       adjustment = 1; 

    DataType *expr_result_type = load_var_expr->get_result_type();
    LoadVariableExpression *new_index_var_expr = create_load_variable_expression(env, expr_result_type, new_induction_sym);

    Expression *replacement = NULL;
    if(adjustment)
       replacement = create_binary_expression(env, expr_result_type, String("subtract"), new_index_var_expr,
					create_int_constant(env, expr_result_type, IInteger(adjustment)));
    else replacement = new_index_var_expr;

    replacement = create_binary_expression(env, expr_result_type, String("multiply"),
					replacement, to<Expression>(deep_suif_clone(inc_expr)));
    replacement = create_binary_expression(env, expr_result_type, String("add"),
					replacement, to<Expression>(deep_suif_clone(init_expr)));

    load_var_expr->get_parent()->replace(load_var_expr, replacement);
    return Walker::Continue;
}

void solve_ivs_statement(Statement *s){

    if (is_a<ScopeStatement>(s))
 	solve_ivs_statement((to<ScopeStatement>(s))->get_body());
    else if (is_a<StoreVariableStatement>(s))
	solve_ivs_store_variable_stmt(s);
    else if (is_a<StatementList>(s))
        for (Iter<Statement*> iter = (to<StatementList>(s))->get_child_statement_iterator();
             iter.is_valid(); iter.next())
	     solve_ivs_statement(iter.current());

}

void solve_ivs_store_variable_stmt(Statement *s){
    StoreVariableStatement *store_var_stmt = to<StoreVariableStatement>(s);

    VariableSymbol *dest_var = store_var_stmt->get_destination();

    BrickAnnote *killed_stmts = to<BrickAnnote>(store_var_stmt->lookup_annote_by_name("killed_stmts"));
    int defs_inside_the_loop_count = 0;
    int defs_outside_the_loop_count = 0;
 
    for(Iter<SuifBrick*> iter = killed_stmts->get_brick_iterator();
        iter.is_valid(); iter.next()){

	SuifObjectBrick *sob = to<SuifObjectBrick>(iter.current());
	StoreVariableStatement *r_def = to<StoreVariableStatement>(sob->get_object());
       
        if(is_enclosing(current_c_for_stmt, r_def))
           defs_inside_the_loop_count++;
	else defs_outside_the_loop_count++;
    }

    if(defs_inside_the_loop_count != 1){
       if(!is_in_list(dest_var, non_aux_induction_vars))
          non_aux_induction_vars->push_back(dest_var);
       return;
    }

    if(defs_outside_the_loop_count > 1){
       if(!is_in_list(dest_var, non_aux_induction_vars))
          non_aux_induction_vars->push_back(dest_var);
       return;
    }

    if(!is_a<BinaryExpression>(store_var_stmt->get_value())){
       if(!is_in_list(dest_var, non_aux_induction_vars))
          non_aux_induction_vars->push_back(dest_var);
       return;
    } 

    SuifEnv *env = store_var_stmt->get_suif_env();
    BinaryExpression *binary_expr = to<BinaryExpression>(store_var_stmt->get_value());
    bool is_const_expr_negative = 0;    

    if(store_var_stmt->lookup_annote_by_name("aux_load_var_expr"))
       store_var_stmt->remove_annote_by_name("aux_load_var_expr");
    BrickAnnote *aux_load_var_expr = create_brick_annote(env, "aux_load_var_expr");
    store_var_stmt->append_annote(aux_load_var_expr);

    if(store_var_stmt->lookup_annote_by_name("aux_inc_expr"))
       store_var_stmt->remove_annote_by_name("aux_inc_expr");
    BrickAnnote *aux_inc_expr = create_brick_annote(env, "aux_inc_expr");
    store_var_stmt->append_annote(aux_inc_expr);

    if(is_a_qualified_load_var_expr(binary_expr->get_source1()) &&
       is_a_qualified_constant_expr(binary_expr->get_source2())){

       if(binary_expr->get_opcode() == String("subtract")) 
	  is_const_expr_negative = 1;
       else if(binary_expr->get_opcode() != String("add")){ 
          if(!is_in_list(dest_var, non_aux_induction_vars))
             non_aux_induction_vars->push_back(dest_var);
          return;
       }

       aux_load_var_expr->append_brick(create_suif_object_brick(env, binary_expr->get_source1()));
       if(!is_const_expr_negative)
          aux_inc_expr->append_brick(create_suif_object_brick(env, binary_expr->get_source2()));
       else aux_inc_expr->append_brick(create_suif_object_brick(env, create_unary_expression(env,  
		binary_expr->get_source2()->get_result_type(), String("negate"), binary_expr->get_source2())));

    }else if(is_a_qualified_load_var_expr(binary_expr->get_source2()) &&
       is_a_qualified_constant_expr(binary_expr->get_source1())){

       aux_load_var_expr->append_brick(create_suif_object_brick(env, binary_expr->get_source2()));
       aux_inc_expr->append_brick(create_suif_object_brick(env, binary_expr->get_source1()));

       if(binary_expr->get_opcode() != String("add")){ 
          if(!is_in_list(dest_var, non_aux_induction_vars))
             non_aux_induction_vars->push_back(dest_var);
          return;
       }
    }else {

       if(!is_in_list(dest_var, non_aux_induction_vars))
          non_aux_induction_vars->push_back(dest_var);
       return;
    }

    if(!is_in_list(store_var_stmt, aux_induction_var_stmts))
       aux_induction_var_stmts->push_back(store_var_stmt);

}

bool is_a_qualified_load_var_expr(Expression *e){

    if(!is_a<LoadVariableExpression>(e))
       return 0;

    LoadVariableExpression *load_var_expr = to<LoadVariableExpression>(e);

    if(is_in_list(load_var_expr->get_source(), non_aux_induction_vars))
       return 0;

    if(is_in_list(load_var_expr->get_source(), loop_invariants))
       return 0;

    BrickAnnote *reaching_defs = to<BrickAnnote>(load_var_expr->lookup_annote_by_name("reaching_defs"));
    int defs_inside_the_loop_count = 0;
    int defs_outside_the_loop_count = 0;

    for(Iter<SuifBrick*> iter = reaching_defs->get_brick_iterator();
        iter.is_valid(); iter.next()){

	SuifObjectBrick *sob = to<SuifObjectBrick>(iter.current());
	StoreVariableStatement *r_def = to<StoreVariableStatement>(sob->get_object());
       

        if(is_enclosing(current_c_for_stmt, r_def))
           defs_inside_the_loop_count++;
	else defs_outside_the_loop_count++;
    }

    if(defs_inside_the_loop_count != 1)
       return 0;

    if(defs_outside_the_loop_count > 1)
       return 0;

    return 1;
}

bool is_a_qualified_constant_expr(Expression *e){

    list<VariableSymbol*>* vars_in_expr = collect_variable_symbols(e); 
    if(!vars_in_expr->empty() && !is_items_in_list(vars_in_expr, loop_invariants))
       return 0;
 
    return 1;

}

void find_aux_induction_variable_init_exprs(){
   SuifEnv* env = current_c_for_stmt->get_suif_env();

   list<StoreVariableStatement*>* to_be_removed = new list<StoreVariableStatement*>;

   for(list<StoreVariableStatement*>::iterator iter = aux_induction_var_stmts->begin();
       iter != aux_induction_var_stmts->end(); iter++){

       StoreVariableStatement *aux_induction_var_stmt = *iter;
 
       BrickAnnote *killed_stmts = to<BrickAnnote>(aux_induction_var_stmt->lookup_annote_by_name("killed_stmts"));
 
       int defs_outside_the_loop_count = 0;
       int defs_inside_the_loop_count = 0;
       StoreVariableStatement *init_def = NULL;
 
       for(Iter<SuifBrick*> iter = killed_stmts->get_brick_iterator();
           iter.is_valid(); iter.next()){

	   SuifObjectBrick *sob = to<SuifObjectBrick>(iter.current());
	   StoreVariableStatement *r_def = to<StoreVariableStatement>(sob->get_object());
       
           if(!is_enclosing(current_c_for_stmt, r_def)){
	      defs_outside_the_loop_count++;
	      init_def = r_def;
           }else defs_inside_the_loop_count++;
       }

       if(defs_outside_the_loop_count != 1){
          to_be_removed->push_back(aux_induction_var_stmt);  
	  continue;
       }

       if(defs_inside_the_loop_count != 1){
          to_be_removed->push_back(aux_induction_var_stmt);  
	  continue;
       }

       if(aux_induction_var_stmt->lookup_annote_by_name("aux_init_expr"))
          aux_induction_var_stmt->remove_annote_by_name("aux_init_expr");
       BrickAnnote *aux_init_expr = create_brick_annote(env, "aux_init_expr");
       aux_induction_var_stmt->append_annote(aux_init_expr);

       aux_init_expr->append_brick(create_suif_object_brick(env, init_def->get_value()));
   } 

   remove_items_from_list(to_be_removed, aux_induction_var_stmts);   

}

void identify_aux_induction_variables(){

    solve_ivs_statement(current_c_for_stmt->get_body());	

}

void reduce_aux_induction_variables(){

   list<StoreVariableStatement*>* to_be_removed = new list<StoreVariableStatement*>;

   for(list<StoreVariableStatement*>::iterator iter = aux_induction_var_stmts->begin();
       iter != aux_induction_var_stmts->end(); iter++){

       StoreVariableStatement *aux_induction_var_stmt = *iter;
       SuifEnv *env = aux_induction_var_stmt->get_suif_env();
 
       BrickAnnote *aux_inc_expr_annote = to<BrickAnnote>(aux_induction_var_stmt->lookup_annote_by_name("aux_inc_expr")); 
       BrickAnnote *aux_load_var_expr_annote = to<BrickAnnote>(aux_induction_var_stmt->lookup_annote_by_name("aux_load_var_expr"));

       BrickAnnote *reached_uses = to<BrickAnnote>(aux_induction_var_stmt->lookup_annote_by_name("reached_uses")); 
       bool propagate = 1;
 
       for(Iter<SuifBrick*> iter = reached_uses->get_brick_iterator();
           iter.is_valid(); iter.next()){

	   SuifObjectBrick *sob = to<SuifObjectBrick>(iter.current());
	   LoadVariableExpression *use = to<LoadVariableExpression>(sob->get_object());
	   Statement *use_parent = get_expression_owner(use);
       
           if(use_parent == aux_induction_var_stmt){
              propagate = 0;
	      break;	          
           }
       }

       if(propagate){

          for(Iter<SuifBrick*> iter = reached_uses->get_brick_iterator();
              iter.is_valid(); iter.next()){

	      SuifObjectBrick *sob = to<SuifObjectBrick>(iter.current());
	      LoadVariableExpression *use = to<LoadVariableExpression>(sob->get_object());
	      Statement *use_parent = get_expression_owner(use);
	
              if(is_a<StoreVariableStatement>(use_parent))
                 if(is_in_list(to<StoreVariableStatement>(use_parent), aux_induction_var_stmts)){

                    BrickAnnote *aux_inc_expr_annote2 =
				     to<BrickAnnote>(use_parent->lookup_annote_by_name("aux_inc_expr")); 
		     		      
		    Expression* const_expr1 = to<Expression>(
				(to<SuifObjectBrick>(aux_inc_expr_annote->get_brick(0)))->get_object());
		    Expression* const_expr2 = to<Expression>(
				(to<SuifObjectBrick>(aux_inc_expr_annote2->get_brick(0)))->get_object());
		
	            Expression* new_inc_expr = create_binary_expression(env,
							const_expr1->get_result_type(), String("add"), 
							to<Expression>(deep_suif_clone(const_expr1)), 
							to<Expression>(deep_suif_clone(const_expr2))); 

		    empty(aux_inc_expr_annote2);
		    aux_inc_expr_annote2->append_brick(create_suif_object_brick(env, new_inc_expr));


                    BrickAnnote *aux_load_var_expr_annote2 = 
				to<BrickAnnote>(use_parent->lookup_annote_by_name("aux_load_var_expr"));

		    Expression* aux_load_var_expr = to<Expression>(
				(to<SuifObjectBrick>(aux_load_var_expr_annote->get_brick(0)))->get_object());
		    Expression* new_load_var_expr = to<Expression>(deep_suif_clone(aux_load_var_expr));

		    empty(aux_load_var_expr_annote2);
		    aux_load_var_expr_annote2->append_brick(create_suif_object_brick(env, new_load_var_expr));


		    BrickAnnote *aux_load_var_expr_reaching_defs = 
				to<BrickAnnote>(aux_load_var_expr->lookup_annote_by_name("reaching_defs"));
	
                    for(Iter<SuifBrick*> iter = aux_load_var_expr_reaching_defs->get_brick_iterator();
                        iter.is_valid(); iter.next()){

	  	        SuifObjectBrick *sob = to<SuifObjectBrick>(iter.current());
	                StoreVariableStatement *def = to<StoreVariableStatement>(sob->get_object());

		        BrickAnnote *def_reached_uses = 
				to<BrickAnnote>(def->lookup_annote_by_name("reached_uses"));
		        
			def_reached_uses->append_brick(create_suif_object_brick(env, new_load_var_expr)); 

		    }

                    (to<StoreVariableStatement>(use_parent))->set_value(
					create_binary_expression(env, aux_load_var_expr->get_result_type(),
					  			String("add"), new_load_var_expr, new_inc_expr));

		    remove_brick(reached_uses, use);		    
	         }    

          }
	
	  to_be_removed->push_back(aux_induction_var_stmt);

       }

   }

   for(list<StoreVariableStatement*>::iterator iter = to_be_removed->begin();
       iter != to_be_removed->end(); iter++){

       StoreVariableStatement *aux_induction_var_stmt = *iter;
       SuifEnv *env = aux_induction_var_stmt->get_suif_env();
 
       BrickAnnote *reached_uses = to<BrickAnnote>(aux_induction_var_stmt->lookup_annote_by_name("reached_uses")); 

       BrickAnnote *aux_load_var_expr_annote = to<BrickAnnote>(aux_induction_var_stmt->lookup_annote_by_name("aux_load_var_expr"));
       Expression* aux_load_var_expr = to<Expression>((to<SuifObjectBrick>(aux_load_var_expr_annote->get_brick(0)))->get_object());

       BrickAnnote *aux_load_var_expr_reaching_defs = 
				to<BrickAnnote>(aux_load_var_expr->lookup_annote_by_name("reaching_defs"));
	
       for(Iter<SuifBrick*> iter = aux_load_var_expr_reaching_defs->get_brick_iterator();
           iter.is_valid(); iter.next()){

           SuifObjectBrick *sob = to<SuifObjectBrick>(iter.current());
           StoreVariableStatement *def = to<StoreVariableStatement>(sob->get_object());

	   if(!is_enclosing(current_c_for_stmt, def))
	      continue;
		
           for(Iter<SuifBrick*> iter = reached_uses->get_brick_iterator();
               iter.is_valid(); iter.next()){

               SuifObjectBrick *sob = to<SuifObjectBrick>(iter.current());
	       LoadVariableExpression *use = to<LoadVariableExpression>(sob->get_object());
	       Statement *use_parent = get_expression_owner(use);
	
               if(is_a<StoreVariableStatement>(use_parent))
                  if(is_in_list(to<StoreVariableStatement>(use_parent), aux_induction_var_stmts))
	             continue;		

               BrickAnnote *aux_inc_expr_annote =
				to<BrickAnnote>(aux_induction_var_stmt->lookup_annote_by_name("aux_inc_expr")); 
               Expression* aux_inc_expr =
				to<Expression>((to<SuifObjectBrick>(aux_inc_expr_annote->get_brick(0)))->get_object());
	
               BrickAnnote *def_inc_expr_annote =
				to<BrickAnnote>(def->lookup_annote_by_name("aux_inc_expr")); 
               Expression* def_inc_expr =
				to<Expression>((to<SuifObjectBrick>(def_inc_expr_annote->get_brick(0)))->get_object());
	       
	       Expression *replacement = NULL;
	       if(is_textually_preceeding(use_parent, def) &&
	          is_textually_preceeding(use_parent, aux_induction_var_stmt) &&
	          is_textually_preceeding(def, aux_induction_var_stmt)){
	          replacement = create_binary_expression(env, aux_load_var_expr->get_result_type(), String("add"), 
								to<Expression>(deep_suif_clone(aux_inc_expr)),
								to<Expression>(deep_suif_clone(def_inc_expr)));
	          replacement = create_binary_expression(env, aux_load_var_expr->get_result_type(), String("subtract"), 
								to<Expression>(deep_suif_clone(aux_induction_var_stmt->get_value())),
								replacement);
	       }else if(is_textually_preceeding(def, use_parent) &&
	          is_textually_preceeding(use_parent, aux_induction_var_stmt) &&
	          is_textually_preceeding(def, aux_induction_var_stmt)){
	          replacement = create_binary_expression(env, aux_load_var_expr->get_result_type(), String("subtract"), 
								to<Expression>(deep_suif_clone(aux_induction_var_stmt->get_value())),
								to<Expression>(deep_suif_clone(aux_inc_expr)));
	       }else if(is_textually_preceeding(def, use_parent) &&
	          is_textually_preceeding(aux_induction_var_stmt, use_parent) &&
	          is_textually_preceeding(def, aux_induction_var_stmt)){
	          replacement = to<Expression>(deep_suif_clone(aux_induction_var_stmt->get_value()));
	       }else if(is_textually_preceeding(use_parent, def) &&
	          is_textually_preceeding(use_parent, aux_induction_var_stmt) &&
	          is_textually_preceeding(aux_induction_var_stmt, def)){
	          replacement = create_binary_expression(env, aux_load_var_expr->get_result_type(), String("subtract"), 
								to<Expression>(deep_suif_clone(aux_induction_var_stmt->get_value())),
								to<Expression>(deep_suif_clone(aux_inc_expr)));
	       }else if(is_textually_preceeding(use_parent, def) &&
	          is_textually_preceeding(aux_induction_var_stmt, use_parent) &&
	          is_textually_preceeding(aux_induction_var_stmt, def)){
	          replacement = to<Expression>(deep_suif_clone(aux_induction_var_stmt->get_value()));
	       }else if(is_textually_preceeding(def, use_parent) &&
	          is_textually_preceeding(aux_induction_var_stmt, use_parent) &&
	          is_textually_preceeding(aux_induction_var_stmt, def)){
	          replacement = create_binary_expression(env, aux_load_var_expr->get_result_type(), String("subtract"), 
	          						to<Expression>(deep_suif_clone(aux_induction_var_stmt->get_value())),
								to<Expression>(deep_suif_clone(def_inc_expr)));
	       }
	
               use->get_parent()->replace(use, replacement);

          }
       }
   }

   remove_items_from_list(to_be_removed, aux_induction_var_stmts);   

   for(list<StoreVariableStatement*>::iterator iter = to_be_removed->begin();
       iter != to_be_removed->end(); iter++)
       remove_statement(*iter);

}

