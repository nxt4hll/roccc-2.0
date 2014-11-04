// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#include "common/system_specific.h"
#include <common/suif_copyright.h>

#include <iostream>
#include <iokernel/cast.h>
#include <iokernel/clone_stream.h>
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
#include "common/suif_list.h"
#include "remove_loop_label_loc_stmts_pass.h"


/**************************** Declarations ************************************/

/**************************** Implementations ************************************/

RemoveLoopLabelLocStmtsPass::RemoveLoopLabelLocStmtsPass( SuifEnv* suif_env) : 
				PipelinablePass( suif_env, "RemoveLoopLabelLocStmtsPass" ) {}


void RemoveLoopLabelLocStmtsPass::do_procedure_definition(ProcedureDefinition *proc_def){
    if(proc_def){

       list<LabelLocationStatement*>* label_loc_stmts = collect_objects<LabelLocationStatement>(proc_def->get_body());

       while(label_loc_stmts->size() > 0){	
	   remove_statement(label_loc_stmts->front());
	   label_loc_stmts->pop_front();
       }

       delete label_loc_stmts;
    }
}

