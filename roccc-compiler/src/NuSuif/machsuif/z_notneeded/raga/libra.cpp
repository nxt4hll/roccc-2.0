/* file "raga/libra.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#include <machine/machine.h>
#include <cfg/cfg.h>

#include <raga/raga.h>
#include <raga/libra.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif


// Flag indicating that spill instructions receive "spill_code" notes
bool note_spills;

// Spill counters
int spill_load_count;		// number of loads  added by spillage
int spill_store_count;		// number of stores added by spillage

RaCell::RaCell()
{
    id = 0;
    frequency = 0.0;
    color = 0;
    opnd = opnd_null();
    temp = 0;
}

/*
 * Implementation of code scan and update utilities for register allocation
 */

class ReplaceOpndFilter : public OpndFilter {
  public:
    ReplaceOpndFilter(Raga *r, InstrHandle h) : raga(r), handle(h) { }
    Opnd operator()(Opnd, InOrOut);
  protected:
    Raga *raga;
    InstrHandle handle;
};

static scan_edit_instr_f    scan_edit_instr_pre;
static scan_edit_instr_f    scan_edit_instr_post;
static scan_edit_screen_f   scan_edit_screen;
static scan_edit_reg_cand_f scan_edit_var;
static scan_edit_reg_cand_f scan_edit_reg;
static scan_edit_reg_cand_f scan_edit_volatile;
static scan_edit_memory_f   scan_edit_memory;

Opnd
ReplaceOpndFilter::operator()(Opnd opnd, InOrOut in_or_out)
{
    const InOrOut IN = OpndFilter::IN;

    if (is_null(opnd) ||
	(scan_edit_screen && !(raga->*scan_edit_screen)(opnd)))
	return opnd;

    if (is_var(opnd)) {
	VarSym *v = get_var(opnd);

	if (is_auto(v) && !is_addr_taken(v)) {
	    if (scan_edit_var)
		return (raga->*scan_edit_var)(opnd, in_or_out == IN, handle);
	} else if (scan_edit_volatile)
	    return (raga->*scan_edit_volatile)(opnd, in_or_out == IN, handle);
    }
    else if (is_reg(opnd)) {
	if (scan_edit_reg)
	    return (raga->*scan_edit_reg)(opnd, in_or_out == IN, handle);
    }
    else if (scan_edit_memory
	     && is_addr_sym(opnd)
	     && is_kind_of<VarSym>(get_sym(opnd))) {
	(raga->*scan_edit_memory)((VarSym*)get_sym(opnd));
    }
    return opnd;
}

void
scan_edit_code(Raga *raga, Cfg *cfg, scan_edit_instr_f do_instr_pre,
	       scan_edit_instr_f do_instr_post, scan_edit_screen_f do_screen,
	       scan_edit_reg_cand_f do_var, scan_edit_reg_cand_f do_reg,
	       scan_edit_reg_cand_f do_volatile, scan_edit_memory_f do_memory,
	       scan_edit_direction_t direction, scan_edit_block_f do_block)
{
    scan_edit_instr_pre  = do_instr_pre;
    scan_edit_instr_post = do_instr_post;
    scan_edit_screen = do_screen;
    scan_edit_var = do_var;
    scan_edit_reg = do_reg;
    scan_edit_volatile = do_volatile;
    scan_edit_memory = do_memory;

    CfgNodeListRpo rpo(cfg, direction == FWD);

    for (CfgNodeHandle nh = rpo.start(); nh != rpo.end(); ++nh) {
	CfgNode *cnode = *nh;

	if (do_block)
	    (raga->*do_block)(cnode, direction);

	if (size(cnode) == 0)
	    continue;

	InstrHandle h = (direction == FWD) ? start(cnode) : last(cnode);
	InstrHandle z = (direction == BWD) ? start(cnode) : last(cnode);
	for ( ;; ) {
	    Instr *instr = *h;
	    ReplaceOpndFilter filter(raga, h);

	    if (scan_edit_instr_pre)
		(raga->*scan_edit_instr_pre)(instr);

	    map_opnds(instr, filter);

	    if (scan_edit_instr_post)
		(raga->*scan_edit_instr_post)(instr);

	    if (h == z)
		break;
	    if (direction == FWD)
		++h;
	    else
		--h;
	}
    }
}

/* load_or_store() -- Create a load or store instruction for given register
 * operand and memory symbol.  Since we don't know the stack offset yet, we
 * just use the symbol to create an effective address.
 */
Instr*
load_or_store(bool do_load, Opnd opnd, VarSym *v)
{
    /*
     * A VR operand may have a type of smaller size than the symbol
     * to which it is spilled.  Use the type of the symbol so that
     * the register is not spilled using one size and reloaded with
     * a larger size.
     */
    TypeId t = unqualify_data_type(get_type(v));
    Opnd ea = opnd_addr_sym(v);
    Instr *mi =
	do_load
	    ? new_instr_alm(opnd, opcode_load(t), ea)
	    : new_instr_alm(ea,  opcode_store(t), opnd);
    return mi;
}


void
AdjacencyMatrix::init(size_t new_count)
{
    size_t needed = (new_count * (new_count - 1) + 14) / 16;

    if (bytes == NULL || allocated < needed || needed < (allocated / 4))
    {
	delete bytes;
	allocated = needed;
	bytes = new unsigned char[needed];
    }
    node_count = new_count;
    memset((void *)bytes, 0, needed);
}

/*
 * Produce a pointer to a matrix whose (i, j) element represents the effect
 * on a candidate node with class Ci of a neighbor node with class Cj.
 * More specifically, it is the worst-case number of members of Ci that the
 * neighbor might block the candidate from being assigned to.
 *
 * NOTE: the caller is responsible for delete-ing the pointer returned.
 */

Vector<Vector<int> >*
class_displacements()
{
    Vector<Vector<int> > *result = new Vector<Vector<int> >;
    int nclasses = reg_class_count();

    for (int i = 0; i < nclasses; ++i)
    {
	const NatSet *ci = reg_members(i);
	Vector<int> displacements;

	for (int j = 0; j < nclasses; ++j)
	{
	    int max = 0;
	    const NatSet *cj = reg_members(j);
	    for (NatSetIter jit = cj->iter(); jit.is_valid(); jit.next())
	    {
		NatSetDense s = *ci;
		s *= *reg_aliases(jit.current());
		int displaced = s.size();
		if (displaced > max)
		    max = displaced;
	    }
	    displacements.push_back(max);
	}
	result->push_back(displacements);
    }
    return result;
}

/*
 * Produce a pointer to a matrix whose (i, j) element represents the aliases
 * of register i that are the class with ID j.
 *
 * NOTE: the caller is responsible for delete-ing the pointer returned.
 */

Vector<Vector<NatSetDense> >*
aliases_modulo_class()
{
    Vector<Vector<NatSetDense> > *result = new Vector<Vector<NatSetDense> >;
    int nregs = reg_count();
    int nclasses = reg_class_count();

    for (int i = 0; i < nregs; ++i)
    {
	Vector<NatSetDense> row;

	for (int j = 0; j < nclasses; ++j)
	{
	    NatSetDense aliases = *reg_aliases(i);
	    aliases *= *reg_members(j);

	    row.push_back(aliases);
	}
	result->push_back(row);
    }
    return result;
}

static scan_edit_reg_cand_f scan_edit_opnd_reg;

class ReplaceOpndRegFilter : public OpndFilter {
  public:
    ReplaceOpndRegFilter(Raga *r, InstrHandle h) : raga(r), handle(h) { }
    Opnd operator()(Opnd, InOrOut);
  protected:
    Raga *raga;
    InstrHandle handle;
};

Opnd
ReplaceOpndRegFilter::operator()(Opnd opnd, InOrOut in_or_out)
{
    const InOrOut IN = OpndFilter::IN;

    if (is_reg(opnd)) {
	if (scan_edit_opnd_reg)
	    return (raga->*scan_edit_opnd_reg)(opnd, in_or_out == IN, handle);
    }
    return opnd;
}

void
scan_edit_opnds(Raga *raga, InstrHandle h, scan_edit_reg_cand_f do_reg)
{
    scan_edit_opnd_reg = do_reg;

    ReplaceOpndRegFilter filter(raga, h);
    map_opnds(*h, filter);
}
