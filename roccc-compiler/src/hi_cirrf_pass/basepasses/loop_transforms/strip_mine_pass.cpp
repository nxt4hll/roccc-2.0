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
#include "roccc_utils/symbol_utils.h"
#include "strip_mine_pass.h"
#include <typebuilder/type_builder.h>

// THIS TILE PASS GENERATES A 1D STRIP-MINED VERSION OF A GIVEN LOOP

/**************************** Declarations ************************************/

class smp_c_for_statement_walker: public SelectiveWalker {
public:
  smp_c_for_statement_walker(SuifEnv *the_env, String lplbl_arg, int ssize)
    : SelectiveWalker(the_env, CForStatement::get_class_name()), strip_size(ssize) { loop_label_argument = lplbl_arg; }
  Walker::ApplyStatus operator () (SuifObject *x);
private:
  String loop_label_argument;
  int strip_size;
};

class smp_load_variable_expression_walker: public SelectiveWalker {
public:
  smp_load_variable_expression_walker(SuifEnv *the_env, VariableSymbol* iv, VariableSymbol* sv)
    : SelectiveWalker(the_env, LoadVariableExpression::get_class_name()), index_variable(iv), strip_index_variable(sv) {}

  Walker::ApplyStatus operator () (SuifObject *x);
private:
  VariableSymbol* index_variable;
  VariableSymbol* strip_index_variable;
};

/**************************** Implementations ************************************/
StripMinePass::StripMinePass(SuifEnv *pEnv) : PipelinablePass(pEnv, "StripMinePass") {}

void StripMinePass::initialize(){
    PipelinablePass::initialize();
    _command_line->set_description("Stripmines CForStatements");
    OptionString *option_loop_label = new OptionString("Loop level",  &loop_label_argument);
    OptionInt *option_strip_size = new OptionInt("Strip size", &strip_size_argument);
    OptionList *strip_mine_pass_arguments = new OptionList();
    strip_mine_pass_arguments->add(option_loop_label);
    strip_mine_pass_arguments->add(option_strip_size);
    _command_line->add(strip_mine_pass_arguments);
}

void StripMinePass::do_procedure_definition(ProcedureDefinition* proc_def){
    if (proc_def){
        smp_c_for_statement_walker walker(get_suif_env(), loop_label_argument, strip_size_argument);
        proc_def->walk(walker);
    }
}

CForStatement* make_strip(SuifEnv *env, VariableSymbol* loop_counter, int upper_bound, int step_size, Statement* body){

    TypeBuilder *type_builder = get_type_builder(env);
	
    Statement *before_stmt = create_store_variable_statement(env, loop_counter, create_int_constant(env, IInteger(0)));
    Expression *test_expr = create_binary_expression(env, type_builder->get_boolean_type(), "is_less_than", create_var_use(loop_counter), 
						      create_int_constant(env, IInteger(upper_bound)));
    Expression *step_expr = create_binary_expression(env, loop_counter->get_type()->get_base_type(), "add", 
						     create_var_use(loop_counter), create_int_constant(env, IInteger(step_size)));
    Statement *step_stmt = create_store_variable_statement(env, loop_counter, step_expr);

    return create_c_for_statement(env, before_stmt, test_expr, step_stmt, body);
}
   
Walker::ApplyStatus smp_c_for_statement_walker::operator () (SuifObject *x) {
    SuifEnv *the_env = get_env();
    CForStatement *c_for_stmt = to<CForStatement>(x);  

    BrickAnnote *loop_label_annote = to<BrickAnnote>(c_for_stmt->lookup_annote_by_name("c_for_label"));
    StringBrick *loop_label_brick = to<StringBrick>(loop_label_annote->get_brick(0));
    String current_loop_label = loop_label_brick->get_value();

    if (current_loop_label != loop_label_argument)
        return Walker::Continue;

    Statement *body = c_for_stmt->get_body();
    if (body) {

        VariableSymbol* loop_counter = get_c_for_basic_induction_variable(c_for_stmt);

        VariableSymbol* strip_loop_counter = new_anonymous_variable(the_env, find_scope(c_for_stmt), loop_counter->get_type());
	name_variable(strip_loop_counter);

	// step size of the strip is the step size of the original loop
	int strip_step_size = get_c_for_step(c_for_stmt);  

	c_for_stmt->set_body(0);

	CForStatement* strip = make_strip(the_env, strip_loop_counter, strip_size, strip_step_size, body);
	c_for_stmt->set_body(strip);

	set_c_for_step(c_for_stmt, strip_size);

	smp_load_variable_expression_walker walker(the_env, loop_counter, strip_loop_counter);
        body->walk(walker);

    }
    return Walker::Truncate;
}

Walker::ApplyStatus smp_load_variable_expression_walker::operator () (SuifObject *x){

    SuifEnv *the_env = get_env();
    LoadVariableExpression *the_load_var_expr = to<LoadVariableExpression>(x);
    Expression *replacement = NULL;
  
    if (index_variable == the_load_var_expr->get_source()) {  
           
	Expression *src1 = clone_expression(the_load_var_expr);
        Expression *src2 = create_load_variable_expression(the_env, the_load_var_expr->get_result_type(), strip_index_variable);
        replacement = create_binary_expression(the_env, the_load_var_expr->get_result_type(), "add", src1, src2);

        the_load_var_expr->get_parent()->replace(the_load_var_expr, replacement);
    }

    return Walker::Continue; 
}



