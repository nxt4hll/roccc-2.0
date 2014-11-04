/* file "x86/code_fin.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "x86/code_fin.h"
#endif

#include <machine/machine.h>

#include <x86/init.h>
#include <x86/opcodes.h>
#include <x86/code_fin.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

using namespace x86;

void
CodeFinX86::init(OptUnit *unit)
{
    CodeFin::init(unit);
    saved_reg_set.remove_all();
}

/*
 * analyze_opnd -- Accumulate the sets of SAV-convention registers
 * actually used in the program.  Identify and record non-parameter
 * automatic variables used in address operands.  (Parameters are handled
 * separately in layout_frame.)  Virtual and symbolic registers should have
 * been eliminated before code finalization.
 */
void
CodeFinX86::analyze_opnd(Opnd opnd)
{
    if (is_hard_reg(opnd)) {
	int r = get_reg(opnd);
	if (reg_callee_saves()->contains(r))
	    saved_reg_set.insert(reg_maximal(r));
    } else if (is_addr_sym(opnd)) {
	VarSym *v = (VarSym*)get_sym(opnd);
	if (is_a<VariableSymbol>(v) &&
	    is_auto(v) &&
	    frame_map.count(v) == 0) {
	    claim(!is_a<ParameterSymbol>(v));
	    frame_map[v] = 0;	// add v to map, but with no frame offset yet
	}
    } else {
	claim(!is_virtual_reg(opnd), "unallocated virtual register");
	claim(!is_var(opnd),	     "unallocated symbolic register");
    }
}

/*
 * layout_frame -- Assign a stack frame location to each variable
 * that needs one.  Store the offsets in frame_map.
 */
void
CodeFinX86::layout_frame()
{
    debug(4, "... determine offsets from %ebp for variables");

    frameoffset = 0;

    SymTable *st = cur_unit->get_symbol_table();
    Iter<SymbolTableObject*> iter = st->get_symbol_table_object_iterator();
    for ( ; iter.is_valid(); iter.next()) {
	SymbolTableObject *sto = iter.current();

	if (is_a<VarSym>(sto)) {
	    VarSym *v = (VarSym*)sto;

	    Map<Sym*, int>::iterator v_handle = frame_map.find(v);
	    if (v_handle == frame_map.end())
		continue;				// v never used

	    // Here v is an automatic variable, other than a parameter,
	    // that is actually used in the program.
	    // Allocate frame space in multiples of 4 bytes.  Otherwise,
	    // must use a different ld/st_to/from_stack routine.

	    int v_size = get_bit_size(get_type(v)) >> 3;// size of v in bytes
	    frameoffset -= (v_size + 3) & -4;		// alloc a multiple of 4

	    (*v_handle).second = frameoffset;		// update frame_map
	}
    }
    claim((-frameoffset & 3) == 0);	// better be aligned on 4-byte boundary

    // Process parameter list in order.  Initialize running offset p_offset
    // to skip over saved frame pointer and return address (four bytes each).
    //
    int p_offset = 8;
    debug(4, "... determine offsets from %ebp for parameters");

    for (int i = 0; i < get_formal_param_count(cur_unit); ++i) {
	VarSym *p = get_formal_param(cur_unit, i);
	int p_size = get_bit_size(get_type(p)) >> 3;	// in bytes

	// set %ebp offset -- always on 4-byte boundary
	claim(frame_map.count(p) == 0);

	// update running offset, rounding up to a 4-byte boundary
	frame_map[p] = p_offset;
	p_offset += (p_size + 3) & -4;
    }

    // Calculate the final framesize value.
    //
    framesize = -frameoffset;
}

/*
 * replace_opnd -- If argument is an address that uses a stack variable,
 * return a replacement frame address that indexes from the stack pointer.
 * Otherwise, return the original operand.
 */
Opnd
CodeFinX86::replace_opnd(Opnd opnd)
{
    if (is_addr_sym(opnd)) {
	return frame_addr(opnd, opnd, 0);
    } else if (SymDispOpnd sdo = opnd) {
	int offset = get_immed_integer(sdo.get_disp()).c_int();
	return frame_addr(opnd, sdo.get_addr_sym(), offset);
    }
    return opnd;
}

/*
 * frame_addr -- Subfunction of replace_opnd taking an address-symbol
 * operand and a `delta' relative to that symbol.  If the symbol has a
 * frame slot, return a replacement address that indexes from the frame
 * pointer (%ebp).  The displacement in this address combines the symbol-
 * relative delta with the frame offset of the symbol.  The latter is a
 * number stored in frame_map that may be positive (for stack-passed formal
 * parameter symbols) or negative (for other locals).  The combined %ebp
 * displacement should be non-negative.
 *
 * If the symbol is not in the frame, return the orig(inal) operand.  */
Opnd
CodeFinX86::frame_addr(Opnd orig, Opnd adr_sym, int delta)
{
    VarSym *v = (VarSym*)get_sym(adr_sym);
    if (frame_map.count(v) == 0)
	return orig;

    delta += frame_map[v];

    Opnd disp = opnd_immed(delta, type_s32);
    return BaseDispOpnd(opnd_reg_fp, disp, get_deref_type(orig));
}

/*
 * make_header_trailer() -- Build the instruction lists for the procedure
 * entry and exits.  Note that not all of the header is built here--it is
 * completed in printmachine.  The header instructions are placed on a list
 * in reverse order so that we can later "pop" them easily into the
 * instruction stream after the proc_entry point.  The trailer instructions
 * are also placed on a list, to be inserted before returns marked with
 * k_incomplete_proc_exit notes.
 */
// FIXME:
#define push_dst (BaseDispOpnd(opnd_reg_sp, opnd_immed(-4, type_s32)))
#define pop_src  (BaseDispOpnd(opnd_reg_sp, opnd_immed(0, type_s32)))

void
CodeFinX86::make_header_trailer()
{
    Instr *mi;

    clear(header());				// begin collecting header code
    clear(trailer());				//   "        "     trailer  "

    // create -- 				push %ebp
    mi = new_instr_alm(push_dst, PUSH, opnd_reg_fp);
    set_dst(mi, 1, opnd_reg_sp);
    header().push_front(mi);

    // create -- 				mov %esp, %ebp
    mi = new_instr_alm(opnd_reg_fp, MOV, opnd_reg_sp);
    header().push_front(mi);

    // create, if needed --			sub %esp,framesize
    if (framesize) {
	mi = new_instr_alm(opnd_reg_sp, SUB, opnd_reg_sp,
			   opnd_immed(framesize, type_s32));
	set_dst(mi, 1, opnd_reg_eflags);
	header().push_front(mi);
    }

    // Store general-purpose saved registers that need saving
    if (!saved_reg_set.is_empty()) {
	NatSetIter gpsrs(saved_reg_set.iter());
	for ( ; gpsrs.is_valid(); gpsrs.next()) {
	    int r = gpsrs.current();
	    // emit a push of r before the others done so far, and
	    // emit a pop  of r after  the others done so far
	    mi = new_instr_alm(push_dst, PUSH, opnd_reg(r, type_u32));
	    set_dst(mi, 1, opnd_reg_sp);
	    header().push_front(mi);
	    mi = new_instr_alm(opnd_reg(r, type_u32), POP, pop_src);
	    set_dst(mi, 1, opnd_reg_sp);
	    trailer().push_front(mi);
	}
    }

    // final trailer instruction for procedure exit
    mi = new_instr_alm(LEAVE);
    trailer().push_back(mi);
}
