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
#include <typebuilder/type_builder.h>
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
#include "normalize_pass.h"

// THIS PASS NORMALIZES REPLACES LOOP INDUCTION VARIABLES W/ COMPILER GENERATED 
// NEW VARIABLES AND HEADERS W/ ONES WHOSE STEP SIZE IS AND LOWER BOUND ARE 1

/**************************** Declarations ************************************/

class n_c_for_statement_walker: public SelectiveWalker {
public:
  n_c_for_statement_walker(SuifEnv *the_env, String lplbl_arg)
    : SelectiveWalker(the_env, CForStatement::get_class_name()) { loop_label_argument = lplbl_arg; }
  Walker::ApplyStatus operator () (SuifObject *x);
private:
  String loop_label_argument;
};

class n_load_variable_expression_walker: public SelectiveWalker {
public:
  n_load_variable_expression_walker(SuifEnv *the_env, VariableSymbol* iv, Expression *lb, int s)
    : SelectiveWalker(the_env, LoadVariableExpression::get_class_name()), index_variable(iv), lower_bound(lb), step(s) {}

  Walker::ApplyStatus operator () (SuifObject *x);
private:
  VariableSymbol* index_variable;
  Expression *lower_bound;
  int step;
};

/**************************** Implementations ************************************/
NormalizePass::NormalizePass(SuifEnv *pEnv) : PipelinablePass(pEnv, "NormalizePass") {}

void NormalizePass::initialize(){
    PipelinablePass::initialize();
    _command_line->set_description("Normalizes a CForStatement");
    OptionString *option_loop_label = new OptionString("Loop label",  &loop_label_argument);
    OptionList *normalize_pass_arguments = new OptionList();
    normalize_pass_arguments->add(option_loop_label);
    _command_line->add(normalize_pass_arguments);
}

void NormalizePass::do_procedure_definition(ProcedureDefinition* proc_def){
    if (proc_def){
        n_c_for_statement_walker walker(get_suif_env(), loop_label_argument);
        proc_def->walk(walker);
    }
}
   
Walker::ApplyStatus n_c_for_statement_walker::operator () (SuifObject *x) {
    SuifEnv *env = get_env();
    CForStatement *c_for_stmt = to<CForStatement>(x);  

    BrickAnnote *loop_label_annote = to<BrickAnnote>(c_for_stmt->lookup_annote_by_name("c_for_label"));
    StringBrick *loop_label_brick = to<StringBrick>(loop_label_annote->get_brick(0));
    String current_loop_label = loop_label_brick->get_value();

    if (current_loop_label != loop_label_argument)
        return Walker::Continue;

    Statement *body = to<Statement>(c_for_stmt->get_body());
    if (body) {
    
	VariableSymbol *c_for_basic_induction_variable = get_c_for_basic_induction_variable(c_for_stmt);
	Expression *c_for_lower_bound_expr = get_c_for_lower_bound_expr(c_for_stmt);
	Expression *c_for_upper_bound_expr = get_c_for_upper_bound_expr(c_for_stmt);
	int c_for_step = get_c_for_step(c_for_stmt);

        String c_for_test_opcode = get_c_for_test_opcode(c_for_stmt); 

        if (is_error_code_set()){
            reset_error_code();
            return Walker::Continue;
        }

        Expression *new_c_for_upper_bound_expr = NULL;	
	TypeBuilder *tb = get_type_builder(env);

	new_c_for_upper_bound_expr = create_binary_expression(env, tb->get_integer_type(), "subtract", 
						to<Expression>(deep_suif_clone(c_for_upper_bound_expr)),
						to<Expression>(deep_suif_clone(c_for_lower_bound_expr)));
	new_c_for_upper_bound_expr = create_binary_expression(env, tb->get_integer_type(), "add", 
						new_c_for_upper_bound_expr,
						create_int_constant(env, tb->get_integer_type(), IInteger(c_for_step)));
	if(c_for_step != 1)
	   new_c_for_upper_bound_expr = create_binary_expression(env, tb->get_floating_point_type(), "divide", 
						new_c_for_upper_bound_expr,
						create_int_constant(env, tb->get_integer_type(), IInteger(c_for_step)));

	set_c_for_lower_bound(c_for_stmt, 1);
	set_c_for_upper_bound_expr(c_for_stmt, new_c_for_upper_bound_expr);

        if (c_for_test_opcode == String("is_less_than") ||
	    c_for_test_opcode == String("is_grater_than"))
            set_c_for_test_opcode(c_for_stmt, String("is_less_than")); 

        if (c_for_test_opcode == String("is_less_than_or_equal_to") ||
            c_for_test_opcode == String("is_greater_than_or_equal_to"))
            set_c_for_test_opcode(c_for_stmt, String("is_less_than_or_equal_to")); 

	set_c_for_step(c_for_stmt, 1);

        n_load_variable_expression_walker walker(env, c_for_basic_induction_variable, 
						 c_for_lower_bound_expr, c_for_step);
	c_for_stmt->get_body()->walk(walker); 

// preparing an expression in the form i = i*step-step+lower_bound; 

	DataType *load_var_type = get_data_type(c_for_basic_induction_variable);
	LoadVariableExpression *load_var = create_load_variable_expression(env, load_var_type,
							c_for_basic_induction_variable);
	Expression *induction_variable_update_expr = create_binary_expression(env, load_var_type, "multiply", load_var,
							create_int_constant(env, load_var_type, IInteger(c_for_step)));
	induction_variable_update_expr = create_binary_expression(env, load_var_type, "subtract", 
							induction_variable_update_expr,
							create_int_constant(env, load_var_type, IInteger(c_for_step)));
	induction_variable_update_expr = create_binary_expression(env, load_var_type, "add", 
							induction_variable_update_expr,
							to<Expression>(deep_suif_clone(c_for_lower_bound_expr)));
        StoreVariableStatement *induction_variable_update_stmt = create_store_variable_statement(env, 
							c_for_basic_induction_variable, induction_variable_update_expr);
        insert_statement_after(c_for_stmt, induction_variable_update_stmt);
    }

    return Walker::Stop;
}

Walker::ApplyStatus n_load_variable_expression_walker::operator () (SuifObject *x){
    SuifEnv *the_env = get_env();
    LoadVariableExpression *the_load_var_expr = to<LoadVariableExpression>(x);
    Expression *replacement = NULL;
  
    if (index_variable == the_load_var_expr->get_source()) {  
      
	VariableSymbol *source_var = the_load_var_expr->get_source();
	DataType *source_var_type = get_data_type(source_var);

  	replacement = to<Expression>(deep_suif_clone(the_load_var_expr));

	if(step != 1)
  	   replacement = create_binary_expression(the_env, source_var_type, "multiply", replacement, 
				create_int_constant(the_env, source_var_type, IInteger(step)));

	replacement = create_binary_expression(the_env, source_var_type, "subtract", replacement,
				create_int_constant(the_env, source_var_type, IInteger(step)));
	replacement = create_binary_expression(the_env, source_var_type, "add", replacement,
				to<Expression>(deep_suif_clone(lower_bound)));

        the_load_var_expr->get_parent()->replace(the_load_var_expr, replacement);
    }

    return Walker::Continue; 
}



