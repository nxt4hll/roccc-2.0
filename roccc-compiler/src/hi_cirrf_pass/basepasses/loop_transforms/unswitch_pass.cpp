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
#include "roccc_utils/list_utils.h"
#include "roccc_utils/symbol_utils.h"
#include "roccc_utils/loop_utils.h"
#include "unswitch_pass.h"

// STATEMENT LIST FLATTENING PASS SHOULD BE EXECUTED BEFORE THIS PASS.

/**************************** Declarations ************************************/

class up_c_for_statement_walker: public SelectiveWalker {
public:
  up_c_for_statement_walker(SuifEnv *the_env, String lplbl_arg)
    : SelectiveWalker(the_env, CForStatement::get_class_name()) { loop_label_argument = lplbl_arg; }
  Walker::ApplyStatus operator () (SuifObject *x);
private:
  list<VariableSymbol*> *loop_invariants;
  String loop_label_argument;
};

/**************************** Implementations ************************************/
UnswitchPass::UnswitchPass(SuifEnv *pEnv) : PipelinablePass(pEnv, "UnswitchPass") {}

void UnswitchPass::initialize(){
    PipelinablePass::initialize();
    _command_line->set_description("Hoists a loop independent if statement, which is the only statement in the loop body");
    OptionString *option_loop_label = new OptionString("Loop label",  &loop_label_argument);
    OptionList *unswitch_pass_arguments = new OptionList();
    unswitch_pass_arguments->add(option_loop_label);
    _command_line->add(unswitch_pass_arguments);
}

void UnswitchPass::do_procedure_definition(ProcedureDefinition* proc_def){
    if (proc_def){
        up_c_for_statement_walker walker(get_suif_env(), loop_label_argument);
        proc_def->walk(walker);
    }
}
   
Walker::ApplyStatus up_c_for_statement_walker::operator () (SuifObject *x) {
    CForStatement *the_c_for = to<CForStatement>(x);  
    Statement *body = to<Statement>(the_c_for->get_body());

    BrickAnnote *loop_label_annote = to<BrickAnnote>(the_c_for->lookup_annote_by_name("c_for_label"));
    StringBrick *loop_label_brick = to<StringBrick>(loop_label_annote->get_brick(0));
    String current_loop_label = loop_label_brick->get_value();

    if (current_loop_label != loop_label_argument)
        return Walker::Continue;

    if (!is_a<IfStatement>(body))
	return Walker::Continue;

    list<VariableSymbol*> *induction_variables = get_c_for_induction_variables(the_c_for);
    loop_invariants = get_loop_invariants(body, induction_variables);    

    IfStatement *loop_body = to<IfStatement>(body);
    list<VariableSymbol*> *source_ops = collect_variable_symbols(loop_body->get_condition());

    IfStatement *replacement = NULL;
    if (!source_ops->empty() && is_items_in_list(source_ops, loop_invariants)) {
	replacement = to<IfStatement>(deep_suif_clone(loop_body));
        CForStatement *c_for_then_stmt = to<CForStatement>(deep_suif_clone(the_c_for));
        c_for_then_stmt->set_body(to<Statement>(deep_suif_clone(loop_body->get_then_part())));
	replacement->set_then_part(c_for_then_stmt);
        if(loop_body->get_else_part()){
           CForStatement *c_for_else_stmt = to<CForStatement>(deep_suif_clone(the_c_for));
           c_for_else_stmt->set_body(to<Statement>(deep_suif_clone(loop_body->get_else_part())));
	   replacement->set_else_part(c_for_else_stmt);
	}
    }

    replace_statement(the_c_for, replacement);
    set_address(replacement);
    return Walker::Replaced;

}

