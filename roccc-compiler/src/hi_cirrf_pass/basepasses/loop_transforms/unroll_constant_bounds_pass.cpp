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
#include <suifkernel/command_line_parsing.h>
#include "transforms/procedure_walker_utilities.h"
#include <utils/expression_utils.h>
#include <utils/symbol_utils.h>
#include <utils/type_utils.h>
#include <utils/cloning_utils.h>
#include <utils/trash_utils.h>
#include <cfenodes/cfe.h>
#include <cfenodes/cfe_factory.h>
#include "roccc_utils/loop_utils.h"
#include "roccc_utils/IR_utils.h"
#include "roccc_utils/warning_utils.h"
#include "unroll_constant_bounds_pass.h"

// To deal with llvm we need to mark the index of the loop we are completely
//  unrolling as a removed variable.  This will prevent us from outputting
//  a size for it and messing up...

/**************************** Declarations ************************************/

class ucbp_c_for_statement_walker: public SelectiveWalker 
{
public:
  ucbp_c_for_statement_walker(SuifEnv *the_env, String loop_lbl, int ufl)
    : SelectiveWalker(the_env, CForStatement::get_class_name()), 
      loop_label_argument(loop_lbl), 
      unroll_factor_limit(ufl) 
  {
    ;
  }
  Walker::ApplyStatus operator () (SuifObject *x);
private:
  String loop_label_argument;
  int unroll_factor_limit;
} ;

class ucbp_load_variable_expression_walker: public SelectiveWalker 
{
public:
  ucbp_load_variable_expression_walker(SuifEnv *the_env, VariableSymbol* iv, int i)
    : SelectiveWalker(the_env, LoadVariableExpression::get_class_name()), 
      index_variable(iv), 
      increment(i) 
  {
    ;
  }

  Walker::ApplyStatus operator () (SuifObject *x);
private:
  VariableSymbol* index_variable;
  int increment;
} ;

/**************************** Implementations ************************************/
UnrollConstantBoundsPass::UnrollConstantBoundsPass(SuifEnv *pEnv) : PipelinablePass(pEnv, "UnrollConstantBoundsPass") {}

void UnrollConstantBoundsPass::initialize(){
    PipelinablePass::initialize();
    _command_line->set_description("Fully unrolls CForStatements");
    OptionString *option_loop_label = new OptionString("Loop label",  &loop_label_argument);
    OptionInt *option_unroll_factor_limit = new OptionInt("Unroll factor limit", &unroll_factor_limit_argument);
    OptionList *unroll_constant_bounds_pass_arguments = new OptionList();
    unroll_constant_bounds_pass_arguments->add(option_loop_label);
    unroll_constant_bounds_pass_arguments->add(option_unroll_factor_limit);
    _command_line->add(unroll_constant_bounds_pass_arguments);
}

void UnrollConstantBoundsPass::do_procedure_definition(ProcedureDefinition* proc_def){

  OutputInformation("Starting unroll constant bounds pass") ;

  if (proc_def)
  {
    ucbp_c_for_statement_walker walker(get_suif_env(), loop_label_argument, unroll_factor_limit_argument);
    proc_def->walk(walker);
  }
  OutputInformation("Ending unroll constant bounds pass") ;
}
   
Walker::ApplyStatus ucbp_c_for_statement_walker::operator () (SuifObject *x) {
    SuifEnv *the_env = get_env();
    CForStatement *the_c_for = to<CForStatement>(x);
  
    if(loop_label_argument != String("none"))
    {
      BrickAnnote *loop_label_annote = to<BrickAnnote>(the_c_for->lookup_annote_by_name("c_for_label"));
      // Assert added by Jason on 3/3/2009
      assert(loop_label_annote != NULL) ;
      StringBrick *loop_label_brick = to<StringBrick>(loop_label_annote->get_brick(0));
      String current_loop_label = loop_label_brick->get_value();

      if(current_loop_label != loop_label_argument)
	return Walker::Continue;
    }
    else if(get_c_for_iteration_count(the_c_for) > unroll_factor_limit)
      return Walker::Continue;

    Statement *body = the_c_for->get_body();

    if (body) {
    
        list<VariableSymbol*>* induction_variables = get_c_for_induction_variables(the_c_for);
	int step = get_c_for_step(the_c_for);
	int iteration_count = get_c_for_iteration_count(the_c_for);

        if (iteration_count == 0)
            return Walker::Continue;

	int unroll_count = iteration_count;

        StatementList *replacement = create_statement_list(the_env);

	Statement *the_c_for_lower_bound = to<Statement>(deep_suif_clone(the_c_for->get_before()));
	replacement->append_statement(the_c_for_lower_bound);       

        Statement *original_body = to<Statement>(deep_suif_clone(body));
        replacement->append_statement(original_body);

        for (int k=1; k<unroll_count; k++) 
        {
             Statement *unrolled_body = to<Statement>(deep_suif_clone(original_body));
	     for (list<VariableSymbol*>::iterator iter = induction_variables->begin();
                  iter != induction_variables->end(); iter++) 
             {
                  ucbp_load_variable_expression_walker walker(the_env, (*iter), k*step);
                  unrolled_body->walk(walker);
             }
             replacement->append_statement(unrolled_body); 
        }
	
	the_c_for->get_parent()->replace(the_c_for, replacement);

	return Walker::Truncate;
    }
    return Walker::Continue;
}

Walker::ApplyStatus ucbp_load_variable_expression_walker::operator () (SuifObject *x){

    SuifEnv *the_env = get_env();
    LoadVariableExpression *the_load_var_expr = to<LoadVariableExpression>(x);
    Expression *replacement = NULL;
  
    if (index_variable == the_load_var_expr->get_source()) {  
           
	Expression *src1 = clone_expression(the_load_var_expr);
        Expression *src2 = create_int_constant(the_env, increment);
        replacement = create_binary_expression(the_env, the_load_var_expr->get_result_type(), "add", src1, src2);

        the_load_var_expr->get_parent()->replace(the_load_var_expr, replacement);
    }

    // Probably not the best place to put this, but then again this whole
    //  pass needs to be rewritten.
    BrickAnnote* removedAnnote = 
     to<BrickAnnote>(index_variable->lookup_annote_by_name("RemovedVariable"));
    if (removedAnnote == NULL)
    {
      index_variable->append_annote(create_brick_annote(the_env, 
							"RemovedVariable")) ;
    }

    return Walker::Continue; 
}


