/* file "cfg/graph.h" -- Control Flow Graph */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef CFG_GRAPH_H
#define CFG_GRAPH_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "cfg/graph.h"
#endif

#include <machine/machine.h>
#include <cfg/cfg_ir.h>

typedef list<CfgNode*>::iterator CfgNodeHandle;

Cfg* new_cfg(InstrList*, bool keep_layout    = false,
			 bool break_at_call  = false,
			 bool break_at_instr = false);
void canonicalize(Cfg*,  bool keep_layout    = false,
                         bool break_at_call  = false,
                         bool break_at_instr = false);
void remove_layout_info(Cfg*);
void fix_layout(Cfg*);

CfgNode* get_node(Cfg*, int pos);
CfgNode* get_node(Cfg*, CfgNodeHandle);
CfgNode* node_at(Cfg*, LabelSym*);

int nodes_size(Cfg*);
CfgNodeHandle nodes_start(Cfg*);
CfgNodeHandle nodes_last(Cfg*);
CfgNodeHandle nodes_end(Cfg*);

CfgNode* get_entry_node(Cfg*);
CfgNode* get_exit_node(Cfg*);

void set_entry_node(Cfg*, CfgNode*);
void set_exit_node(Cfg*, CfgNode*);

inline int
size(Cfg *cfg)
{
    return nodes_size(cfg);
}

inline CfgNodeHandle
start(Cfg *cfg)
{
    return nodes_start(cfg);
}

inline CfgNodeHandle
end(Cfg *cfg)
{
    return nodes_end(cfg);
}

bool remove_unreachable_nodes(Cfg*);
bool merge_node_sequences(Cfg*);
bool optimize_jumps(Cfg*);

void fprint(FILE*, Cfg*);
void fprint(FILE*, Cfg*, bool follow_layout, bool show_code);

#endif /* CFG_GRAPH_H */
