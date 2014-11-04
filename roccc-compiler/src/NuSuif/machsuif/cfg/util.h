/* file "cfg/util.h" -- Control Flow Graph Utilities */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef CFG_UTIL_H
#define CFG_UTIL_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "cfg/util.h"
#endif

#include <machine/machine.h>

#include <cfg/cfg_ir.h>
#include <cfg/graph.h>

class DepthFirstWalkAction {
  public:
    virtual ~DepthFirstWalkAction() { }
    virtual void operator()(CfgNode*) { }
};

extern void
    depth_first_walk(CfgNode *start, bool forward, NatSet *visited,
		     DepthFirstWalkAction&, bool not_impossible = false);

class CfgNodeListRpo
{
  public:
    CfgNodeListRpo(Cfg *graph, bool forward = true);
    virtual ~CfgNodeListRpo();

    int size();
    CfgNodeHandle start();
    CfgNodeHandle end();

    void prepend(CfgNode*);
    void append(CfgNode*);
    void replace(CfgNodeHandle, CfgNode*);
    void insert_before(CfgNodeHandle, CfgNode*);
    void insert_after(CfgNodeHandle, CfgNode*);
    CfgNode* remove(CfgNodeHandle);

  protected:
    list<CfgNode*> nodes;
};

extern IdString k_cfg_node;

CfgNode *label_cfg_node(Sym *);

CfgNode* get_parent_node(Instr*);

extern void generate_vcg(FILE *, Cfg *);	// Dump vcg format file

extern bool has_pred(CfgNode *node, CfgNode *pred);

#endif /* CFG_UTIL_H */
