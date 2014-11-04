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
#include "roccc_utils/IR_utils.h"
#include "roccc_utils/warning_utils.h"
#include "cse_pass.h"

using namespace std;

/**************************** Declarations ************************************/

class cse_binary_expression_walker: public SelectiveWalker {
public:
  cse_binary_expression_walker(SuifEnv *env)
    :SelectiveWalker(env, BinaryExpression::get_class_name()) {}
  Walker::ApplyStatus operator () (SuifObject *x);
};

/**************************** Implementations ************************************/
CommonSubExpressionEliminationPass::CommonSubExpressionEliminationPass(SuifEnv *pEnv) :
                                                        PipelinablePass(pEnv, "CommonSubExpressionEliminationPass") {}

void CommonSubExpressionEliminationPass::do_procedure_definition(ProcedureDefinition* proc_def)
{
  OutputInformation("Common subexpression elimination pass begins") ;
  if (proc_def)
  {
    cse_binary_expression_walker walker(get_suif_env());
    proc_def->walk(walker);
  }
  OutputInformation("Common subexpression elimination pass ends") ;
}

Walker::ApplyStatus cse_binary_expression_walker::operator () (SuifObject *x) {
        SuifEnv *env = get_env();
        BinaryExpression *binary_expr = to<BinaryExpression>(x);

        if(is_a<CForStatement>(get_expression_owner(binary_expr)))
           return Walker::Continue;

        ExecutionObject *temp_obj = to<ExecutionObject>(binary_expr->get_parent());
        while(temp_obj != get_expression_owner(binary_expr))
           if(is_a<ArrayReferenceExpression>(temp_obj) || is_a<MultiDimArrayExpression>(temp_obj))
              return Walker::Continue;
           else temp_obj = to<ExecutionObject>(temp_obj->get_parent());

        BrickAnnote *reached_available_exprs = to<BrickAnnote>(binary_expr->lookup_annote_by_name("reached_available_exprs"));

        Iter<SuifBrick*> iter = reached_available_exprs->get_brick_iterator();

        if(!iter.is_valid())
           return Walker::Continue;

        VariableSymbol *dest_var = new_unique_variable(env, find_scope(binary_expr),
                                                       retrieve_qualified_type(binary_expr->get_result_type()));
        StoreVariableStatement *common_expr_store_stmt = create_store_variable_statement(env, dest_var,
                                                       to<Expression>(binary_expr->deep_clone()));

        insert_statement_before(get_expression_owner(binary_expr), common_expr_store_stmt);
        replace_expression(binary_expr, create_var_use(dest_var));

        for( ; iter.is_valid(); iter.next()){

            SuifObjectBrick *sob = to<SuifObjectBrick>(iter.current());
            BinaryExpression *reached_expr = to<BinaryExpression>(sob->get_object());

            if(is_equal(binary_expr, reached_expr))
               replace_expression(reached_expr, create_var_use(dest_var));
        }

        return Walker::Continue;
}
