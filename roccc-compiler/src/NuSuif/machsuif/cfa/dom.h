/* file "cfa/dom.h" -- Dominance Analysis */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef CFA_DOM_H
#define CFA_DOM_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "cfa/dom.h"
#endif

#include <machine/machine.h>
#include <cfg/cfg.h>

class DominanceInfo {
  public:
    DominanceInfo(Cfg *graph);
    virtual ~DominanceInfo();

    Cfg *graph() const { return _graph; }

    // Perform control-flow analysis
    void find_dominators();
    void find_postdominators();
    void find_dom_frontier();
    void find_reverse_dom_frontier();

    // Access analysis results
    bool dominates(int n_dominator, int n_dominatee) const;
    bool dominates(CfgNode *dominator, CfgNode *dominatee) const;
    bool postdominates(int n_dominator, int n_dominatee) const;
    bool postdominates(CfgNode *dominator, CfgNode *dominatee) const;
    const NatSet *dominators(int n) const;
    const NatSet *dominators(CfgNode *n) const;
    const NatSet *postdominators(int n) const;
    const NatSet *postdominators(CfgNode *n) const;
    CfgNode *immed_dom(int n) const;
    CfgNode *immed_dom(CfgNode *n) const;
    CfgNode *immed_postdom(int n) const;
    CfgNode *immed_postdom(CfgNode *n) const;
    const NatSet *dom_frontier(int n) const;
    const NatSet *dom_frontier(CfgNode *n) const;
    const NatSet *reverse_dom_frontier(int n) const;
    const NatSet *reverse_dom_frontier(CfgNode *n) const;

    void print(FILE * = stdout) const;

  protected:
    NatSetDense *do_dominators(bool forward) const;
    CfgNode **do_immed_dominators(bool forward) const;
    void do_dom_frontiers(CfgNode *, bool forward);

  private:
    Cfg *_graph;

    NatSetDense *_doms;		// bit vector per node
    NatSetDense *_pdoms;	// bit vector per node
    CfgNode **_idom;		// node per node
    CfgNode **_ipdom;		// node per node
    NatSetDense *_df;		// bit vector per node
    NatSetDense *_rdf;		// bit vector per node
};

#endif /* CFA_DOM_H */
