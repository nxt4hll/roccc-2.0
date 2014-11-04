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
#include "roccc_utils/IR_utils.h"
#include "skew_pass.h"

// THIS PASS GENERATES A SKEWED VERSION OF A GIVEN 2D LOOP
// IT'S A USEFUL TRANSFORMATION WHEN FOLLOWED BY AND INTERCHANGE 
// ESP. FOR PARALLELIZING WAVEFRONT COMPUTATIONS

// FIX ME: DATA DEP. ANALYZER SHOULD SEND TO THIS PASS THE LOOP TO BE SKEWED W/ RESTECT TO
//         AS WELL AS THE SKEW_FACTOR 
// CURRENTLY THE LOOP IS SKEWED W/ RESPECT TO THE INNERMOST ENCLOSING CFORSTATEMENT AND
// THE SKEW_FACTOR IS ONE.

/**************************** Declarations ************************************/

class sp_c_for_statement_walker: public SelectiveWalker {
public:
  sp_c_for_statement_walker(SuifEnv *the_env, String lplbl_arg)
    : SelectiveWalker(the_env, CForStatement::get_class_name()) { loop_label_argument = lplbl_arg; }
  Walker::ApplyStatus operator () (SuifObject *x);
private:
  String loop_label_argument;
};

class sp_array_reference_expression_walker: public SelectiveWalker {
public:
  sp_array_reference_expression_walker(SuifEnv *the_env, VariableSymbol* iv, VariableSymbol *sv, int sf)
    : SelectiveWalker(the_env, ArrayReferenceExpression::get_class_name()), index_variable(iv), 
							skew_index_variable(sv), skew_factor(sf) {}

  Walker::ApplyStatus operator () (SuifObject *x);
private:
  VariableSymbol* index_variable;
  VariableSymbol* skew_index_variable; 
  int skew_factor; 
};

class sp_load_variable_expression_walker: public SelectiveWalker {
public:
  sp_load_variable_expression_walker(SuifEnv *the_env, VariableSymbol* iv, VariableSymbol* sv, int sf)
    : SelectiveWalker(the_env, LoadVariableExpression::get_class_name()), index_variable(iv), 
							skew_index_variable(sv), skew_factor(sf) {}

  Walker::ApplyStatus operator () (SuifObject *x);
private:
  VariableSymbol* index_variable;
  VariableSymbol* skew_index_variable;
  int skew_factor; 
};

/**************************** Implementations ************************************/
SkewPass::SkewPass(SuifEnv *pEnv) : PipelinablePass(pEnv, "SkewPass") {}

void SkewPass::initialize(){
    PipelinablePass::initialize();
    _command_line->set_description("Skews a CForStatement");
    OptionString *option_loop_label = new OptionString("Loop label",  &loop_label_argument);
    OptionList *skew_pass_arguments = new OptionList();
    skew_pass_arguments->add(option_loop_label);
    _command_line->add(skew_pass_arguments);
}

void SkewPass::do_procedure_definition(ProcedureDefinition* proc_def){
    if (proc_def){
        sp_c_for_statement_walker walker(get_suif_env(), loop_label_argument);
        proc_def->walk(walker);
    }
}

Walker::ApplyStatus sp_c_for_statement_walker::operator () (SuifObject *x) {
    SuifEnv *the_env = get_env();
    CForStatement *c_for_stmt = to<CForStatement>(x);  

    BrickAnnote *loop_label_annote = to<BrickAnnote>(c_for_stmt->lookup_annote_by_name("c_for_label"));
    StringBrick *loop_label_brick = to<StringBrick>(loop_label_annote->get_brick(0));
    String current_loop_label = loop_label_brick->get_value();

    if (current_loop_label != loop_label_argument)
        return Walker::Continue;

    Statement *body = c_for_stmt->get_body();
    if (body) {

	Iter<CForStatement> iter = object_iterator<CForStatement>(body);
	if(iter.is_valid())
	   return Walker::Continue;

	CForStatement* parent_c_for_stmt = get_enclosing_c_for_stmt(c_for_stmt);
	int skew_factor = 1;

        VariableSymbol* c_for_loop_counter = get_c_for_basic_induction_variable(c_for_stmt);
        VariableSymbol* parent_loop_counter = get_c_for_basic_induction_variable(parent_c_for_stmt);

	Expression *c_for_lower_bound = get_c_for_lower_bound_expr(c_for_stmt);

        Expression *adjustment = create_load_variable_expression(the_env, c_for_lower_bound->get_result_type(), parent_loop_counter);
        adjustment = create_binary_expression(the_env, c_for_lower_bound->get_result_type(), "multiply", adjustment, 
						create_int_constant(the_env, adjustment->get_result_type(), IInteger(skew_factor)));

        Expression *replacement = create_binary_expression(the_env, c_for_lower_bound->get_result_type(), "add", 
						to<Expression>(deep_suif_clone(c_for_lower_bound)), adjustment);
	set_c_for_lower_bound_expr(c_for_stmt, replacement);

	Expression *c_for_upper_bound = get_c_for_upper_bound_expr(c_for_stmt);
        replacement = create_binary_expression(the_env, c_for_upper_bound->get_result_type(), "add",
						to<Expression>(deep_suif_clone(c_for_upper_bound)),
						to<Expression>(deep_suif_clone(adjustment)));
	set_c_for_upper_bound_expr(c_for_stmt, replacement);

	sp_array_reference_expression_walker walker(the_env, c_for_loop_counter, parent_loop_counter, 1);
        body->walk(walker);

    }
    return Walker::Truncate;
}

Walker::ApplyStatus sp_array_reference_expression_walker::operator () (SuifObject *x){
    SuifEnv *the_env = get_env();
    ArrayReferenceExpression *the_array_ref_expr = to<ArrayReferenceExpression>(x);
       
    sp_load_variable_expression_walker walker(the_env, index_variable, skew_index_variable, skew_factor);
    the_array_ref_expr->get_index()->walk(walker);

    return Walker::Continue; 
}

Walker::ApplyStatus sp_load_variable_expression_walker::operator () (SuifObject *x){
    SuifEnv *the_env = get_env();
    LoadVariableExpression *the_load_var_expr = to<LoadVariableExpression>(x);
    Expression *replacement = NULL;
  
    if (index_variable == the_load_var_expr->get_source()) {  
           
        Expression *src1 = create_load_variable_expression(the_env, the_load_var_expr->get_result_type() , skew_index_variable);
        Expression *src2 = create_int_constant(the_env, the_load_var_expr->get_result_type() , IInteger(skew_factor));
        replacement = create_binary_expression(the_env, the_load_var_expr->get_result_type(), "multiply", src1, src2);

        Expression *src3 = create_load_variable_expression(the_env, the_load_var_expr->get_result_type() , index_variable);
        replacement = create_binary_expression(the_env, the_load_var_expr->get_result_type(), "subtract", src3, replacement);

        the_load_var_expr->get_parent()->replace(the_load_var_expr, replacement);
    }

    return Walker::Continue; 
}
