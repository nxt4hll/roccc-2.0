// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#include "common/system_specific.h"
#include <common/suif_copyright.h>

#include <iostream>
#include <iokernel/cast.h>
#include <iokernel/clone_stream.h>
#include <common/i_integer.h>
#include "common/suif_map.h"
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
#include "roccc_utils/list_utils.h"
#include "roccc_utils/IR_utils.h"
#include "roccc_utils/control_flow_utils.h"
#include "roccc_utils/symbol_utils.h"
#include "roccc_utils/loop_utils.h"
#include "roccc_utils/warning_utils.h"
#include "invariant_code_motion_pass.h"

using namespace std;

/**************************** Declarations ************************************/

class icm_c_for_statement_walker: public SelectiveWalker {
public:
  icm_c_for_statement_walker(SuifEnv *the_env)
    : SelectiveWalker(the_env, CForStatement::get_class_name()) {}
  Walker::ApplyStatus operator () (SuifObject *x);
};

class icm_statement_walker: public SelectiveWalker {
public:
  icm_statement_walker(SuifEnv *the_env, Statement *lp, list<VariableSymbol*> *lpinv)
    : SelectiveWalker(the_env, Statement::get_class_name()), the_loop(lp), loop_invariants(lpinv) {}

  Walker::ApplyStatus operator () (SuifObject *x);
private:
  Statement *the_loop;
  list<VariableSymbol*> *loop_invariants;
};

class icm_expression_walker: public SelectiveWalker {
public:
  icm_expression_walker(SuifEnv *the_env, CForStatement *lp, list<VariableSymbol*> *lpinv,  list<VariableSymbol*> *dan)
    : SelectiveWalker(the_env, Expression::get_class_name()), parent_c_for_loop(lp), loop_invariants(lpinv),
							      defined_array_names(dan) {}

  Walker::ApplyStatus operator () (SuifObject *x);
private:
  CForStatement *parent_c_for_loop;
  list<VariableSymbol*> *loop_invariants;
  list<VariableSymbol*> *defined_array_names;
};

suif_map<Expression*, VariableSymbol*> *invariant_temporaries;

/**************************** Implementations ************************************/
InvariantCodeMotionPass::InvariantCodeMotionPass(SuifEnv *pEnv) : PipelinablePass(pEnv, "InvariantCodeMotionPass") {}

void InvariantCodeMotionPass::do_procedure_definition(ProcedureDefinition* proc_def){
  OutputInformation("Invariant code motion pass begins") ;
    if (proc_def){
	invariant_temporaries = new suif_map<Expression*, VariableSymbol*>;

        icm_c_for_statement_walker walker(get_suif_env());
        proc_def->walk(walker);

	delete invariant_temporaries;
    }
    OutputInformation("Invariant code motion pass ends") ;
}

Walker::ApplyStatus icm_c_for_statement_walker::operator () (SuifObject *x) {
    SuifEnv *the_env = get_env();
    CForStatement *the_c_for = to<CForStatement>(x);  
    Statement *body = the_c_for->get_body();

    if (body) {

        list<VariableSymbol*> *induction_variables = get_c_for_induction_variables(the_c_for);
	list<VariableSymbol*> *loop_invariants = get_loop_invariants(body, induction_variables); 
        list<VariableSymbol*> *defined_array_names = collect_array_name_symbols_defined_in_stores(the_c_for);

	invariant_temporaries->clear();

	icm_expression_walker walker2(the_env, the_c_for, loop_invariants, defined_array_names);
	body->walk(walker2);

	delete induction_variables;
	delete loop_invariants;
	delete defined_array_names;
    }

    return Walker::Continue;
}

Walker::ApplyStatus icm_statement_walker::operator () (SuifObject *x){

//    Statement *the_stmt = to<Statement>(x);

    return Walker::Continue; 
}

Walker::ApplyStatus icm_expression_walker::operator () (SuifObject *x){

    SuifEnv *the_env = get_env();
    Expression *the_expr = to<Expression>(x);

    if (is_a<LoadVariableExpression>(x) || is_a<IntConstant>(x) || is_a<SymbolAddressExpression>(x))
	return Walker::Continue;

    list<VariableSymbol*> *source_ops = collect_variable_symbols(the_expr);
    list<VariableSymbol*> *source_array_names = collect_array_name_symbols_used_in_loads(the_expr);

    if (is_items_in_list(source_ops, loop_invariants) && !is_items_in_list(source_array_names, defined_array_names)) {

	VariableSymbol *var = NULL;
        suif_map<Expression*, VariableSymbol*>::iterator iter = invariant_temporaries->begin();
        bool found = 0;
        while(!found && (iter != invariant_temporaries->end())){
           found = is_equal(the_expr, (*iter).first);
           var = (*iter).second;
           iter++;
        }
        if(!found){
           var = new_anonymous_variable(the_env, find_scope(the_expr), retrieve_qualified_type(the_expr->get_result_type()));
	   name_variable(var);
           invariant_temporaries->enter_value(the_expr, var);
        }

        LoadVariableExpression *the_load_var_expr = create_load_variable_expression(the_env, the_expr->get_result_type(), var);	
        replace_expression(the_expr, the_load_var_expr);

	if(!found){
           StoreVariableStatement *store_var_stmt = create_store_variable_statement(the_env, var, the_expr);	
           insert_statement_before(parent_c_for_loop, store_var_stmt);
	}

	delete source_ops;
	delete source_array_names;

        return Walker::Truncate; 
    }
    
    delete source_ops;
    delete source_array_names;

    return Walker::Continue; 
}
