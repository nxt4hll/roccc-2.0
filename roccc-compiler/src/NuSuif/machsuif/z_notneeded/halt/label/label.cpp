/* file "label/label.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "label/label.h"
#endif

#include <machine/machine.h>
#include <cfg/cfg.h>
#include <halt/halt.h>

#include <label/label.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

using namespace halt;

LabelPass::LabelPass()
{
    unique_id = 0;
    unique_id_entry = 0;
    debug_proc = empty_id_string;

    label_cbr = false;
    label_mbr = false;
    label_entry = false;
    label_exit = false;
    label_setlongjmp = false;
    label_lod = false;
    label_str = false;
    label_bb = false;
}

void
LabelPass::initialize(void)
{
    // if necessary, lookup symbols for setjmp and longjmp
    if (label_setlongjmp) {
    	setjmp = lookup_external_proc(setjmp_proc);
	longjmp = lookup_external_proc("longjmp");
    }
}

void
LabelPass::do_opt_unit(OptUnit *unit)
{
    cur_unit = unit;

    Sym* target;
    IdString name = get_name(get_proc_sym(unit));
    debug(2, "Processing procedure \"%s\"", name.chars());
    debug(4, "...unique_id is %d", unique_id);

    if (debug_proc != empty_id_string && debug_proc != name)
	return;		// when debugging, only instrument the specified proc

    // this pass requires a unit's body to be a CFG
    claim(is_kind_of<Cfg>(get_body(cur_unit)), "Body is not in CFG form");
    unit_cfg = static_cast<Cfg*>(get_body(cur_unit));

    // Establish CFG preferences: nodes are maximal basic blocks and
    // not broken at calls, and the initial layout is retained.
    //canonicalize(unit_cfg, true, false, false);

    if_debug(4)
	fprint(stderr, unit_cfg, false, true);		// no layout, just code

    if (label_entry) {		// label procedure entry
	debug(6, "labelling procedure entry");

	// get first successor of entry node, the real procedure entry point
	CfgNode *b = get_entry_node(unit_cfg);
	b = *succs_start(b);

	unique_id_entry = unique_id;	// remember unique_id
	label_block(b, ENTRY);
    }

    // visit each block b in procedure
    for (int i = 0; i < nodes_size(unit_cfg); ++i) {
        CfgNode *b = get_node(unit_cfg, i);

	// label basic blocks, except entry and exit
	if (label_bb && (b != get_entry_node(unit_cfg))
	    && (b != get_exit_node(unit_cfg))) {
	    label_block(b, BLOCK);
	}

	for (InstrHandle h = start(b); h != end(b); ++h) {
	    Instr *instr = *h;

	    if (label_cbr && is_cbr(instr))
		label_instr(instr, CBR, unique_id++);

	    if (label_mbr && (has_note(instr, k_mbr_target_def) ||
			      has_note(instr, k_mbr_index_def)))
		label_instr(instr, MBR, unique_id++);

	    if (label_entry && is_return(instr)) {
		if (label_exit)
		    label_instr(instr, EXIT, unique_id++);
		else
		    label_instr(instr, EXIT, unique_id_entry);
	    }

	    if (label_lod && reads_memory(instr)) 
		label_instr(instr, LOAD, unique_id++);

	    if (label_str && writes_memory(instr))
		label_instr(instr, STORE, unique_id++);
	    
	    if (label_setlongjmp && is_call(instr)) {
		target = get_target(instr);
		if (target == NULL) {
		    if (OneNote<IrObject*> note = 
			get_note(instr, k_call_target)) {
			target = to<Sym>(note.get_value());
		    } else {
			continue;
		    }
		} 
		     
		if (setjmp != NULL && setjmp == target) {
		    label_instr(instr, SETJMP, unique_id++);
		} else if (longjmp == target && longjmp != NULL && target != NULL) {
		    label_instr(instr, LONGJMP, unique_id++);
		}
	    }

	}
    }
}

void
LabelPass::finalize()
{
    debug(4, "next unique number is %d", unique_id);
}

void
LabelPass::label_block(CfgNode *block, int i_kind)
{
    debug(4, "labelling block %d", get_number(block));

    // create note and instruction to hold note
    Instr *holder = new_instr_dot(opcode_null);
    HaltLabelNote note(i_kind, unique_id);
    set_note(holder, k_halt, note);

    unique_id++;	// increment for next instrumentation point

    Instr *first = first_non_label(block);
    if (first == NULL) {
	// no non-labels in block so append holder to end of block
	append(block, holder);

    } else {
	// get handle of first
	for (InstrHandle h = start(block); h != end(block); ++h) {
	    if (*h == first) {
		if (has_note(*h, k_proc_entry))
		    insert_after(block, h, holder);
		else
		    insert_before(block, h, holder);
		return;
	    }
	}
	claim(false, "oops, couldn't find first");
    }
}

void
LabelPass::label_instr(Instr* in, int i_kind, long u_num)
{
    if_debug(4) {
	fprintf(stderr, "labelling: ");
	fprint(stderr, in);
    }
    HaltLabelNote note(i_kind, u_num);
    set_note(in, k_halt, note);	     
}

