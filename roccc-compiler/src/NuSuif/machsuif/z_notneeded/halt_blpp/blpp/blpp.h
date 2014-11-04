/* file "blpp/blpp.h" -- Ball and Larus path profiling pass */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef BLPP_BLPP_H
#define BLPP_BLPP_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "blpp/blpp.h"
#endif

#include <machine/machine.h>
#include <cfg/cfg.h>


typedef struct BlppNode {
    int id;
    List<struct BlppEdge*> preds;
    List<struct BlppEdge*> succs;
} BlppNode;

typedef struct BlppEdge {
    BlppNode* head;
    BlppNode* tail;

    int chord;
    int events;
    int d_id;
    int inc;
    int reset;
    int inst;
} BlppEdge;

typedef struct PathInfo {
    int instr_len;
    NatSetDense src_lines;
} PathInfo;

// This defines the substrate-independent OPI class.
class BlppPass {
  public:
    BlppPass();

    void initialize();
    void do_opt_unit(OptUnit*);
    void finalize();

    int proc_def;

  protected:
    OptUnit *cur_unit;
    Cfg	*unit_cfg;

    // edge based representation of graph
    Vector<BlppNode> nodes;
    List<BlppEdge> edges;
    int num_nodes, tmp;

    // routines for BLPP algorithm
    void create_graph(void);
    void assign_edge_vals(void);
    void choose_st(void);
    void calc_incs(void);
    void annotate(void);

    // supporting routines
    BlppEdge* find_edge(int, int);
    void new_node(int);
    BlppEdge new_edge(int, int);
    int* get_rpo(void);
    void dfs(int*, int*, BlppNode);
    void print_graph(void);
    int get_succ_index(CfgNode*, CfgNode*);
    BlppEdge* dummy_match(BlppEdge*);
    void dfs_st(int, int, BlppEdge*);
    int dir(BlppEdge*, BlppEdge*);
    void label_light(BlppEdge*, int, int);
    void label_heavy(BlppEdge*, int, int, int);
    void label_proc(int uid);
    void gen_static_info(void);
    PathInfo* update(BlppEdge*, PathInfo*, int, int, NatSet*);
    void add_lines_to_set(int, NatSet*) ;
};


#endif /* BLPP_BLPP_H */



