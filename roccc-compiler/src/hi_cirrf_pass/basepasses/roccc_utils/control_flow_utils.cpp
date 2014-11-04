// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#include "common/system_specific.h"
#include <common/suif_copyright.h>
#include <common/suif_list.h>

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
#include "annote_utils.h"
#include "bit_vector_annote_utils.h"
#include "list_utils.h"
#include "IR_utils.h"
#include "print_utils.h"
#include "control_flow_utils.h"

/**************************** Declerations ************************************/

/************************** Implementations ***********************************/

// computes intersections of all the predecessors' outs, then assigns the result to in
// computes the forward must for available expressions
void intersect_predecessor_annotes(BrickAnnote *in, BrickAnnote *predecessors){

        Iter<SuifBrick*> iter = predecessors->get_brick_iterator();

        if(iter.is_valid()){
           SuifObjectBrick *sob = to<SuifObjectBrick>(iter.current());
           Statement *pred = to<Statement>(sob->get_object());

           BrickAnnote *pred_out = to<BrickAnnote>(pred->lookup_annote_by_name("out_available_exprs"));
           union_annotes(in, pred_out);

           for(iter.next(); iter.is_valid(); iter.next()){
               sob = to<SuifObjectBrick>(iter.current());
               pred = to<Statement>(sob->get_object());

               pred_out = to<BrickAnnote>(pred->lookup_annote_by_name("out_available_exprs"));
               intersect_annotes(in, pred_out);
           }
        }
}

// computes unions of all the predecessors out_stmts, then assigns the result to in
// computes the forward may for reaching defs
void union_predecessor_annotes(BrickAnnote *in, BrickAnnote *predecessors){

        Iter<SuifBrick*> iter = predecessors->get_brick_iterator();
	
	if(!iter.is_valid())
	   return;

        SuifObjectBrick *sob = to<SuifObjectBrick>(iter.current());
        Statement *pred = to<Statement>(sob->get_object());

        BrickAnnote *pred_out = to<BrickAnnote>(pred->lookup_annote_by_name("out_stmts"));
        bv_copy_annotes(in, pred_out);
	
        for(iter.next(); iter.is_valid(); iter.next()){
	
            sob = to<SuifObjectBrick>(iter.current());
            pred = to<Statement>(sob->get_object());

            pred_out = to<BrickAnnote>(pred->lookup_annote_by_name("out_stmts"));
            bv_union_annotes(in, pred_out);
        }
}

void union_predecessor_annotes(BrickAnnote *in, BrickAnnote *predecessors, String annote_name){

        for(Iter<SuifBrick*> iter = predecessors->get_brick_iterator();
	    iter.is_valid(); iter.next()){
	
            SuifObjectBrick *sob = to<SuifObjectBrick>(iter.current());
            Statement *pred = to<Statement>(sob->get_object());

            BrickAnnote *pred_out = to<BrickAnnote>(pred->lookup_annote_by_name(annote_name));
            union_annotes(in, pred_out);
        }
}

