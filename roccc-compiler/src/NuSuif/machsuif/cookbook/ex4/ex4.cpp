/* file "ex4/ex4.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "ex4/ex4.h"
#endif

#include <machine/machine.h>

#include "ex4.h"

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

void
Ex4::initialize()
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

    // inspect reg info to get a generic type for this register
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

class MyOpndFilter : public OpndFilter {
  public:
    MyOpndFilter(Opnd r) : reserved_reg_opnd(r),
        in_flag(false), out_flag(false) { }

    Opnd operator()(Opnd, InOrOut);

    bool found_reserved_reg_as_input() { return in_flag; }
    bool found_reserved_reg_as_output() { return out_flag; }

  protected:
    Opnd reserved_reg_opnd;
    bool in_flag, out_flag;
};

Opnd
MyOpndFilter::operator()(Opnd opnd, InOrOut in_or_out)
{
    if (is_reg(opnd) && (opnd == reserved_reg_opnd)) {
	// found a reference to the reserved reg; update flags
	in_flag  |= (in_or_out == IN);
	out_flag |= (in_or_out == OUT);
    }

    return opnd;
}

void
Ex4::do_opt_unit(OptUnit *unit)
{
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
	MyOpndFilter filter(reserved_reg_opnd);

        if (is_label(mi) || is_dot(mi) || has_note(mi, k_store_reserved_reg))
	    continue;

	map_opnds(mi, filter);

	if (filter.found_reserved_reg_as_input()) {
	    
	    Instr *ld_tmp0;
	    ld_tmp0 = new_instr_alm(reserved_reg_opnd,
				    opcode_load(reserved_reg_type),
				    clone(ea_var_reserved_reg));
	    set_note(ld_tmp0, k_reserved_reg_load, note_flag());
	    insert_before(mil, h, ld_tmp0);

	}

	if (filter.found_reserved_reg_as_output()) {
	    
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
Ex4::finalize()
{
    printf("Running finalize()\n");
}
