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

#include "roccc_utils/warning_utils.h"

#include "dowhile_to_while_transform_pass.h"

/**************************** Declarations ************************************/

class dw2wtp_do_while_statement_walker: public SelectiveWalker {
public:
  dw2wtp_do_while_statement_walker(SuifEnv *env)
    :SelectiveWalker(env, DoWhileStatement::get_class_name()) {}
  Walker::ApplyStatus operator () (SuifObject *x); 
};

/**************************** Implementations ************************************/
DoWhileToWhileTransformPass::DoWhileToWhileTransformPass(SuifEnv *pEnv) : 
							PipelinablePass(pEnv, "DoWhileToWhileTransformPass") {}

void DoWhileToWhileTransformPass::do_procedure_definition(ProcedureDefinition* proc_def)
{
  OutputInformation("Do...while to while loop transformation begins") ;
  if (proc_def)
  {
    dw2wtp_do_while_statement_walker walker(get_suif_env());
    proc_def->walk(walker);
  }
  OutputInformation("Do...while to while loop transformation ends") ;
}

Walker::ApplyStatus dw2wtp_do_while_statement_walker::operator () (SuifObject *x) {
	SuifEnv *env = get_env();
	DoWhileStatement *do_while_stmt = to<DoWhileStatement>(x);

	Expression *do_while_condition = do_while_stmt->get_condition();
	do_while_stmt->set_condition(0);
	Statement *do_while_body = do_while_stmt->get_body();
	do_while_stmt->set_body(0);

	IfStatement *first_iteration = create_if_statement(env, 
						to<Expression>(deep_suif_clone(do_while_condition)), 
						to<Statement>(deep_suif_clone(do_while_body)));
	WhileStatement *remaining_iterations = create_while_statement(env, do_while_condition, do_while_body);

	StatementList *replacement = create_statement_list(env);
	replacement->append_statement(first_iteration);
	replacement->append_statement(remaining_iterations);

	do_while_stmt->get_parent()->replace(do_while_stmt, replacement);
	set_address(replacement);
    	return Walker::Replaced;
}



