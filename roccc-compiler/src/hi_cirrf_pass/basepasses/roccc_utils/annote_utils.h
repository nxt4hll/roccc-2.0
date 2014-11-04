// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#ifndef ANNOTE_UTILS_H
#define ANNOTE_UTILS_H

void create_n_append_annote(ExecutionObject *eo, String name);

void empty(BrickAnnote *ba);

BrickAnnote* clone(BrickAnnote *ba);

bool search(BrickAnnote *ba, SuifObject *so);
bool search(BrickAnnote *ba, String s);

void remove_brick(BrickAnnote *ba, SuifObject *so);

// removes the bricks of rb from ba, if the objects in BrickAnnote rb are in BrickAnnote ba 
void remove_bricks(BrickAnnote *ba, BrickAnnote *rb);

// appends the bricks of ab into ba, if the objects in BrickAnnote ab are not in BrickAnnote ba 
void append_bricks(BrickAnnote *ba, BrickAnnote *ab);

// empties A and copies the contents of B into A
void copy_annotes(BrickAnnote *A, BrickAnnote *B);

// computes a union b, then assigns the result to A
void union_annotes(BrickAnnote *A, BrickAnnote *B);

// computes a intersect b, then assigns the result to A
void intersect_annotes(BrickAnnote *A, BrickAnnote *B);

// computes a-b, then assigns the result to a new BrickAnnote
BrickAnnote* subtract_annotes(BrickAnnote *A, BrickAnnote *B);

#endif

