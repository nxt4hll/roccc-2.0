/* file "make_2opnd/make_2opnd.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "make_2opnd/make_2opnd.h"
#endif

#include <machine/machine.h>
#include <cfg/cfg.h>
#include <cfa/cfa.h>
#include <bvd/bvd.h>

#include <make_2opnd/make_2opnd.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif


void
Make_2opnd::initialize()
{
    debug(1, "Debug level is %d", debuglvl);
}

void
Make_2opnd::do_opt_unit(OptUnit *unit)
{
    IdString name = get_name(get_proc_sym(unit));
    debug(2, "Processing procedure \"%s\"", name.chars());

    // This pass requires a unit's body to be a CFG.  Be sure that a
    // preceding pass has left it in that form.

    claim(is_kind_of<Cfg>(get_body(unit)), "Body is not in CFG form");
    unit_cfg = static_cast<Cfg*>(get_body(unit));
    if_debug(5) {
	fprintf(stderr, "\nInitial CFG:\n");
	fprint(stderr, unit_cfg, false, true);		// no layout, just code
    }

    for (int i = 0; i < nodes_size(unit_cfg); ++i)
    {
	CfgNode *node = get_node(unit_cfg, i);
	for (InstrHandle h = start(node); h != end(node); /* */)
	{
	    InstrHandle this_h = h++;
	    Instr *instr = *this_h;

	    if (is_two_opnd(instr))
	    {
		Opnd d0 = get_dst(instr);
		Opnd s0 = get_src(instr, 0);

		if (s0 == d0)
		    continue;

		TypeId stype = get_type(s0);
		int move_opcode = opcode_move(stype);
		bool need_last_move = true;
		Opnd temp;

		if (is_reg(d0) || is_var(d0))
		{
		    // use dst reg as temp and skip last move
		    temp = d0;
		    need_last_move = false;
		}
		else
		{
		    temp = opnd_reg(stype);
		}

		// move s0 to temp
		insert_before(node, this_h,
			      new_instr_alm(temp, move_opcode, s0));

		// rewrite original instruction in 2-operand form
		set_src(instr, 0, temp);
		set_dst(instr, temp);

		// ensure result reaches true destination
		if (need_last_move)
		    insert_after(node, this_h,
				 new_instr_alm(d0, move_opcode, temp));
	    }
	}
    }


    if_debug(5) {
	fprintf(stderr, "\nFinal CFG:\n");
	fprint(stderr, unit_cfg, false, true);		// no layout, just code
    }
}
