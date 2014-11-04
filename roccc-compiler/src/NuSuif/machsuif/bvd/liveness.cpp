/* file "bvd/liveness.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "bvd/liveness.h"
#endif

#include <machine/machine.h>

#include <bvd/liveness.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif


/* -------------------  Implementation of RegPartition  ------------------- */

RegPartition::RegPartition(OpndCatalog *catalog)
    : _catalog(catalog), _mates(reg_count())
{
    int nregs = reg_count();

    // Create a temporary map reg_id from each register number to its
    // catalog index, if catalog accepts the register, or to -1, if it
    // doesn't.

    Vector<int> reg_id(nregs);

    for (unsigned reg = 0; reg < reg_id.size(); ++reg) {
	int slot = -1;
	_catalog->enroll(opnd_reg(reg, type_v0), &slot);
	reg_id[reg] = slot;
    }

    for (int reg = 0; reg < nregs; ++reg) {
	const NatSet *aliases = reg_aliases(reg);
	for (NatSetIter i = aliases->iter(); i.is_valid(); i.next()) {
	    int mate = reg_id[i.current()];
	    if (mate >= 0)
		_mates[reg].insert(mate);
	}
    }
}

const NatSet*
RegPartition::mates(int reg) const
{
    claim((unsigned)reg < _mates.size(), "Invalid register number");

    return &_mates[reg];
}



/* ------------------  Implementation of DefUseAnalyzer  ------------------ */

class DefUseTeller : public OpndFilter {	// applied to each operand
    OpndCatalog *_catalog;
    const RegPartition *_partition;
    NatSetSparse *_defs;
    NatSetSparse *_uses;

  public:
    DefUseTeller(OpndCatalog *catalog, const RegPartition *partition,
		 NatSetSparse *defs, NatSetSparse *uses)
	: _catalog(catalog), _partition(partition), _defs(defs), _uses(uses)
    { }
    Opnd operator()(Opnd, InOrOut io);
};

Opnd DefUseTeller::operator()(Opnd opnd, InOrOut io)
{
    NatSet *result = ((io == IN) ? _uses : _defs);

    if (is_hard_reg(opnd)) {
	*result += *_partition->mates(get_reg(opnd));
    }
    else {
	int slot = -1;
	_catalog->enroll(opnd, &slot);
	if (slot >= 0)
	    result->insert(slot);
    }
    return opnd;
}


/*
 * defs_uses_from_note
 *
 * Given an instruction, a note key (k_regs_defd or k_regs_used), and a
 * hard-register partition, insert the mate sets of any registers in the
 * chosen annotation into the output set `result'.
 */
void
defs_uses_from_note(Instr *mi, NoteKey key, const RegPartition *partition,
		    NatSet *result)
{
    if (NatSetNote note = get_note(mi, key)) {
	NatSetDense noted;
	note.get_set(&noted);
	for (NatSetIter nit = noted.iter(); nit.is_valid(); nit.next()) {
	    int reg = nit.current();
	    *result += *partition->mates(reg);
	}
    }
}

void
DefUseAnalyzer::analyze(Instr *mi)
{
    _defs.remove_all();
    _uses.remove_all();
    DefUseTeller teller(_catalog, _partition, &_defs, &_uses);

    map_opnds(mi, teller);

    defs_uses_from_note(mi, k_regs_used, _partition, uses());
    defs_uses_from_note(mi, k_regs_defd, _partition, defs());
}
