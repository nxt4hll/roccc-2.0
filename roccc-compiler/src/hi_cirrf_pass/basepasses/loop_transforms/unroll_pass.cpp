// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#include "common/system_specific.h"
#include <common/suif_copyright.h>

#include <sstream>
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
#include <typebuilder/type_builder.h>
#include <suifkernel/group_walker.h> 
#include <suifkernel/command_line_parsing.h>
#include "transforms/procedure_walker_utilities.h"
#include <utils/expression_utils.h>
#include <utils/symbol_utils.h>
#include <utils/type_utils.h>
#include <utils/cloning_utils.h>
#include <cfenodes/cfe.h>
#include <cfenodes/cfe_factory.h>
#include "roccc_utils/annote_utils.h"
#include "roccc_utils/loop_utils.h"
#include "roccc_utils/symbol_utils.h"
#include "roccc_utils/IR_utils.h"
#include "roccc_utils/warning_utils.h"
#include "unroll_pass.h"

using namespace std;

// THIS UNROLLING ALGORITHM ASSUMES NORMALIZED LOOPS WHOSE STEP SIZE IS 1 AS INPUTS

/**************************** Declarations ************************************/

class c_for_statement_walker: public SelectiveWalker {
public:
  c_for_statement_walker(SuifEnv *the_env, String loop_lbl, int uc)
    : SelectiveWalker(the_env, CForStatement::get_class_name()), loop_label_argument(loop_lbl),
								 unroll_count(uc) {}
  Walker::ApplyStatus operator () (SuifObject *x);
private:
  String loop_label_argument;
  int unroll_count;
};

class load_variable_expression_walker: public SelectiveWalker {
public:
  load_variable_expression_walker(SuifEnv *the_env, VariableSymbol* iv, int inc)
    : SelectiveWalker(the_env, LoadVariableExpression::get_class_name()), index_variable(iv), increment(inc) {}

  Walker::ApplyStatus operator () (SuifObject *x);
private:
  VariableSymbol* index_variable;
  int increment;
};

class load_variable_expression_walker2: public SelectiveWalker {
public:
  load_variable_expression_walker2(SuifEnv *the_env, VariableSymbol* iv, VariableSymbol* n_iv)
    : SelectiveWalker(the_env, LoadVariableExpression::get_class_name()), index_variable(iv), new_index_variable(n_iv) {}

  Walker::ApplyStatus operator () (SuifObject *x);
private:
  VariableSymbol* index_variable;   
  VariableSymbol* new_index_variable;
};

class store_variable_statement_walker: public SelectiveWalker {
public:
  store_variable_statement_walker(SuifEnv *the_env, VariableSymbol* iv, VariableSymbol* n_iv)
    : SelectiveWalker(the_env, StoreVariableStatement::get_class_name()), index_variable(iv), new_index_variable(n_iv) {}

  Walker::ApplyStatus operator () (SuifObject *x);
private:
  VariableSymbol* index_variable;   
  VariableSymbol* new_index_variable;
};

ExecutionObject *proc_def_body;

/**************************** Implementations ************************************/
UnrollPass::UnrollPass(SuifEnv *pEnv) : PipelinablePass(pEnv, "UnrollPass") {}

void UnrollPass::initialize(){
    PipelinablePass::initialize();
    _command_line->set_description("Partially unrolls CForStatements");
    OptionString *option_loop_label = new OptionString("Loop label",  &loop_label_argument);
    OptionInt *option_unroll_count = new OptionInt("Unroll count", &unroll_count_argument);
    OptionList *unroll_pass_arguments = new OptionList();
    unroll_pass_arguments->add(option_loop_label);
    unroll_pass_arguments->add(option_unroll_count);
    _command_line->add(unroll_pass_arguments);
}

void UnrollPass::do_procedure_definition(ProcedureDefinition* proc_def)
{
  std::stringstream informationStream ;
  informationStream << "Unrolling " << loop_label_argument << " by "
		    << unroll_count_argument << "." ;
  OutputInformation(informationStream.str().c_str()) ;
  
  if (proc_def)
  {
    proc_def_body = proc_def->get_body();
    c_for_statement_walker walker(get_suif_env(), loop_label_argument, unroll_count_argument);
    proc_def->walk(walker);
  }
  OutputInformation("Loop unroll pass ends") ;

}
   
Walker::ApplyStatus c_for_statement_walker::operator () (SuifObject *x) 
{
    SuifEnv *the_env = get_env();
    CForStatement *the_c_for = to<CForStatement>(x);  

    //    if(!is_stmt_within_begin_end_hw_marks(the_c_for))
    //   return Walker::Continue;  

    Statement *body = the_c_for->get_body();

    BrickAnnote *loop_label_annote = 
      to<BrickAnnote>(the_c_for->lookup_annote_by_name("c_for_label"));

    // Not every loop can have a label.  If there is no label, just return.

    if (loop_label_annote == NULL)
    {
      return Walker::Continue ;
    }

    StringBrick *loop_label_brick = 
      to<StringBrick>(loop_label_annote->get_brick(0));
    String current_loop_label = loop_label_brick->get_value(); 

    if(current_loop_label != loop_label_argument)
       return Walker::Continue;

    if (body) {

        VariableSymbol* induction_variable = get_c_for_basic_induction_variable(the_c_for);
	//	TypeBuilder *tb = get_type_builder(the_env);

	// Jason on 12/21/2009: For the time being we will not output the
	//  remainder loop.  If the user unrolls by an unusual amount than
	//  that is too bad...

// LOOP THAT COMPUTES THE FIRST (1 TO UPPER_BOUND % UNROLL_FACTOR) MANY ITERATIONS

	CForStatement *remainder_loop = to<CForStatement>(deep_suif_clone(the_c_for));	

	Expression *upper_bound_expr = get_c_for_upper_bound_expr(remainder_loop);
	Expression *lower_bound_expr = get_c_for_lower_bound_expr(remainder_loop);

	DataType *result_data_type = upper_bound_expr->get_result_type();
	Expression *new_upper_bound_expr = create_binary_expression(the_env, result_data_type,
							String("subtract"),
							to<Expression>(deep_suif_clone(upper_bound_expr)),
							to<Expression>(deep_suif_clone(lower_bound_expr)));

	if(get_c_for_test_opcode(the_c_for) == String("is_less_than_or_equal_to"))
 	   new_upper_bound_expr = create_binary_expression(the_env, upper_bound_expr->get_result_type(),
							String("add"), new_upper_bound_expr,
							create_int_constant(the_env, result_data_type, IInteger(1)));
	new_upper_bound_expr = create_binary_expression(the_env, result_data_type, String("remainder"),
							new_upper_bound_expr,
							create_int_constant(the_env, result_data_type, IInteger(unroll_count+1)));
	new_upper_bound_expr = create_binary_expression(the_env, result_data_type, String("add"),
							to<Expression>(deep_suif_clone(lower_bound_expr)),
							new_upper_bound_expr);
	set_c_for_upper_bound_expr(remainder_loop, new_upper_bound_expr);

        VariableSymbol *new_induction_variable = new_anonymous_variable(the_env, find_scope(proc_def_body), induction_variable->get_type());
        name_variable(new_induction_variable, induction_variable->get_name());
        store_variable_statement_walker walker(the_env, induction_variable, new_induction_variable);
        remainder_loop->walk(walker);
        load_variable_expression_walker2 walker2(the_env, induction_variable, new_induction_variable);
        remainder_loop->walk(walker2);

	// Commented out, so there is a big memory leak, so clean up
	//  in the next patch!
	//	insert_statement_before(the_c_for, remainder_loop);
	set_c_for_test_opcode(remainder_loop, "is_less_than");
        BrickAnnote *remainder_loop_label_annote = to<BrickAnnote>(remainder_loop->lookup_annote_by_name("c_for_label"));
        empty(remainder_loop_label_annote);
	remainder_loop_label_annote->insert_brick(0, create_string_brick(the_env, String("remainder_") + current_loop_label));

// LOOP THAT COMPUTES THE REMAINING ((UPPER_BOUND % UNROLL_FACTOR) TO UPPER_BOUND) MANY ITERATIONS

	set_c_for_lower_bound_expr(the_c_for, to<Expression>(deep_suif_clone(new_upper_bound_expr)));
	
	int original_c_for_step = get_c_for_step(the_c_for);
	set_c_for_step(the_c_for, original_c_for_step * (unroll_count+1));

	the_c_for->set_body(0);

	StatementList *unrolled_body = create_statement_list(the_env);
        for (int k=0; k<=unroll_count; k++) 
        {
             Statement *iter_body = to<Statement>(deep_suif_clone(body));

             load_variable_expression_walker walker(the_env, induction_variable, k*original_c_for_step);
             iter_body->walk(walker);

             unrolled_body->append_statement(iter_body); 
        }

	the_c_for->set_body(unrolled_body);

    }
    return Walker::Continue;
}

Walker::ApplyStatus load_variable_expression_walker::operator () (SuifObject *x){

    SuifEnv *the_env = get_env();
    LoadVariableExpression *the_load_var_expr = to<LoadVariableExpression>(x);
    Expression *replacement = NULL;
  
    if (index_variable == the_load_var_expr->get_source()) {  
           
	Expression *src1 = clone_expression(the_load_var_expr);
        Expression *src2 = create_int_constant(the_env, IInteger(increment));
        replacement = create_binary_expression(the_env, the_load_var_expr->get_result_type(), "add", src1, src2);

        the_load_var_expr->get_parent()->replace(the_load_var_expr, replacement);
    }

    return Walker::Continue; 
}

Walker::ApplyStatus load_variable_expression_walker2::operator () (SuifObject *x){
        
  //    SuifEnv *the_env = get_env();
    LoadVariableExpression *the_load_var_expr = to<LoadVariableExpression>(x);
    //Expression *replacement = NULL;

    if (index_variable == the_load_var_expr->get_source())
        the_load_var_expr->set_source(new_index_variable);
       
    return Walker::Continue;
}
    
Walker::ApplyStatus store_variable_statement_walker::operator () (SuifObject *x){
        
  // SuifEnv *the_env = get_env();
    StoreVariableStatement *the_store_var_stmt = to<StoreVariableStatement>(x);
    //Expression *replacement = NULL;

    if (index_variable == the_store_var_stmt->get_destination())
        the_store_var_stmt->set_destination(new_index_variable);
       
    return Walker::Continue;
}
    


