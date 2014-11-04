/* file "bvd/solve.h" -- Iterative bit-vector data-flow solver */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef BVD_SOLVE_H
#define BVD_SOLVE_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "bvd/solve.h"
#endif

#include <machine/machine.h>
#include <cfg/cfg.h>

#include <bvd/flow_fun.h>


class Bvd {
  public:
    virtual ~Bvd() { }

    const NatSet* in_set(CfgNode*)  const;
    const NatSet* out_set(CfgNode*) const;

    virtual void find_kill_and_gen(Instr *) = 0;

    virtual const NatSet* kill_set() const = 0;
    virtual const NatSet* gen_set() const = 0;

    virtual int num_slots() const = 0;
    void print(CfgNode*, FILE* = stdout) const;

  protected:
    enum direction { forward, backward };
    enum confluence_rule { any_path, all_paths };

    Bvd(Cfg*,
	direction = forward,
	confluence_rule = any_path,
	FlowFun *entry_flow = NULL,
	FlowFun *exit_flow = NULL,
	int num_slots_hint = 0);

    bool solve(int iteration_limit = INT_MAX);

  protected:
    Cfg* _graph;
    FlowFun* _entry_flow;
    FlowFun* _exit_flow;
    int _num_slots_hint;

    direction _dir;
    confluence_rule _rule;

    Vector<FlowFun> _flow;		// slot set transformer for each node
    Vector<NatSetDense> _in;		// node-entry slot set for each node
    Vector<NatSetDense> _out;		// node-exit  slot set for each node

    Vector<NatSetDense> *_before;	// &_in  if forward, &_out if backward
    Vector<NatSetDense> *_after;	// &_out if forward, &_in  if backward

    void compute_local_flow(int);	// subroutines ...
    void combine_flow(CfgNode *);	//   ... of method solve
};

extern "C" void enter_bvd(int *argc, char *argv[]);
extern "C" void exit_bvd(void);

#endif /* BVD_SOLVE_H */
