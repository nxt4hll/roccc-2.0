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
#include <cfenodes/cfe.h>
#include <utils/expression_utils.h>

#include "roccc_utils/IR_utils.h"
#include "roccc_utils/control_flow_utils.h"
#include "roccc_utils/bit_vector_data_flow_utils.h"
#include "roccc_utils/warning_utils.h"

#include "mem_dp_boundary_mark_pass.h"

using namespace std;

// THIS PASS NEEDS TO BE CALLED AFTER THE SCALAR REPLACEMENT PASS AND BEFORE ARRAY FEEDBACK ANALYSIS PASS.

/**************************** Declarations ************************************/

class mdbm_c_for_statement_walker: public SelectiveWalker {
public:
  mdbm_c_for_statement_walker(SuifEnv *the_env)
    : SelectiveWalker(the_env, CForStatement::get_class_name()) {}
  Walker::ApplyStatus operator () (SuifObject *x);
};

/**************************** Implementations ************************************/
MEM_DP_BoundaryMarkPass::MEM_DP_BoundaryMarkPass(SuifEnv *pEnv) : PipelinablePass(pEnv, "MEM_DP_BoundaryMarkPass"){}

void MEM_DP_BoundaryMarkPass::do_procedure_definition(ProcedureDefinition* proc_def)
{
  OutputInformation("Memory/Datapath marking pass begins") ;
  if (proc_def)
  {
    mdbm_c_for_statement_walker walker(get_suif_env());
    proc_def->walk(walker);
  }
  OutputInformation("Memory/Datapath marking pass ends") ;
}

Walker::ApplyStatus mdbm_c_for_statement_walker::operator () (SuifObject *x) {
    SuifEnv *env = get_env();
    CForStatement *c_for_stmt = to<CForStatement>(x);
                
    //    if(!is_stmt_within_begin_end_hw_marks(c_for_stmt))
    //   return Walker::Continue;

    Statement *body = c_for_stmt->get_body();
             
    if (body){

        Iter<CForStatement> iter_c_for = object_iterator<CForStatement>(body);
        if(iter_c_for.is_valid())
           return Walker::Continue;
             
	StatementList *stmt_list_body = NULL;

	if(is_a<StatementList>(body))
           stmt_list_body = to<StatementList>(body); 
	else{
           stmt_list_body = create_statement_list(env);
	   c_for_stmt->set_body(0);
	   stmt_list_body->append_statement(body);
	   c_for_stmt->set_body(stmt_list_body);
        }

        Iter<Statement*> iter = stmt_list_body->get_statement_iterator();
        for( ; iter.is_valid(); iter.next()){
            Statement *child_stmt = iter.current();
            if(is_a<StoreVariableStatement>(child_stmt)){
               Iter<LoadExpression> iter2 = object_iterator<LoadExpression>(child_stmt);
               if(!iter2.is_valid())
                  break;
            }else break;
        }

        MarkStatement *end_of_mem_reads_mark = create_mark_statement(env);
	if(iter.is_valid())
           insert_statement_before(iter.current(), end_of_mem_reads_mark);
        else
           stmt_list_body->insert_statement(0, end_of_mem_reads_mark);
        BrickAnnote *ba = create_brick_annote(env, "end_of_mem_reads");
        ba->append_brick(create_suif_object_brick(env, end_of_mem_reads_mark));
        c_for_stmt->append_annote(ba);
  
        for( ; iter.is_valid(); iter.next()){
            Statement *child_stmt = iter.current();
            if(is_a<StoreStatement>(child_stmt))
               break;
        }

        MarkStatement *beg_of_mem_writes_mark = create_mark_statement(env);
	if(iter.is_valid())
           insert_statement_before(iter.current(), beg_of_mem_writes_mark);
        else
           stmt_list_body->append_statement(beg_of_mem_writes_mark);
        ba = create_brick_annote(env, "beg_of_mem_writes");
        ba->append_brick(create_suif_object_brick(env, beg_of_mem_writes_mark));
        c_for_stmt->append_annote(ba);
    }        
    
    return Walker::Continue;
}               
                                     
