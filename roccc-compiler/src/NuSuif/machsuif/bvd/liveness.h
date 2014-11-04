/* file "bvd/liveness.h" -- Liveness analyzer */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef BVD_LIVENESS_H
#define BVD_LIVENESS_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "bvd/liveness.h"
#endif

#include <machine/machine.h>
#include <cfg/cfg.h>

#include <bvd/solve.h>

class RegPartition {
  public:
    RegPartition(OpndCatalog*);
    const NatSet* mates(int reg) const;

  private:
    OpndCatalog *_catalog;
    Vector<NatSetSparse> _mates;
};

class DefUseAnalyzer {
  public:
    DefUseAnalyzer(OpndCatalog *catalog)
	: _catalog(catalog), _partition(new RegPartition(catalog))
	{ _own_partition = _partition; }
    DefUseAnalyzer(OpndCatalog *catalog, const RegPartition *partition)
	: _catalog(catalog), _partition(partition), _own_partition(NULL) { }
    virtual ~DefUseAnalyzer() { delete _own_partition; }

    virtual void analyze(Instr *mi);

    NatSetIter defs_iter() const	      { return _defs.iter(); }
    NatSetIter uses_iter() const	      { return _uses.iter(); }

    const NatSet *defs_set() const	      { return &_defs; }
    const NatSet *uses_set() const	      { return &_uses; }

  protected:
    OpndCatalog *catalog()		      { return _catalog; }
    const RegPartition *partition()	      { return _partition; }
    NatSet *defs()			      { return &_defs; }
    NatSet *uses()			      { return &_uses; }

  private:
    OpndCatalog *_catalog;
    const RegPartition *_partition;
    const RegPartition *_own_partition;
    NatSetSparse _defs;
    NatSetSparse _uses;
};

class Liveness : public Bvd {
  public:
    Liveness(Cfg *graph, OpndCatalog *catalog, DefUseAnalyzer *analyzer)
	: Bvd(graph, backward, any_path)
    {
	_catalog = catalog;

	if (analyzer) {
	    _analyzer_own = NULL; _analyzer = analyzer;
	} else
	    _analyzer_own = _analyzer = new DefUseAnalyzer(catalog);

	solve();
    }

    virtual ~Liveness()			      { delete _analyzer_own; }

    virtual void find_kill_and_gen(Instr *mi)
	{ _analyzer->analyze(mi); }

    virtual const NatSet* kill_set() const    { return _analyzer->defs_set(); }
    virtual const NatSet* gen_set()  const    { return _analyzer->uses_set(); }

    virtual int num_slots() const	      { return _catalog->num_slots(); }

  protected:
    OpndCatalog* catalog()		      { return _catalog; }
    DefUseAnalyzer* analyzer()		      { return _analyzer; }

  private:
    OpndCatalog *_catalog;
    DefUseAnalyzer *_analyzer;
    DefUseAnalyzer *_analyzer_own;	      // default analyzer
};

#endif /* BVD_LIVENESS_H */
