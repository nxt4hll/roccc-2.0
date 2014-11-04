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
#include <cfenodes/cfe.h>
#include "utils/expression_utils.h"
#include "roccc_utils/warning_utils.h"
#include "hw_sw_boundary_mark_pass.h"

/**************************** Declarations ************************************/

class hsbm_call_statement_walker: public SelectiveWalker {
public:
  hsbm_call_statement_walker(SuifEnv *env)
    :SelectiveWalker(env, CallStatement::get_class_name()) {}
  Walker::ApplyStatus operator () (SuifObject *x); 
};

/**************************** Implementations ************************************/
HW_SW_BoundaryMarkPass::HW_SW_BoundaryMarkPass(SuifEnv *pEnv) : PipelinablePass(pEnv, "HW_SW_BoundaryMarkPass") {}

void HW_SW_BoundaryMarkPass::do_procedure_definition(ProcedureDefinition* proc_def)
{
  OutputInformation("Determination of Hardware/Software boundary pass begins");
    if (proc_def){
	SuifEnv *env = get_suif_env();

        if(proc_def->lookup_annote_by_name("begin_hw"))
           delete (to<BrickAnnote>(proc_def->remove_annote_by_name("begin_hw")));
        if(proc_def->lookup_annote_by_name("end_hw"))
           delete (to<BrickAnnote>(proc_def->remove_annote_by_name("end_hw")));

        BrickAnnote *ba = create_brick_annote(env, "begin_hw");
        proc_def->append_annote(ba);
        ba = create_brick_annote(env, "end_hw");
        proc_def->append_annote(ba);

	hsbm_call_statement_walker walker(get_suif_env());
	proc_def->walk(walker);
    }
  OutputInformation("Determination of Hardware/Software boundary pass ends");
}

Walker::ApplyStatus hsbm_call_statement_walker::operator () (SuifObject *x) {
	SuifEnv *env = get_env();
	CallStatement *call_stmt = to<CallStatement>(x);

	SymbolAddressExpression *callee_address = to<SymbolAddressExpression>(call_stmt->get_callee_address());
	ProcedureSymbol *proc_symbol = to<ProcedureSymbol>(callee_address->get_addressed_symbol());
	String called_proc_name = proc_symbol->get_name();

	MarkStatement *mark_stmt = NULL;

        ProcedureDefinition* proc_def = get_procedure_definition(call_stmt);
           
	if(called_proc_name == String("begin_hw")){
	   mark_stmt = create_mark_statement(env);
           BrickAnnote *ba = to<BrickAnnote>(proc_def->lookup_annote_by_name("begin_hw"));
	   ba->append_brick(create_suif_object_brick(env, mark_stmt));
	}else if(called_proc_name == String("end_hw")){
	   mark_stmt = create_mark_statement(env);
           BrickAnnote *ba = to<BrickAnnote>(proc_def->lookup_annote_by_name("end_hw"));
	   ba->append_brick(create_suif_object_brick(env, mark_stmt));
	}

	if(mark_stmt){
	   call_stmt->get_parent()->replace(call_stmt, mark_stmt);
	   set_address(mark_stmt);
    	   return Walker::Replaced;
	}

        return Walker::Continue;
}
