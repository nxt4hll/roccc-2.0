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
#include "availability_chain_builder_pass.h"

using namespace std;

// THIS PASS ANNOTES THE BINARY EXPRESSIONS w/ OTHER IDENTICAL BINARY EXPRESSIONS
// WHERE THE FIRST BINARY EXPRESSION IS AVAILABLE IN THE CONTEXT OF THE SECOND

// THE PASS ASSUMES THAT ALL VARIABLES ARE UNIQUELY NAMED THROUGHOUT THE ENTIRE PROGRAM.
// IN OTHER WORDS, NO VARIABLE NAME IS DEFINED IN MORE THAN ONE PLACE THROUGHOUT THE PROGRAM
// THE RESULTS ARE REPORTED AS ANNOTATIONS TO THE SUIF IR.
        
/**************************** Declarations ************************************/

class annote_binary_expressions: public SelectiveWalker {
public:
  annote_binary_expressions(SuifEnv *env)
    :SelectiveWalker(env, BinaryExpression::get_class_name()) {}
  Walker::ApplyStatus operator () (SuifObject *x);
};
           
class annote_binary_expressions2: public SelectiveWalker {
public:
  annote_binary_expressions2(SuifEnv *env)
    :SelectiveWalker(env, BinaryExpression::get_class_name()) {}
  Walker::ApplyStatus operator () (SuifObject *x);
};
 
/**************************** Implementations ************************************/
AvailabilityChainBuilderPass::AvailabilityChainBuilderPass(SuifEnv *pEnv) :
					 PipelinablePass(pEnv, "AvailabilityChainBuilderPass") {}

void AvailabilityChainBuilderPass::do_procedure_definition(ProcedureDefinition* proc_def)
{
  OutputInformation("Availablity chain builder begins") ;
  if (proc_def)
  { 
    annote_binary_expressions walker(get_suif_env());
    proc_def->walk(walker);   
    annote_binary_expressions2 walker2(get_suif_env());
    proc_def->walk(walker2);
  }
  OutputInformation("Availablity chain builder ends") ;
}

Walker::ApplyStatus annote_binary_expressions::operator () (SuifObject *x) {
        SuifEnv *env = get_env();
        BinaryExpression *binary_expr = to<BinaryExpression>(x);
              
        BrickAnnote *reached_available_exprs = create_brick_annote(env, "reached_available_exprs");
        binary_expr->append_annote(reached_available_exprs);
           
        return Walker::Continue;
}       
           
Walker::ApplyStatus annote_binary_expressions2::operator () (SuifObject *x) {
        SuifEnv *env = get_env();
        BinaryExpression *binary_expr = to<BinaryExpression>(x);
           
        Statement *parent_stmt = get_expression_owner(binary_expr);
        BrickAnnote *in_available_exprs = to<BrickAnnote>(parent_stmt->lookup_annote_by_name("in_available_exprs"));
           
        for(Iter<SuifBrick*> iter = in_available_exprs->get_brick_iterator();
            iter.is_valid(); iter.next()){
           
            SuifObjectBrick *sob = to<SuifObjectBrick>(iter.current());
            BinaryExpression *incoming_expr = to<BinaryExpression>(sob->get_object());
        
            if(is_equal(binary_expr, incoming_expr)){
               BrickAnnote *reached_available_exprs = to<BrickAnnote>(incoming_expr->lookup_annote_by_name("reached_available_exprs"));
               reached_available_exprs->append_brick(create_suif_object_brick(env, binary_expr));
            }
        }
              
        return Walker::Continue;
}       

