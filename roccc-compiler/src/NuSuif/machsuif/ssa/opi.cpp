/* file "ssa/opi.cpp" */

/*
    Copyright (c) 2000-2001 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "ssa/opi.h"
#endif

#include <machine/machine.h>
#include <suifvm/suifvm.h>
#include <cfg/cfg.h>
#include <cfa/cfa.h>
#include <bvd/bvd.h>

#include <ssa/ssa_ir.h>
#include <ssa/ssa_ir_factory.h>
#include <ssa/ssa_core_ptr.h>
#include <ssa/ssa_core.h>
#include <ssa/opi.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif


SsaCfg*
new_ssa_cfg(Cfg *cfg, OptUnit *unit, unsigned flags)
{
    claim(cfg->get_parent() == NULL || cfg->get_parent() == unit,
	  "CFG's parent is not the given optimization unit");

    // Must orphan the CFG before reparenting it.  Do this by detaching it
    // from the unit.  Otherwise, when the unit body is updated later
    // (i.e., set to the new SSA form), the parent of its old body (the
    // CFG) gets set to NULL.
					
    set_body(unit, NULL);
    SsaCfg *ssa = create_ssa_cfg(the_suif_env, cfg, unit, false, false, false,
				 false, false, false, false, false, false);

    ssa->core.set_underlying_ptr(new SsaCore(ssa));
    ssa->core->build(flags);

    return ssa;
}

Cfg*
restore(SsaCfg *ssa, bool not_old_names)
{
    return ssa->core->restore(not_old_names);
}

/*
 * ssa_cfg_to_instr_list -- First restore conventional (non-SSA form).
 * Then convert the resulting CFG to InstrList form, delete the now-empty
 * CFG, and return the instruction list.
 */
InstrList*
ssa_cfg_to_instr_list(SsaCfg *ssa_cfg)
{
    Cfg *cfg = restore(ssa_cfg);
    InstrList *instr_list = cfg->to_instr_list();

    delete cfg;
    return instr_list;
}

void
fprint(FILE *fp, SsaCfg *ssa)
{
    ssa->core->print(fp);
}

Cfg*
get_cfg(SsaCfg *ssa)
{
    return ssa->get_cfg();
}

bool
has_minimal_form(SsaCfg *ssa)
{
    return ssa->core->get_build_minimal_form();
}

bool
has_semi_pruned_form(SsaCfg *ssa)
{
    return ssa->core->get_build_semi_pruned_form();
}

bool
has_pruned_form(SsaCfg *ssa)
{
    return ssa->core->get_build_pruned_form();
}

bool
has_def_use_chains(SsaCfg *ssa)
{
    return ssa->get_build_def_use_chains();
}

bool
has_copies_folded(SsaCfg *ssa)
{
    return ssa->core->get_fold_copies();
}

bool
has_useless_phi_nodes_folded(SsaCfg *ssa)
{
    return ssa->core->get_omit_useless_phi_nodes();
}

int
get_loc_count(SsaCfg *ssa)
{
    return ssa->core->get_loc_count();
}

int
get_value_count(SsaCfg *ssa)
{
    return ssa->core->get_value_count();
}

const OpndCatalog*
get_value_catalog(SsaCfg *ssa)
{
    return ssa->core->get_value_catalog();
}

Opnd
get_value_name(SsaCfg *ssa, int value_id)
{
    return ssa->core->get_value_name(value_id);
}

CfgNode*
get_def_block(SsaCfg *ssa, int value_id)
{
    return ssa->core->get_def_block(value_id);
}

Operation
get_unique_def(SsaCfg *ssa, Opnd value_name)
{
    return ssa->core->get_unique_def(value_name);
}

const List<Operation>&
get_def_use_chain(SsaCfg *ssa, Opnd value_name)
{
    return ssa->core->get_def_use_chain(value_name);
}

const List<PhiNode*>&
get_phi_nodes(SsaCfg *ssa, CfgNode *block)
{
    return ssa->core->get_phi_nodes(block);
}

PhiNode*
remove_phi_node(SsaCfg *ssa, CfgNode *block, PhiNode *phi)
{
    return ssa->core->remove_phi_node(block, phi);
}

PhiHandle
append_phi_node(SsaCfg *ssa, CfgNode *block, PhiNode *phi)
{
    return ssa->core->append_phi_node(block, phi);
}

void
record_use_swap(SsaCfg *ssa, Opnd out, Opnd in, Operation operation)
{
    ssa->core->record_use_swap(out, in, operation);
}

void
record_all_uses(SsaCfg *ssa, Operation operation)
{
    ssa->core->record_all_uses(operation);
}

void
record_def(SsaCfg *ssa, Opnd old_name, Operation operation, int dst_pos)
{
    ssa->core->record_def(old_name, operation, dst_pos);
}


// ----------------------------	 class PhiNode  --------------------------------

PhiNode*
new_phi_node(int src_count)
{
    PhiNode *phi = create_phi_node(the_suif_env, NULL, false);

    while (src_count-- > 0)
	phi->append_src(NULL);

    return phi;
}

int
srcs_size(PhiNode *phi)
{
    return phi->get_src_count();
}

Opnd
get_src(PhiNode *phi, int pos)
{
    return phi->get_src(pos);
}

void
set_src(PhiNode *phi, int pos, Opnd opnd)
{
    phi->replace_src(pos, opnd);
}

Opnd
get_dst(PhiNode *phi)
{
    return phi->get_dst();
}

void
set_dst(PhiNode *phi, Opnd opnd)
{
    phi->set_dst(opnd);
}

bool
get_is_useless(PhiNode *phi)
{
    return phi->get_useless();
}

void
set_is_useless(PhiNode *phi, bool useless)
{
    phi->set_useless(useless);
}

CfgNode*
get_parent_node(PhiNode *phi)
{
    SuifObject *annote = phi->get_parent();
    claim(is_kind_of<SsaPhiAnnote>(annote), "Bad phi-node parent");

    SuifObject *block = static_cast<SsaPhiAnnote*>(annote)->get_parent();
    claim(is_kind_of<CfgNode>(block), "Bad phi-node annote parent");

    return static_cast<CfgNode*>(block);
}

void
map_opnds(PhiNode *phi, OpndFilter &filter)
{
    int size = phi->get_src_count();
    for (int i = 0; i < size; ++i)
	phi->replace_src(i, filter(phi->get_src(i), OpndFilter::IN));

    phi->set_dst(filter(phi->get_dst(), OpndFilter::OUT));
}


// ---------------------------	class Operation  -------------------------------

Opnd
get_dst(Operation operation, int pos)
{
    if (operation.is_phi_handle())
	return get_dst(*operation.get_phi_handle());
    else
	return get_dst(*operation.get_instr_handle(), pos);
}

CfgNode*
get_parent_node(Operation operation)
{
    if (operation.is_phi_handle())
	return get_parent_node(*operation.get_phi_handle());
    else
	return get_parent_node(*operation.get_instr_handle());
}

void
map_opnds(Operation operation, OpndFilter &filter)
{
    if (operation.is_phi_handle())
	map_opnds(*operation.get_phi_handle(), filter);
    else
	map_opnds(*operation.get_instr_handle(), filter);
}

bool
has_note(Operation operation, NoteKey key)
{
    if (operation.is_phi_handle())
	return has_note(*operation.get_phi_handle(), key);
    else
	return has_note(*operation.get_instr_handle(), key);
}

Note
get_note (Operation operation, NoteKey key)
{
    if (operation.is_phi_handle())
	return get_note(*operation.get_phi_handle(), key);
    else
	return get_note(*operation.get_instr_handle(), key);
}

Note
take_note(Operation operation, NoteKey key)
{
    if (operation.is_phi_handle())
	return take_note(*operation.get_phi_handle(), key);
    else
	return take_note(*operation.get_instr_handle(), key);
}

void
set_note (Operation operation, NoteKey key, const Note &note)
{
    if (operation.is_phi_handle())
	set_note(*operation.get_phi_handle(), key, note);
    else
	set_note(*operation.get_instr_handle(), key, note);
}
