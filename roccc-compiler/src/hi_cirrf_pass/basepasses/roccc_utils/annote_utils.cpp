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

/**************************** Declerations ************************************/


/************************** Implementations ***********************************/

void create_n_append_annote(ExecutionObject *eo, String name){
	if(eo->lookup_annote_by_name(name)){
	   empty(to<BrickAnnote>(eo->lookup_annote_by_name(name)));
	   delete (eo->remove_annote_by_name(name));
	}
	SuifEnv *env = eo->get_suif_env();
	BrickAnnote *ba = create_brick_annote(env, name);
	eo->append_annote(ba);
}

void empty(BrickAnnote *ba){
	while(ba->get_brick_count() > 0)
	   delete (ba->remove_brick(0));
}

BrickAnnote* clone(BrickAnnote *ba){
	SuifEnv* env = ba->get_suif_env();
        BrickAnnote *clone = create_brick_annote(env, ba->get_name() + "clone");

	for(Iter<SuifBrick*> iter = ba->get_brick_iterator();
	    iter.is_valid(); iter.next()){

	    SuifObjectBrick *sob = to<SuifObjectBrick>(iter.current());
	    clone->append_brick(create_suif_object_brick(env, sob->get_object()));
	}
	
	return clone;
}

bool search(BrickAnnote *ba, SuifObject *so){
	for(Iter<SuifBrick*> iter = ba->get_brick_iterator();
	    iter.is_valid(); iter.next()){

	    SuifObjectBrick *sob = to<SuifObjectBrick>(iter.current());
	    if(sob->get_object() == so)
	       return 1;	
	}
	return 0;
}

bool search(BrickAnnote *ba, String s){
	for(Iter<SuifBrick*> iter = ba->get_brick_iterator();
	    iter.is_valid(); iter.next()){

	    StringBrick *sb = to<StringBrick>(iter.current());
	    if(sb->get_value() == s)
	       return 1;	
	}
	return 0;
}

void remove_brick(BrickAnnote *ba, SuifObject *so){
	int i = 0;
	while(i < ba->get_brick_count()){
	    SuifObjectBrick *sob = to<SuifObjectBrick>(ba->get_brick(i));
	    if(sob->get_object() == so)
	       delete (ba->remove_brick(i));
	    else i++;
	}
}

// empties A and copies the contents of B into A 
void copy_annotes(BrickAnnote *A, BrickAnnote *B){

	empty(A);

	for(Iter<SuifBrick*> iter = B->get_brick_iterator();
	    iter.is_valid(); iter.next()){

	    SuifObjectBrick *sob = to<SuifObjectBrick>(iter.current());
	    A->append_brick(create_suif_object_brick(sob->get_suif_env(), sob->get_object()));
	}
	
}

// computes a union b, then assigns the result to A
void union_annotes(BrickAnnote *A, BrickAnnote *B){
	bool found;

        for(Iter<SuifBrick*> iter = B->get_brick_iterator();
            iter.is_valid(); iter.next()){

	    SuifObjectBrick *sob = to<SuifObjectBrick>(iter.current());
	    found = search(A, sob->get_object());
	    if(!found)
	       A->append_brick(create_suif_object_brick(sob->get_suif_env(), sob->get_object()));
	}
}

// computes a intersect b, then assigns the result to A
void intersect_annotes(BrickAnnote *A, BrickAnnote *B){
	bool found;
	int i = 0;
	while(i < A->get_brick_count()){
	    SuifObjectBrick *sob = to<SuifObjectBrick>(A->get_brick(i));
	    found = search(B, sob->get_object());
	    if(!found)
	       delete (A->remove_brick(i));
	    else i++;
	}
}

// computes a-b, then assigns the result to a new BrickAnnote
BrickAnnote* subtract_annotes(BrickAnnote *A, BrickAnnote *B){
        bool found;
            
        BrickAnnote *C = create_brick_annote(A->get_suif_env(), "difference");
         
        for(Iter<SuifBrick*> iter = A->get_brick_iterator();
            iter.is_valid(); iter.next()){

            SuifObjectBrick *sob = to<SuifObjectBrick>(iter.current());
            found = search(B, sob->get_object());
            if(!found)
               C->append_brick(create_suif_object_brick(sob->get_suif_env(), sob->get_object()));
        }
        return C;
}

