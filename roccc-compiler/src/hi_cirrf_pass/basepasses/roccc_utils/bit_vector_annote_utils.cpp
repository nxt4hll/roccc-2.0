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
#include "roccc_extra_types/bit_vector.h"
#include "bit_vector_annote_utils.h"

/**************************** Declerations ************************************/


/************************** Implementations ***********************************/

void bv_create_n_append_annote(ExecutionObject *eo, String name, BitVectorMap *bvm){
	if(eo->lookup_annote_by_name(name))
	   delete (eo->remove_annote_by_name(name));
	SuifEnv *env = eo->get_suif_env();
	BrickAnnote *ba = create_brick_annote(env, name);
	eo->append_annote(ba);	

        BitVector *bv = new BitVector(bvm);
        bv->reset();
        ba->append_brick(create_suif_object_brick(env, bv));
}

void bv_empty(BrickAnnote *ba){
  	SuifObjectBrick *sob = to<SuifObjectBrick>(ba->get_brick(0));
	BitVector *bv = (BitVector*)(sob->get_object());
	bv->reset();
}

BrickAnnote* bv_clone(BrickAnnote *ba){
	SuifEnv* env = ba->get_suif_env();
	SuifObjectBrick *sob = to<SuifObjectBrick>(ba->get_brick(0));
	BitVector *bv = (BitVector*)(sob->get_object());

        BrickAnnote *clone = create_brick_annote(env, ba->get_name());
	clone->append_brick(create_suif_object_brick(env, bv->clone()));
	
	return clone;
}

bool bv_search(BrickAnnote *ba, SuifObject *so){

	SuifObjectBrick *sob = to<SuifObjectBrick>(ba->get_brick(0));
	BitVector *bv = (BitVector*)(sob->get_object());
	
	return bv->is_marked(so);
}

bool bv_search(BrickAnnote *ba, String s){
	for(Iter<SuifBrick*> iter = ba->get_brick_iterator();
	    iter.is_valid(); iter.next()){

	    StringBrick *sb = to<StringBrick>(iter.current());
	    if(sb->get_value() == s)
	       return 1;	
	}
	return 0;
}

void bv_mark(BrickAnnote *ba, SuifObject *so){

	SuifObjectBrick *sob = to<SuifObjectBrick>(ba->get_brick(0));
	BitVector *bv = (BitVector*)(sob->get_object());
	bv->mark(so);	
}

void bv_remove_mark(BrickAnnote *ba, SuifObject *so){

	SuifObjectBrick *sob = to<SuifObjectBrick>(ba->get_brick(0));
	BitVector *bv = (BitVector*)(sob->get_object());
	bv->unmark(so);	
}

// copies the contents of B into A 
void bv_copy_annotes(BrickAnnote *A, BrickAnnote *B){

  //  if (A == NULL || B == NULL)
  //  return ;

	SuifObjectBrick *sob = to<SuifObjectBrick>(B->get_brick(0));
	BitVector *bv_B = (BitVector*)(sob->get_object());

	sob = to<SuifObjectBrick>(A->get_brick(0));
	BitVector *bv_A = (BitVector*)(sob->get_object());
	
	bv_A->copy(bv_B);
}

// computes a union b, then assigns the result to A
void bv_union_annotes(BrickAnnote *A, BrickAnnote *B){

	SuifObjectBrick *sob = to<SuifObjectBrick>(B->get_brick(0));
	BitVector *bv_B = (BitVector*)(sob->get_object());

	sob = to<SuifObjectBrick>(A->get_brick(0));
	BitVector *bv_A = (BitVector*)(sob->get_object());
	
	bv_A->union_(bv_B);
}

// computes a intersect b, then assigns the result to A
void bv_intersect_annotes(BrickAnnote *A, BrickAnnote *B){

	SuifObjectBrick *sob = to<SuifObjectBrick>(B->get_brick(0));
	BitVector *bv_B = (BitVector*)(sob->get_object());

	sob = to<SuifObjectBrick>(A->get_brick(0));
	BitVector *bv_A = (BitVector*)(sob->get_object());
	
	bv_A->intersect(bv_B);
}

// computes a-b, then assigns the result to a new BrickAnnote
BrickAnnote* bv_subtract_annotes(BrickAnnote *A, BrickAnnote *B){

	SuifEnv *env = A->get_suif_env();

        BrickAnnote *C = create_brick_annote(env, "difference");
         
	SuifObjectBrick *sob = to<SuifObjectBrick>(B->get_brick(0));
	BitVector *bv_B = (BitVector*)(sob->get_object());

	sob = to<SuifObjectBrick>(A->get_brick(0));
	BitVector *bv_A = (BitVector*)(sob->get_object());

	BitVector *bv_C= bv_A->subtract(bv_B);

        C->append_brick(create_suif_object_brick(env, bv_C));

        return C;
}

// computes a-b, then assigns the result to c
void bv_subtract_annotes(BrickAnnote *C, BrickAnnote *A, BrickAnnote *B){

	SuifEnv *env = A->get_suif_env();

	SuifObjectBrick *sob = to<SuifObjectBrick>(A->get_brick(0));
	BitVector *bv_A = (BitVector*)(sob->get_object());

	sob = to<SuifObjectBrick>(B->get_brick(0));
	BitVector *bv_B = (BitVector*)(sob->get_object());

	sob = to<SuifObjectBrick>(C->get_brick(0));
	BitVector *bv_C = (BitVector*)(sob->get_object());

	bv_C->subtract(bv_A, bv_B);
}

// computes a-b, then assigns the result to a 
void bv_subtract_n_overwrite_annotes(BrickAnnote *A, BrickAnnote *B){

	SuifObjectBrick *sob = to<SuifObjectBrick>(B->get_brick(0));
	BitVector *bv_B = (BitVector*)(sob->get_object());

	sob = to<SuifObjectBrick>(A->get_brick(0));
	BitVector *bv_A = (BitVector*)(sob->get_object());
	
	bv_A->subtract_n_overwrite(bv_B);
}


