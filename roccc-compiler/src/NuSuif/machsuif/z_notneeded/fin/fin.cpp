/* file "fin/fin.cc" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "fin/fin.h"
#endif

#include <machine/machine.h>
#include <cfg/cfg.h>

#include <fin/fin.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

/*
 * Purpose of program: This program finalizes a target-specific
 * Machine-SUIF file by laying out the stack frame and adding
 * prologue/epilogue code.
 */

void
Fin::initialize()
{
    debug(1, "Debug level is %d", debuglvl);

    // grab the target-specific finalizer
    code_fin = target_code_fin();
}

void
Fin::do_opt_unit(OptUnit *unit)
{
    debug(3, "Processing procedure %s", get_name(unit).chars());

    // get ready to finalize the current unit
    code_fin->init(unit);

    prepare_unit(unit);
    analyze_unit();

    // assign variables to stack frame positions
    code_fin->layout_frame();

    // generate register saves and restores and related pseudo-ops
    code_fin->make_header_trailer();

    modify_unit();

    // perform final target-specific processing
    code_fin->finalize();
}

/*
 * When EGCS_BUG_DOESNT_MATTER is defined and this file is compiled
 * using EGCS-1.1.2, an internal compiler error results.
 */

#ifndef EGCS_BUG_DOESNT_MATTER
typedef SuifObject Container;
#endif

/*
 * Helpers for mapping a function over the instructions of the unit body,
 * whether it's in InstrList or Cfg form.  */
void
#ifdef EGCS_BUG_DOESNT_MATTER
map_instrs(InstrList *body, void (*per_instr)(InstrList*, InstrHandle))
#else
map_instrs(InstrList *body, void (*per_instr)(Container*, InstrHandle))
#endif
{
    for (InstrHandle h = start(body); h != end(body); /* */)
	per_instr(body, h++);
}

void
#ifdef EGCS_BUG_DOESNT_MATTER
map_instrs(Cfg *body, void (*per_instr)(CfgNode*, InstrHandle))
#else
map_instrs(Cfg *body, void (*per_instr)(Container*, InstrHandle))
#endif
{
    for (CfgNodeHandle nh = start(body); nh != end(body); ++nh)
	for (InstrHandle ih = start(*nh); ih != end(*nh); /* */)
	    per_instr(*nh, ih++);
}

/*
 * Helper that deletes one instruction if it's marked as a header or
 * trailer instruction.
 */
#ifdef EGCS_BUG_DOESNT_MATTER
template <class Container>
#endif
void
drop_header_trailer_instr(Container *container, InstrHandle h)
{
    claim(is_kind_of<InstrList>(container) || is_kind_of<CfgNode>(container));

    Instr *instr = *h;

    if (has_note(instr, k_header_trailer)) {
#ifdef EGCS_BUG_DOESNT_MATTER
	remove(container, h);
#else
	if (is_kind_of<InstrList>(container))
	    remove(to<InstrList>(container), h);
	else
	    remove(to<CfgNode>(container), h);
#endif
	delete instr;
    }
}

/*
 * prepare_unit -- Initialize unit-specific state.  Classify unit body as
 * CFG or instruction list.  Excise any instructions with header/trailer
 * annotations from a previous fin pass run.
 */
void
Fin::prepare_unit(OptUnit *unit)
{
    debug(3, "Processing unit %s", get_name(unit).chars());
    cur_unit = unit;

    AnyBody *body = get_body(unit);
    if (is_kind_of<InstrList>(body))
    {
	cur_cfg = NULL;
	cur_il = static_cast<InstrList*>(body);

#ifdef EGCS_BUG_DOESNT_MATTER
	map_instrs(cur_il, drop_header_trailer_instr<InstrList>);
#else
	map_instrs(cur_il, drop_header_trailer_instr);
#endif
    }
    else if (is_kind_of<Cfg>(body))
    {
	cur_il = NULL;
	cur_cfg = static_cast<Cfg*>(body);

#ifdef EGCS_BUG_DOESNT_MATTER
	map_instrs(cur_cfg, drop_header_trailer_instr<CfgNode>);
#else
	map_instrs(cur_cfg, drop_header_trailer_instr);
#endif
    }
    else
    {
	claim(false, "Body is neither an InstrList nor a CFG");
    }
}

/*
 * analyze_filter -- Filter used by analyze_unit to examine operands,
 * accumulating information needed to generate header and trailer code and
 * layout the stack frame.
 */
class AnalyzeFilter : public OpndFilter
{
  public:
    virtual Opnd operator()(Opnd opnd, InOrOut) {
	target_code_fin()->analyze_opnd(opnd);
	return opnd;
    }
};

/*
 * Helper that maps an AnalyzeFilter over the operands of an instruction.
 */
#ifdef EGCS_BUG_DOESNT_MATTER
template <class Container>
#endif
void
map_analyze_filter(Container*, InstrHandle h)
{
    AnalyzeFilter filter;
    map_opnds(*h, filter);
}

/*
 * analyze_unit -- Identify saved-registers non-parameter local variables
 * used in the current unit.  Record results in target_code_fin() by way of the
 * AnalyzeFilter object.
 */
void
Fin::analyze_unit()
{
#ifdef EGCS_BUG_DOESNT_MATTER
    if (cur_il != NULL)
	map_instrs(cur_il,  map_analyze_filter<InstrList>);
    else
	map_instrs(cur_cfg, map_analyze_filter<CfgNode>);
#else
    if (cur_il != NULL)
	map_instrs(cur_il,  map_analyze_filter);
    else
	map_instrs(cur_cfg, map_analyze_filter);
#endif
}

/*
 * ModifyFilter -- Filter used by modify_unit to replace address
 * operands involving local symbols by addresses in the stack frame.
 */
class ModifyFilter : public OpndFilter
{
  public:
    ModifyFilter() : OpndFilter(false) { }	// don't skip addr exps
    Opnd operator()(Opnd opnd, InOrOut)
	{ return target_code_fin()->replace_opnd(opnd); }
};

/*
 * Helper that performs the per-instruction part of modify_unit (see below).
 */
#ifdef EGCS_BUG_DOESNT_MATTER
template <class Container>
#endif
void
modify_instr(Container *container, InstrHandle h)
{
    Instr *mi = *h;

    if_debug(6)
	fprint(stderr, mi);

    // Find unit entry point and insert header code.  The convention is
    // that the list of header instructions is _backwards_.  We reverse
    // and insert the list after the proc-entry point.
    //
    if (!is_null(get_note(mi, k_proc_entry))) {
	InstrHandle entry_point = h;

	List<Instr*> &header = target_code_fin()->header();
	claim(!header.empty());

	while (!header.empty()) {
	    Instr *hmi = header.front();
	    header.pop_front();
	    set_note(hmi, k_header_trailer, note_flag());
#ifdef EGCS_BUG_DOESNT_MATTER
	    insert_after(container, entry_point, hmi);
#else
	    if (is_kind_of<InstrList>(container))
		insert_after(to<InstrList>(container), entry_point, hmi);
	    else
		insert_after(to<CfgNode>(container), entry_point, hmi);
#endif
	}
    }

    ModifyFilter filter;
    map_opnds(mi, filter);

    // Find each RET instruction by looking for its
    // k_incomplete_proc_exit annotation and insert trailer
    // code before it.
    //
    if (!is_null(get_note(mi, k_incomplete_proc_exit))) {
	InstrHandle exit_point = h;
	List<Instr*> &trailer = target_code_fin()->trailer();

	for (InstrHandle th = trailer.begin(); th != trailer.end(); ++th) {
	    Instr *tmi = deep_clone(*th);
	    set_note(tmi, k_header_trailer, note_flag());
#ifdef EGCS_BUG_DOESNT_MATTER
	    insert_before(container, exit_point, tmi);
#else
	    if (is_kind_of<InstrList>(container))
		insert_before(to<InstrList>(container), exit_point, tmi);
	    else
		insert_before(to<CfgNode>(container), exit_point, tmi);
#endif
	}
    }
}

/*
 * Replace local variables by stack references.  Use the frame map accessed
 * via the ModifyFilter.  Also insert header and trailer instructions, and
 * leave the header and trailer lists consumed.
 */
void
Fin::modify_unit()
{
    if_debug(6)
	fprintf(stderr, "Code before finalization:\n");

#ifdef EGCS_BUG_DOESNT_MATTER
    if (cur_il != NULL)
	map_instrs(cur_il,  modify_instr<InstrList>);
    else
	map_instrs(cur_cfg, modify_instr<CfgNode>);
#else
    if (cur_il != NULL)
	map_instrs(cur_il,  modify_instr);
    else
	map_instrs(cur_cfg, modify_instr);
#endif

    // Discard instructions from which inserted trailer code was cloned
    //
    List<Instr*> &trailer = target_code_fin()->trailer();
    while (!trailer.empty()) {
	delete trailer.front();
	trailer.pop_front();
    }

    claim(target_code_fin()->header().empty(), "Missing unit entry point");
}
