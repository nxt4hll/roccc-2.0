// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#ifndef BIT_VECTOR_ANNOTE_UTILS_H
#define BIT_VECTOR_ANNOTE_UTILS_H

#include "roccc_extra_types/bit_vector.h"

void bv_create_n_append_annote(ExecutionObject *eo, String name, BitVectorMap *bvm);

void bv_empty(BrickAnnote *ba);

BrickAnnote* bv_clone(BrickAnnote *ba);

bool bv_search(BrickAnnote *ba, SuifObject *so);
bool bv_search(BrickAnnote *ba, String s);

void bv_mark(BrickAnnote *ba, SuifObject *so);

void bv_remove_mark(BrickAnnote *ba, SuifObject *so);

// removes the bricks of rb from ba, if the objects in BrickAnnote rb are in BrickAnnote ba 
void bv_remove_bricks(BrickAnnote *ba, BrickAnnote *rb);

// appends the bricks of ab into ba, if the objects in BrickAnnote ab are not in BrickAnnote ba 
void bv_append_bricks(BrickAnnote *ba, BrickAnnote *ab);

// empties A and copies the contents of B into A
void bv_copy_annotes(BrickAnnote *A, BrickAnnote *B);

// computes a union b, then assigns the result to A
void bv_union_annotes(BrickAnnote *A, BrickAnnote *B);

// computes a intersect b, then assigns the result to A
void bv_intersect_annotes(BrickAnnote *A, BrickAnnote *B);

// computes a-b, then assigns the result to a new BrickAnnote
BrickAnnote* bv_subtract_annotes(BrickAnnote *A, BrickAnnote *B);

// computes a-b, then assigns the result to C
void bv_subtract_annotes(BrickAnnote *C, BrickAnnote *A, BrickAnnote *B);

// computes a-b, then assigns the result to a 
void bv_subtract_n_overwrite_annotes(BrickAnnote *A, BrickAnnote *B);

#endif

