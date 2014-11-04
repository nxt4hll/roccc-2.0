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
#include <cfenodes/cfe.h>
#include <cfenodes/cfe_factory.h>
#include "roccc_utils/loop_utils.h"
#include "peel_pass.h"

/**************************** Declarations ************************************/

class pp_c_for_statement_walker: public SelectiveWalker {
public:
  pp_c_for_statement_walker(SuifEnv *the_env, String lplbl_arg, int fpc, int lpc)
    : SelectiveWalker(the_env, CForStatement::get_class_name()), first_iter_peel_count(fpc), 
						 last_iter_peel_count(lpc) { loop_label_argument = lplbl_arg; }
  Walker::ApplyStatus operator () (SuifObject *x);
private:
  String loop_label_argument;
  int first_iter_peel_count;
  int last_iter_peel_count;
};

/**************************** Implementations ************************************/
PeelPass::PeelPass(SuifEnv *pEnv) : PipelinablePass(pEnv, "PeelPass") {}

void PeelPass::initialize(){ 
    PipelinablePass::initialize();  
    _command_line->set_description("Peels the first or last iterations of a CForStatement"); 
    OptionString *option_loop_label = new OptionString("Loop label",  &loop_label_argument);
    OptionInt *option_first_iter_peel_count = new OptionInt("First iter peel count", &first_iter_peel_count_argument);
    OptionInt *option_last_iter_peel_count = new OptionInt("Last iter peel count", &last_iter_peel_count_argument);
    OptionList *peel_pass_arguments = new OptionList();
    peel_pass_arguments->add(option_loop_label);
    peel_pass_arguments->add(option_first_iter_peel_count);
    peel_pass_arguments->add(option_last_iter_peel_count);
    _command_line->add(peel_pass_arguments);
}

void PeelPass::do_procedure_definition(ProcedureDefinition* proc_def){
    if (proc_def){
        pp_c_for_statement_walker walker(get_suif_env(), loop_label_argument, first_iter_peel_count_argument, 
									      last_iter_peel_count_argument);
        proc_def->walk(walker);
    }
}
   
Walker::ApplyStatus pp_c_for_statement_walker::operator () (SuifObject *x) {
    SuifEnv *the_env = get_env();
    CForStatement *the_c_for = to<CForStatement>(x);  

    BrickAnnote *loop_label_annote = to<BrickAnnote>(the_c_for->lookup_annote_by_name("c_for_label"));
    StringBrick *loop_label_brick = to<StringBrick>(loop_label_annote->get_brick(0));
    String current_loop_label = loop_label_brick->get_value();

    if (current_loop_label != loop_label_argument)
        return Walker::Continue;
  
    Statement *body = the_c_for->get_body();
    if (body) {
    
	if(first_iter_peel_count > 0){

	   StatementList *peeled_body = create_statement_list(the_env);
           Statement *the_c_for_lower_bound = to<Statement>(deep_suif_clone(the_c_for->get_before()));
           peeled_body->append_statement(the_c_for_lower_bound);       

           for (int k=1; k<=first_iter_peel_count; k++) {
               StatementList *original_body = to<StatementList>(deep_suif_clone(body));
               peeled_body->append_statement(original_body);       
               Statement *the_c_for_step = to<Statement>(deep_suif_clone(the_c_for->get_step()));
               peeled_body->append_statement(the_c_for_step);       
           }

	   insert_statement_before(the_c_for, peeled_body);
	   int new_c_for_lower_bound = get_c_for_lower_bound(the_c_for) + 
				       get_c_for_step(the_c_for) * first_iter_peel_count;
	   set_c_for_lower_bound(the_c_for, new_c_for_lower_bound);
	}

	if(last_iter_peel_count > 0){

	   StatementList *peeled_body = create_statement_list(the_env);

           for (int k=1; k<=last_iter_peel_count; k++) {
               StatementList *original_body = to<StatementList>(deep_suif_clone(body));
               peeled_body->append_statement(original_body);       
               Statement *the_c_for_step = to<Statement>(deep_suif_clone(the_c_for->get_step()));
               peeled_body->append_statement(the_c_for_step);       
           }

	   insert_statement_after(the_c_for, peeled_body);
	   int new_c_for_upper_bound = get_c_for_upper_bound(the_c_for) - 
				       get_c_for_step(the_c_for) * last_iter_peel_count;
	   set_c_for_upper_bound(the_c_for, new_c_for_upper_bound);
	}

    }

    return Walker::Continue;
}

