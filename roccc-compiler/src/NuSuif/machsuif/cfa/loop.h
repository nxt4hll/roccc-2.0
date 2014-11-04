/* file "cfa/loop.h" -- Natural Loop Analysis */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef CFA_LOOP_H
#define CFA_LOOP_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "cfa/loop.h"
#endif

#include <machine/machine.h>
#include <cfg/cfg.h>
#include <cfa/dom.h>

class NaturalLoopInfo {
  public:
    NaturalLoopInfo(DominanceInfo *dom_info)
	: _dom_info(dom_info), _depth(NULL), _loop(NULL) { }
    virtual ~NaturalLoopInfo() { delete [] _depth; }

    DominanceInfo *dom_info() const { return _dom_info; }

    void find_natural_loops();

    const NatSet* loop_at(int n) const;
    const NatSet* loop_at(CfgNode *n) const;
    int loop_depth(int n) const;
    int loop_depth(CfgNode *n) const;
    void set_loop_depth(CfgNode *n, int d);
    void set_loop_depth(int n, int d);

    bool is_loop_begin(int n) const;	// true if block is loop entry
    bool is_loop_begin(CfgNode *cn) const;
    bool is_loop_end(int n) const;	// true if block jumps to loop entry
    bool is_loop_end(CfgNode *cn) const;
    bool is_loop_exit(int n) const;	// true if block is a loop exit
    bool is_loop_exit(CfgNode *cn) const;

    void print(FILE* = stdout) const;

  private:
    DominanceInfo *_dom_info;
    int *_depth;			// int per node
    NatSetDense *_loop;			// bit vector per node
};

#endif /* CFA_LOOP_H */
