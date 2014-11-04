// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#ifndef CONTROL_FLOW_UTILS_H
#define CONTROL_FLOW_UTILS_H

// computes intersections of all the predecessors' outs, then assigns the result to in
// computes the forward must for available expressions
void intersect_predecessor_annotes(BrickAnnote *in, BrickAnnote *predecessors);

// computes unions of all the predecessors out_stmts, then assigns the result to in
// computes the forward may for reaching defs
void union_predecessor_annotes(BrickAnnote *in, BrickAnnote *predecessor);

void union_predecessor_annotes(BrickAnnote *in, BrickAnnote *predecessor, String annote_name);

#endif

