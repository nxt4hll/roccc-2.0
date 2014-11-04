/* file "cfg/node.h" -- Control Flow Graph Nodes */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef CFG_NODE_H
#define CFG_NODE_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "cfg/node.h"
#endif

#include <machine/machine.h>
#include <cfg/cfg_ir.h>
#include <cfg/graph.h>

CfgNode* new_empty_node(Cfg*);
CfgNode* insert_empty_node(Cfg*, CfgNode *tail, CfgNode *head);
CfgNode* insert_empty_node(Cfg*, CfgNode *tail, int succ_pos);
CfgNode* clone_node(Cfg*, CfgNode*);

Cfg* get_parent(CfgNode*);
int get_number(CfgNode*);
LabelSym* get_label(CfgNode*);
void fprint(FILE*, CfgNode*, bool show_code = false,
	    bool show_addrs = false, bool no_header = false);

int instrs_size(CfgNode*);
InstrHandle instrs_start(CfgNode*);
InstrHandle instrs_last(CfgNode*);
InstrHandle instrs_end(CfgNode*);
InstrHandle prepend(CfgNode*, Instr*);
InstrHandle append(CfgNode*, Instr*);
void replace(CfgNode*, InstrHandle, Instr*);
InstrHandle insert_before(CfgNode*, InstrHandle, Instr*);
InstrHandle insert_after(CfgNode*, InstrHandle, Instr*);
Instr* remove(CfgNode*, InstrHandle);

inline int
size(CfgNode *node)
{
    return instrs_size(node);
}

inline InstrHandle
start(CfgNode *node)
{
    return instrs_start(node);
}

inline InstrHandle
last(CfgNode *node)
{
    return instrs_last(node);
}

inline InstrHandle
end(CfgNode *node)
{
    return instrs_end(node);
}

int succs_size(CfgNode*);
CfgNodeHandle succs_start(CfgNode*);
CfgNodeHandle succs_end(CfgNode*);

int preds_size(CfgNode*);
CfgNodeHandle preds_start(CfgNode*);
CfgNodeHandle preds_end(CfgNode*);

CfgNode *fall_succ(CfgNode*);
CfgNode *taken_succ(CfgNode*);
CfgNode* get_pred(CfgNode*, int pos);
CfgNode* get_succ(CfgNode*, int pos);
void set_succ(CfgNode*, int pos, CfgNode *succ);
void set_fall_succ(CfgNode*, CfgNode *succ);
void set_taken_succ(CfgNode*, CfgNode *succ);
void set_exceptional_succ(CfgNode*, int n, CfgNode *succ);
void set_impossible_succ(CfgNode*, int n, CfgNode *succ);
bool is_normal_succ(CfgNode*, CfgNode *succ);
bool is_normal_succ(CfgNode*, int pos);
bool is_exceptional_succ(CfgNode*, CfgNode *succ);
bool is_exceptional_succ(CfgNode*, int pos);
bool is_possible_succ(CfgNode*, CfgNode *succ);
bool is_possible_succ(CfgNode*, int pos);
bool is_impossible_succ(CfgNode*, CfgNode *succ);
bool is_impossible_succ(CfgNode*, int pos);
bool is_abnormal_succ(CfgNode*, CfgNode *succ);
bool is_abnormal_succ(CfgNode*, int pos);
void remove_abnormal_succ(CfgNode*, CfgNode *succ);
void remove_abnormal_succ(CfgNode*, int pos);

bool ends_in_cti(CfgNode*);	// has CTI
bool ends_in_ubr(CfgNode*);	// has CTI satisfying is_ubr()
bool ends_in_cbr(CfgNode*);	// has CTI satisfying is_cbr()
bool ends_in_mbr(CfgNode*);	// has CTI satisfying is_mbr()
bool ends_in_call(CfgNode*);	// has CTI satisfying is_call()
bool ends_in_return(CfgNode*);	// has CTI satisfying is_return()
Instr* get_cti(CfgNode*);
InstrHandle get_cti_handle(CfgNode*);

void invert_branch(CfgNode*);

void reflect_cti(CfgNode*, Instr *cti, CfgNode *implicit_succ = NULL);

CfgNode *get_layout_pred(CfgNode*);
CfgNode *get_layout_succ(CfgNode*);

void clear_layout_succ(CfgNode*);
bool set_layout_succ(CfgNode*, CfgNode *succ);
bool set_layout_succ(CfgNode*, LabelSym *succ_label);

Instr *first_non_label(CfgNode*);
Instr *first_active_instr(CfgNode*);
Instr *last_non_cti(CfgNode*);

#endif /* CFG_NODE_H */
