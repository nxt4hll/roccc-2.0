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
#include <suifkernel/command_line_parsing.h>
#include "transforms/procedure_walker_utilities.h"
#include <utils/expression_utils.h>
#include <utils/symbol_utils.h>
#include <utils/type_utils.h>
#include <utils/cloning_utils.h>
#include <cfenodes/cfe.h>
#include <cfenodes/cfe_factory.h>
#include "roccc_utils/loop_utils.h"
#include "prod_consumer_adjustment_pass.h"

// THIS UNROLLING ALGORITHM ASSUMES NORMALIZED LOOPS WHOSE STEP SIZE IS 1 AS INPUTS

/**************************** Declarations ************************************/

class c_for_statement_walker: public SelectiveWalker {
public:
  c_for_statement_walker(SuifEnv *the_env, int uc, int lplvl)
    : SelectiveWalker(the_env, CForStatement::get_class_name()), unroll_count(uc),
								 loop_level(lplvl) {}
  Walker::ApplyStatus operator () (SuifObject *x);
private:
  int unroll_count;
  int loop_level;
};

class array_reference_expression_walker: public SelectiveWalker {
public:
  array_reference_expression_walker(SuifEnv *the_env, VariableSymbol* iv, int uc)
    : SelectiveWalker(the_env, ArrayReferenceExpression::get_class_name()), index(iv), unroll_count(uc) {}

  Walker::ApplyStatus operator () (SuifObject *x);
private:
  VariableSymbol* index;
  int unroll_count;
};

class load_variable_expression_walker: public SelectiveWalker {
public:
  load_variable_expression_walker(SuifEnv *the_env, VariableSymbol* iv, int uc)
    : SelectiveWalker(the_env, LoadVariableExpression::get_class_name()), index_variable(iv), unroll_count(uc) {}

  Walker::ApplyStatus operator () (SuifObject *x);
private:
  VariableSymbol* index_variable;
  int unroll_count;
};

/**************************** Implementations ************************************/
UnrollPass::UnrollPass(SuifEnv *pEnv) : PipelinablePass(pEnv, "UnrollPass") {}

void UnrollPass::initialize(){
    PipelinablePass::initialize();
    _command_line->set_description("Partially unrolls CForStatements");
    OptionInt *loop_level_argument = new OptionInt("Loop level",  &loop_level);
    OptionInt *unroll_count_argument = new OptionInt("Unroll count", &unroll_count);
    OptionList *unroll_pass_arguments = new OptionList();
    unroll_pass_arguments->add(loop_level_argument);
    unroll_pass_arguments->add(unroll_count_argument);
    _command_line->add(unroll_pass_arguments);
}

void UnrollPass::do_procedure_definition(ProcedureDefinition* proc_def){
    if (proc_def){
        c_for_statement_walker walker(get_suif_env(), unroll_count, loop_level);
        proc_def->walk(walker);
    }
}
   
Walker::ApplyStatus c_for_statement_walker::operator () (SuifObject *x) {
    SuifEnv *the_env = get_env();
    CForStatement *the_c_for = to<CForStatement>(x);  
    Statement *body = the_c_for->get_body();

    BrickAnnote *nest_level_annote = to<BrickAnnote>(the_c_for->lookup_annote_by_name("c_for_info"));
    IntegerBrick *nest_level_brick = to<IntegerBrick>(nest_level_annote->get_brick(0));
    int loop_nest_level = (nest_level_brick->get_value()).c_int(); 

    if(loop_nest_level != loop_level)
       return Walker::Continue;

    if (body) {
    
        list<VariableSymbol*>* induction_variables = get_c_for_induction_variables(the_c_for);

        Statement *the_step = the_c_for->get_step();
	the_c_for->set_step(0);
        Statement *the_new_step = to<Statement>(deep_suif_clone(the_step));
        for (list<VariableSymbol*>::iterator iter = induction_variables->begin();
             iter != induction_variables->end(); iter++) {
             load_variable_expression_walker walker(the_env, (*iter), unroll_count-1);
             the_new_step->walk(walker);
        }
	the_c_for->set_step(the_new_step);

/*
	     if (i<unroll_count) 
             {
  	         Expression *condition = to<Expression>(deep_suif_clone(the_test));
                 Statement *then_part = to<Statement>(deep_suif_clone(the_c_for->get_body()));
	         insert_statement_after(then_part, to<Statement>(deep_suif_clone(the_step)));
                 IfStatement *the_if_stmt = create_if_statement(the_env, condition, then_part, 0);
	         insert_statement_after(the_c_for, the_if_stmt);
	     }
*/
	
	the_c_for->set_body(0);
	StatementList *new_body = create_statement_list(the_env);
        Statement *original_body = to<Statement>(deep_suif_clone(body));

	new_body->append_statement(body);
        for (int k=0; k<unroll_count; k++) 
        {
             Statement *unrolled_body = to<Statement>(deep_suif_clone(original_body));
	     for (list<VariableSymbol*>::iterator iter = induction_variables->begin();
                  iter != induction_variables->end(); 
	          iter++) 
             {
                  array_reference_expression_walker walker(the_env, (*iter), k);
                  unrolled_body->walk(walker);
             }
             new_body->append_statement(unrolled_body); 
        }

	the_c_for->set_body(new_body);
    }
    return Walker::Continue;
}

Walker::ApplyStatus array_reference_expression_walker::operator () (SuifObject *x){

    SuifEnv *the_env = get_env();
    ArrayReferenceExpression *the_array_ref_expr = to<ArrayReferenceExpression>(x);
       
    load_variable_expression_walker walker(the_env, index, unroll_count);
    the_array_ref_expr->get_index()->walk(walker);

    return Walker::Continue; 
}

Walker::ApplyStatus load_variable_expression_walker::operator () (SuifObject *x){

    SuifEnv *the_env = get_env();
    LoadVariableExpression *the_load_var_expr = to<LoadVariableExpression>(x);
    Expression *replacement = NULL;
  
    if (index_variable == the_load_var_expr->get_source()) {  
           
	Expression *src1 = clone_expression(the_load_var_expr);
        Expression *src2 = create_int_constant(the_env, unroll_count+1);
        replacement = create_binary_expression(the_env, the_load_var_expr->get_result_type(), "add", src1, src2);

        the_load_var_expr->get_parent()->replace(the_load_var_expr, replacement);
    }

    return Walker::Continue; 
}


