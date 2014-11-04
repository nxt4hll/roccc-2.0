/* file "ex3/ex3.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "ex3/ex3.h"
#endif

#include <machine/machine.h>

#include "ex3.h"

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

void
Ex3::initialize()
{
    printf("Running initialize()\n");

    // determine register to reserve
    int reserved_reg = -1;
    if (reserved_reg_name != empty_id_string) {
	// get abstract register number for user-specified reg name
	reserved_reg = reg_lookup(reserved_reg_name.chars());

    } else {
	// Since the user didn't specify a register to reserve, we use
	// the first temporary (caller-save) register.
	const NatSet *caller_saves = reg_caller_saves();
	claim(!caller_saves->is_empty(),
	      "No caller-save regs and so nothing to do");
	reserved_reg = caller_saves->iter().current();
    }

    // inspect reg width to get a generic type for this register
    int sz = reg_width(reserved_reg);
    switch (sz) {
      case  8: reserved_reg_type = type_u8;  break;
      case 16: reserved_reg_type = type_u16; break;
      case 32: reserved_reg_type = type_u32; break;
      case 64: reserved_reg_type = type_u64; break;
      default:
	claim(false, "unexpected register width = %d", sz);
    }

    // build a hard register operand
    reserved_reg_opnd = opnd_reg(reserved_reg, reserved_reg_type);

    // initialize note key variables for my annotations
    k_reserved_reg_load = "reserved_reg_LOAD";
    k_store_reserved_reg = "STORE_reserved_reg";
}


void
Ex3::do_opt_unit(OptUnit *unit)
{
    int i;

    IdString name = get_name(get_proc_sym(unit));
    printf("Processing procedure \"%s\"\n", name.chars());

    // get the body of the OptUnit
    AnyBody *body = get_body(unit);

    // verify that it is an InstrList
    claim (is_a<InstrList>(body),
           "expected OptUnit's body in InstrList form");
    InstrList *mil = (InstrList *)body;

    // build a stack location to hold the reserved register's contents
    VarSym *var_reserved_reg = new_unique_var(reserved_reg_type,
					      "_var_reserved_reg");
    // build an effective-address operand for var_reserved_reg
    Opnd ea_var_reserved_reg = opnd_addr_sym(var_reserved_reg);

    for (InstrHandle h = start(mil); h != end(mil); ++h) {
        Instr *mi = *h;

        if (is_label(mi) || is_dot(mi) || has_note(mi, k_store_reserved_reg))
	    continue;

	bool found_reserved_reg_as_src = false;
        
	for (i = 0; (i < srcs_size(mi) && !found_reserved_reg_as_src); i++) {
	    Opnd o = get_src(mi, i);
	    found_reserved_reg_as_src = is_reg(o) && (o == reserved_reg_opnd);
	}


	if (found_reserved_reg_as_src) {
	    
	    Instr *ld_tmp0;
	    ld_tmp0 = new_instr_alm(reserved_reg_opnd,
				    opcode_load(reserved_reg_type),
				    clone(ea_var_reserved_reg));
	    set_note(ld_tmp0, k_reserved_reg_load, note_flag());
	    insert_before(mil, h, ld_tmp0);

	}

	bool found_reserved_reg_as_dst = false;
        
	for (i = 0; (i < dsts_size(mi) && !found_reserved_reg_as_dst); i++) {
	    Opnd o = get_dst(mi, i);
	    found_reserved_reg_as_dst = is_reg(o) && (o == reserved_reg_opnd);
	}


	if (found_reserved_reg_as_dst) {
	    
	    Instr *st_tmp0;
	    st_tmp0 = new_instr_alm(clone(ea_var_reserved_reg),
				    opcode_store(reserved_reg_type),
				    reserved_reg_opnd);
	    set_note(st_tmp0, k_store_reserved_reg, note_flag());
	    insert_after(mil, h, st_tmp0);
	}
    }
}


void
Ex3::finalize()
{
    printf("Running finalize()\n");
}


