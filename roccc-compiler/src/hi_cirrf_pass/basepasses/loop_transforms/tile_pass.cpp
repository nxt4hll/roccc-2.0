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
#include "roccc_utils/IR_utils.h"
#include "tile_pass.h"
#include <typebuilder/type_builder.h>

// THIS TILE PASS GENERATES A 2D TILED VERSION OF A GIVEN 2D LOOP

/**************************** Declarations ************************************/

class tp_c_for_statement_walker: public SelectiveWalker {
public:
  tp_c_for_statement_walker(SuifEnv *the_env, String loop_lbl1, String loop_lbl2, int ts1, int ts2)
    : SelectiveWalker(the_env, CForStatement::get_class_name()), loop_label_argument1(loop_lbl1), 
							    	 loop_label_argument2(loop_lbl2),
								 tile_size_argument1(ts1),
								 tile_size_argument2(ts2) {}
  Walker::ApplyStatus operator () (SuifObject *x);
private:
  String loop_label_argument1;
  String loop_label_argument2;
  int tile_size_argument1;
  int tile_size_argument2;
};

class tp_load_variable_expression_walker: public SelectiveWalker {
public:
  tp_load_variable_expression_walker(SuifEnv *the_env, VariableSymbol* iv, VariableSymbol* tv)
    : SelectiveWalker(the_env, LoadVariableExpression::get_class_name()), index_variable(iv), tile_index_variable(tv) {}

  Walker::ApplyStatus operator () (SuifObject *x);
private:
  VariableSymbol* index_variable;
  VariableSymbol* tile_index_variable;
};

CForStatement *tp_first_loop;
CForStatement *tp_second_loop;

/**************************** Implementations ************************************/
TilePass::TilePass(SuifEnv *pEnv) : PipelinablePass(pEnv, "TilePass") {}

void TilePass::initialize(){
    PipelinablePass::initialize();
    _command_line->set_description("Tiles CForStatements");
    OptionString *option_loop_label1 = new OptionString("Loop label1",  &loop_label_argument1);
    OptionString *option_loop_label2 = new OptionString("Loop label2",  &loop_label_argument2);
    OptionInt *option_tile_size1 = new OptionInt("Tile Size1",  &tile_size_argument1);
    OptionInt *option_tile_size2 = new OptionInt("Tile Size2",  &tile_size_argument2);
    OptionList *tile_pass_arguments = new OptionList();
    tile_pass_arguments->add(option_loop_label1);
    tile_pass_arguments->add(option_loop_label2);
    tile_pass_arguments->add(option_tile_size1);
    tile_pass_arguments->add(option_tile_size2);
    _command_line->add(tile_pass_arguments);
}

void TilePass::do_procedure_definition(ProcedureDefinition* proc_def){
    if (proc_def){
        tp_c_for_statement_walker walker(get_suif_env(), loop_label_argument1, loop_label_argument2,
							 tile_size_argument1, tile_size_argument2);
        proc_def->walk(walker);
    }
}

CForStatement* make_tile(SuifEnv *env, VariableSymbol* loop_counter1, VariableSymbol* loop_counter2, int upper_bound1, int upper_bound2, 
			 int step_size1, int step_size2, Statement* body){

    TypeBuilder *type_builder = get_type_builder(env);
	
    Statement *before2_stmt = create_store_variable_statement(env, loop_counter2, create_int_constant(env, IInteger(0)));
    Expression *test2_expr = create_binary_expression(env, type_builder->get_boolean_type(), "is_less_than", create_var_use(loop_counter2), 
						      create_int_constant(env, IInteger(upper_bound2)));
    Expression *step2_expr = create_binary_expression(env, loop_counter2->get_type()->get_base_type(), "add", 
						      create_var_use(loop_counter2), create_int_constant(env, IInteger(step_size2)));
    Statement *step2_stmt = create_store_variable_statement(env, loop_counter2, step2_expr);

    CForStatement *c_for_stmt2 = create_c_for_statement(env, before2_stmt, test2_expr, step2_stmt, body);


    Statement *before_stmt = create_store_variable_statement(env, loop_counter1, create_int_constant(env, IInteger(0)));
    Expression *test_expr = create_binary_expression(env, type_builder->get_boolean_type(), "is_less_than", create_var_use(loop_counter1), 
						      create_int_constant(env, IInteger(upper_bound1)));
    Expression *step_expr = create_binary_expression(env, loop_counter1->get_type()->get_base_type(), "add", 
						     create_var_use(loop_counter1), create_int_constant(env, IInteger(step_size1)));
    Statement *step_stmt = create_store_variable_statement(env, loop_counter1, step_expr);

    CForStatement *tile = create_c_for_statement(env, before_stmt, test_expr, step_stmt, c_for_stmt2);
    return tile;
}
   
Walker::ApplyStatus tp_c_for_statement_walker::operator () (SuifObject *x) {
    SuifEnv *the_env = get_env();
    CForStatement *c_for_stmt = to<CForStatement>(x);  

    if(!is_stmt_within_begin_end_hw_marks(c_for_stmt))
       return Walker::Continue;

    BrickAnnote *loop_label_annote = to<BrickAnnote>(c_for_stmt->lookup_annote_by_name("c_for_label"));
    String current_loop_label = (to<StringBrick>(loop_label_annote->get_brick(0)))->get_value();

    if(current_loop_label != loop_label_argument1){
       if(current_loop_label != loop_label_argument2)
          return Walker::Continue;
       tp_second_loop = c_for_stmt;
    }else tp_first_loop = c_for_stmt;

    if(tp_first_loop == NULL || tp_second_loop == NULL)
       return Walker::Continue;

    Statement *body = tp_first_loop->get_body();
    if (body) {

        VariableSymbol* loop_counter1 = get_c_for_basic_induction_variable(tp_first_loop);
        VariableSymbol* loop_counter2 = get_c_for_basic_induction_variable(tp_second_loop);

        VariableSymbol* tile_loop_counter1 = new_anonymous_variable(the_env, find_scope(tp_first_loop),
									       loop_counter1->get_type());
	name_variable(tile_loop_counter1);
        VariableSymbol* tile_loop_counter2 = new_anonymous_variable(the_env, find_scope(tp_second_loop),
										loop_counter2->get_type());
	name_variable(tile_loop_counter2);

	// step sizes of the tiles in horizontal and vertical direction are the step size of the original loops in those directions
	int tile_step_size1 = get_c_for_step(tp_first_loop);  	
	int tile_step_size2 = get_c_for_step(tp_second_loop); 

	Statement* tile_body = tp_second_loop->get_body();
	tp_second_loop->set_body(0);

	CForStatement* tile = make_tile(the_env, tile_loop_counter1, tile_loop_counter2, tile_size_argument1,
					tile_size_argument2, tile_step_size1, tile_step_size2, tile_body);
	tp_second_loop->set_body(tile);

	set_c_for_step(tp_first_loop, tile_size_argument1);
	set_c_for_step(tp_second_loop, tile_size_argument2);

	tp_load_variable_expression_walker walker(the_env, loop_counter1, tile_loop_counter1);
        tile_body->walk(walker);
        tp_load_variable_expression_walker walker2(the_env, loop_counter2, tile_loop_counter2);
        tile_body->walk(walker2);

    }

    return Walker::Stop;
}

Walker::ApplyStatus tp_load_variable_expression_walker::operator () (SuifObject *x){

    SuifEnv *the_env = get_env();
    LoadVariableExpression *the_load_var_expr = to<LoadVariableExpression>(x);
    Expression *replacement = NULL;
  
    if (index_variable == the_load_var_expr->get_source()) {  
           
	Expression *src1 = clone_expression(the_load_var_expr);
        Expression *src2 = create_load_variable_expression(the_env, the_load_var_expr->get_result_type() , tile_index_variable);
        replacement = create_binary_expression(the_env, the_load_var_expr->get_result_type(), "add", src1, src2);

        the_load_var_expr->get_parent()->replace(the_load_var_expr, replacement);
    }

    return Walker::Continue; 
}



