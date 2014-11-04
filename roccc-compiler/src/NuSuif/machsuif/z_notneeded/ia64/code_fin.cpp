/* file "ia64/code_fin.cpp" */

/*
    Copyright (c) 2002 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "ia64/code_fin.h"
#endif

#include <ctype.h>		// (isdigit)
#include <machine/machine.h>
#include <cfg/cfg.h>

#include <ia64/init.h>
#include <ia64/opcodes.h>
#include <ia64/reg_info.h>
#include <ia64/code_fin.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

using namespace ia64;

void
CodeFinIa64::init(OptUnit *unit)
{
    CodeFin::init(unit);

    max_in_reg	  = GR_STACK0 - 1;
    max_local_reg = GR_STACK0 - 1;
    max_out_reg   = GR_OUT0 - 1;

    va_first_var  = NULL;

    for (int i = CLASS_GR; i <= CLASS_PR; ++i)
	saved_reg_set[i].remove_all();
}

/*
 * analyze_opnd:
 *
 * Accumulate the sets of preserved (SAV) registers actually used in the
 * program.  Don't count the automatically-stacked GRs, because they don't
 * need manual spilling/filling.
 *
 * Record the largest-numbered local (stacked) register seen and the largest-
 * numbered out register seen.
 *
 * Identify and record automatic variables other than stack-passed parameters
 * that are used in address operands.  (Stack-passed parameters live in the
 * caller's frame.  They are handled separately in layout_frame.)
 *
 * Check that virtual and symbolic registers have been eliminated before
 * code finalization.
 */
void
CodeFinIa64::analyze_opnd(Opnd opnd)
{
    if (is_hard_reg(opnd)) {
	int r = get_reg(opnd);

	int bank = ((r >= GR_SAV0) && (r <= GR_LAST_SAV)) ? CLASS_GR
	         : ((r >= FR_SAV0) && (r <= FR_LAST_SAV)) ? CLASS_FR
	         : ((r >= PR_SAV0) && (r <= PR_LAST_SAV)) ? CLASS_PR
	         : ((r >= BR_SAV0) && (r <= BR_LAST_SAV)) ? CLASS_BR
	         : -1;
	if (bank >= 0) {					// preserved
	    if ((r >= GR_STACK0) && (r <= GR_LAST_STACK)) {	// automatic
		if (r > max_local_reg)
		    max_local_reg = r;
	    } else {						// manual save
		saved_reg_set[bank].insert(r);
	    }
	} else if ((r >= GR_OUT0) && (r <= GR_LAST_OUT)) {
	    if (r > max_out_reg)
		max_out_reg = r;
	}
    } else if (is_addr_sym(opnd)) {
	VarSym *v = (VarSym*)get_sym(opnd);
	if (is_kind_of<VarSym>(v) &&
	    is_auto(v) &&
	    frame_map.count(v) == 0 &&
	    (!is_kind_of<ParameterSymbol>(v) || is_reg_param(v))) {
	    frame_map[v] = 0;	// add v to map, but with no frame offset yet
	    if (has_note(v, k_va_first_unnamed)) {
		claim(!va_first_var, "Multiple va_first_unnamed occurrences");
		va_first_var = v;
	    }
	}
    } else {
	claim(!is_virtual_reg(opnd), "unallocated virtual register");
	claim(!is_var(opnd),	     "unallocated symbolic register");
    }
}

/*
 * layout_frame -- Assign stack frame locations to each variable that needs one.
 * Store the offsets in frame_map.
 *
 * A local variable needs a memory-stack location unless it is never used or it
 * is a memory-passed parameter.
 */
void
CodeFinIa64::layout_frame()
{
    debug(4, "... determine offsets from $sp for variables");

    // Force max_arg_area to be a 16-byte multiple to be sure that the local-
    // storage area starts on a 16-byte boundary.

    max_arg_area = (max_arg_area + 15) & -16;

    // Unless this is a leaf procedure, reserve a 16-byte scratch area for
    // callees at the young end of the current frame.

    int scratch_area_size = is_leaf ? 0 : 16;

    // Frame_offset is the running offset of the current variable in the
    // local-storage area.  Initialize it to the distance between the young
    // end of the frame and the local-storage area.

    frame_offset = scratch_area_size + max_arg_area;

    SymTable *st = cur_unit->get_symbol_table();
    Iter<SymbolTableObject*> iter = st->get_symbol_table_object_iterator();
    for ( ; iter.is_valid(); iter.next()) {
	SymbolTableObject *sto = iter.current();

	if (is_kind_of<VarSym>(sto)) {
	    VarSym *v = (VarSym*)sto;

	    if (!is_reg_param(v) && is_a<ParameterSymbol>(v))
		continue;				// v's in caller's frame

	    Map<Sym*,int>::iterator v_handle = frame_map.find(v);
	    if (v_handle == frame_map.end())
		continue;				// v never used

	    // Here v is an automatic variable, other than a stack-passed
	    // parameter, that is actually used in the program.  First,
	    // adjust frame_offset to accommodate v'a alignment.  The
	    // frame_offset is already a multiple of four bytes.  An
	    // alignment value greater than four bytes will itself be a
	    // multiple of four.  Round frame_offset up if necessary to
	    // satisfy that alignment constraint.

	    TypeId v_type = get_type(v);
	    int v_align = get_bit_alignment(v_type) >> 3; // in bytes
	    if (v_align > 4) {
		claim(v_align % 4 == 0);
		frame_offset =
		    ((frame_offset + v_align - 1) / v_align) * v_align;
	    }
	    (*v_handle).second = frame_offset;		// update frame_map

	    // Now allocate a multiple of four bytes to v.

	    int v_size = get_bit_size(v_type) >> 3;	// v's size in bytes
	    frame_offset += (v_size + 3) & -4;
	}
    }

    // Compute number of bytes for registers saved in memory between locals
    // and the frame base.
    
    save_area_size = saved_reg_set[CLASS_GR].size() * 8 +
		     saved_reg_set[CLASS_BR].size() * 8 +
		     saved_reg_set[CLASS_FR].size() * 16;

    // The sum of local-area and save-area sizes must be a multiple of 16.
    // Frame_offset is now the local-area size.  Pad it to make the sum a
    // 16-byte multiple.  (Hint: save_area_size is already a multiple of 8.)

    claim((frame_offset & 3) == 0);			// now a 4-byte multiple
    frame_offset = (frame_offset + (save_area_size & 15) + 15) & -16;

    debug(4, "... determine offsets from $sp for memory-passed parameters");

    // Process parameter list in order.  Set running offset param_offset for
    // memory-passed parameters.  Also determine the highest GR arg register
    // used.
    // FIXME: does not allow for aggregates passed partly in regs and partly
    // in memory.

    int param_offset = frame_offset + save_area_size;
    if (param_offset < (scratch_area_size + 16))
	param_offset = (scratch_area_size + 16);

    for (int i = 0; i < get_formal_param_count(cur_unit); i++) {
	VarSym *p = get_formal_param(cur_unit, i);

	// Each parameter consumes a multiple of 8 bytes.

	int p_size = get_bit_size(get_type(p)) >> 3;	// in bytes
	p_size = (p_size + 7) & -8;

	if (is_reg_param(p)) {
	    int first_reg = get_param_reg(p);

	    if (GR_ARG0 <= first_reg && first_reg <= GR_LAST_ARG) {
		claim(first_reg > max_in_reg);

		max_in_reg = first_reg + (p_size / 8) - 1;
		if (max_in_reg > GR_LAST_ARG)
		    max_in_reg = GR_LAST_ARG;
	    }
	} else {
	    claim(frame_map.count(p) == 0);

	    frame_map[p] = param_offset;
	    param_offset += p_size;
	}
    }

    // In a varargs procedure, the GR parameter registers that are not used
    // by named arguments must be spilled to memory adjacent to any
    // parameters passed in memory by the caller.  So in the varargs case,
    // the first thing at the old end of the frame (beginning actually in
    // the caller's scratch area) is a varargs spill area.  Note that we
    // increase max_in_reg to cover any reg-passed, unnamed varargs.

    if (is_varargs) {
	va_area_size = 64 - 8 * (max_in_reg - GR_STACK0 + 1);
	max_in_reg = GR_LAST_ARG;
    } else {
	va_area_size = 0;
    }

    // Frame size is the sum of the sizes of the scratch area, the
    // outgoing-arg area, the local-storage area, the callee-saves-register
    // area, and the register-passed-varargs area.  (The first three have
    // already been combined in frame_offset.)  The sum of frame_offset and
    // save_area_size is already a 16-byte multiple, but va_area_size may
    // not be, so pad to bring frame_size to a multiple of 16 bytes.

    frame_size = frame_offset + save_area_size + ((va_area_size + 15) & -16);

    // For a procedure that invokes va_start, bind the variable whose symbol
    // is stored in va_first_var to the offset from SP of the first unnamed
    // argument, whether passed in memory (va_area_size == 0) or in a register
    // that has been dumped to memory (va_area_size > 0).

    claim((va_first_var == NULL) == (is_varargs == false));
    if (is_varargs)
	frame_map[va_first_var] = 
	    (va_area_size == 0) ? param_offset
				: (frame_size - (va_area_size - 16));
}

/*
 * replace_opnd -- If argument is an address that uses a stack variable,
 * return a replacement frame address that indexes from the stack pointer.
 * Otherwise, return the original operand.
 */
Opnd
CodeFinIa64::replace_opnd(Opnd opnd)
{
    if (is_addr_sym(opnd)) {
	return frame_addr(opnd, opnd, 0);
    } else if (SymDispOpnd sdo = opnd) {
	int offset = get_immed_int(sdo.get_disp());
	return frame_addr(opnd, sdo.get_addr_sym(), offset);
    }

    return opnd;
}

/*
 * frame_addr -- Subfunction of replace_opnd taking an address-symbol operand
 * and a `delta' relative to that symbol.  If the symbol has a frame slot,
 * return an immediate operand representing an offset from the stack pointer.
 * This offset is the sum of the symbol-relative delta and the offset from SP
 * of the start of the symbol's value in the memory frame.  The combined
 * offset must be non-negative.
 *
 * If the symbol is not in the frame, return the orig(inal) operand.
 */
Opnd
CodeFinIa64::frame_addr(Opnd orig, Opnd addr_sym, int delta)
{
    VarSym *v = (VarSym*)get_sym(addr_sym);
    if (frame_map.count(v) == 0)
	return orig;

    delta += frame_map[v];
    claim(delta >= 0);

    return opnd_immed(delta, type_s64);
}

/*
 * make_header_trailer() -- Build the instruction lists for the procedure
 * entry and exits.  The header instructions are placed on a list
 * in reverse order so that we can later "pop" them easily into the
 * instruction stream after the proc_entry point.  The trailer instructions
 * are also placed on a list, to be inserted before returns marked with
 * k_incomplete_proc_exit notes.
 */
void
CodeFinIa64::make_header_trailer()
{
    int opcode_prologue	 = make_opcode(PROLOGUE, EMPTY,EMPTY,EMPTY);
    int opcode_spill	 = make_opcode(SPILL,	 EMPTY,EMPTY,EMPTY);
    int opcode_memoffset = make_opcode(MEMOFFSET,EMPTY,EMPTY,EMPTY);
    int opcode_body	 = make_opcode(BODY,	 EMPTY,EMPTY,EMPTY);
    int opcode_alloc	 = make_opcode(ALLOC,	 EMPTY,EMPTY,EMPTY);
    int opcode_fframe	 = make_opcode(FFRAME,	 EMPTY,EMPTY,EMPTY);
    int opcode_save	 = make_opcode(SAVE,	 EMPTY,EMPTY,EMPTY);
    int opcode_restore	 = make_opcode(RESTORE,	 EMPTY,EMPTY,EMPTY);
    int opcode_add	 = make_opcode(ADD,	 EMPTY,EMPTY,EMPTY);
    int opcode_mov_ar	 = make_opcode(MOV_AR,	 EMPTY,EMPTY,EMPTY);
    int opcode_mov_br	 = make_opcode(MOV_BR,	 EMPTY,EMPTY,EMPTY);
    int opcode_mov_pr	 = make_opcode(MOV_PR,	 EMPTY,EMPTY,EMPTY);
    int opcode_ldf_fill	 = make_opcode(LDF, EMPTY, _LDTYPE_FILL,  LDHINT_NONE);
    int opcode_ld8_fill	 = make_opcode(LD,  SZ_8,  _LDTYPE_FILL,  LDHINT_NONE);
    int opcode_stf_spill = make_opcode(STF, EMPTY, _STTYPE_SPILL, STHINT_NONE);
    int opcode_st8_spill = make_opcode(ST,  SZ_8,  _STTYPE_SPILL, STHINT_NONE);

    bool save_unat = (va_area_size > 0) || !saved_reg_set[CLASS_GR].is_empty();

    int r_pfs	= 1 + (max_local_reg < max_in_reg ? max_in_reg : max_local_reg);
    int r_rp	= 1 + r_pfs;
    int r_pr	= 1 + r_rp;
    int r_unat	= save_unat ? (1 + r_pr) : 0;
    int n_in	= max_in_reg - GR_STACK0 + 1;		// incoming arg regs
    int n_local = (r_unat ? r_unat : r_pr)-GR_STACK0+1; // locals & saved state
    int n_out	= max_out_reg - GR_OUT0 + 1;		// outgoing arg regs
    int d_save	= frame_size + 16 - save_area_size;	// sp offset of 1st save
    int d_spill	= va_area_size - 16;			// psp - save-area end
    Opnd o_ar_pfs   = opnd_reg(AR_PFS,	  type_v0);	// reg ar.pfs itself
    Opnd o_ar_unat  = opnd_reg(AR_UNAT,	  type_v0);	// reg ar.unat itself
    Opnd o_pfs	    = opnd_reg(r_pfs,	  type_v0);	// reg to save ar.pfs
    Opnd o_rp	    = opnd_reg(r_rp,	  type_v0);	// reg to save RA
    Opnd o_pr	    = opnd_reg(r_pr,	  type_v0);	// reg to save all PR
    Opnd o_unat	    = opnd_reg(r_unat,	  type_v0);	// reg to save ar.unat
    Opnd o_rp_token = opnd_reg(RP_TOKEN,  type_v0);	// phony arg in .save rp
    Opnd o_pr_token = opnd_reg(PR_TOKEN,  type_v0);	// phony arg in .save pr
    Opnd o_sp_token = opnd_reg(SP_TOKEN,  type_v0);	// phony for .restore sp
    Opnd o_ra	    = opnd_reg(BR_RA,	  type_v0);	// actual BR b0
    Opnd o_ptr	    = opnd_reg(GR_TMP0,	  type_p64);	// index into save area
    Opnd o_temp	    = opnd_reg(GR_TMP0+1, type_p64);	// scratch register
    Opnd o_in	    = opnd_immed(n_in,	  type_s64);
    Opnd o_local    = opnd_immed(n_local, type_s64);
    Opnd o_out	    = opnd_immed(n_out,	  type_s64);
    Opnd o_size	    = opnd_immed( frame_size, type_s64);
    Opnd o_neg_size = opnd_immed(-frame_size, type_s64);
    Opnd o_0	    = opnd_immed_0_u64;
    Opnd o_neg_1    = opnd_immed(-1,	  type_s64);
    Opnd o_neg_8    = opnd_immed(-8,	  type_s64);
    Opnd o_neg_16   = opnd_immed(-16,	  type_s64);
    Opnd o_save	    = opnd_immed(d_save,  type_s64);	// sp offset of 1st save
    Opnd o_spill    = opnd_immed(d_spill, type_s64);	// psp - save-area end
    Opnd o_at_ptr   = BaseDispOpnd(o_ptr, o_0);		// addr for spill/fill

    Instr *mi;

    clear(header());				// begin collecting header code

    mi = new_instr_dot(opcode_prologue);
    header().push_front(mi);

    // The .spill directive declares the distance from the previous stack
    // pointer (psp) to the save area.  This may be negative, since the save
    // area usually starts in the callers scratch area.

    if (save_area_size > 0) {
	mi = new_instr_dot(opcode_spill, o_spill);
	header().push_front(mi);
    }

    mi = new_instr_dot(opcode_save, o_ar_pfs, o_pfs);
    header().push_front(mi);
    mi = new_instr_alm(o_pfs, opcode_alloc, o_ar_pfs, o_in);
    set_src(mi, 2, o_local);
    set_src(mi, 3, o_out);
    set_src(mi, 4, o_0);
    header().push_front(mi);
    mi = new_instr_dot(opcode_fframe, o_size);
    header().push_front(mi);
    mi = new_instr_alm(opnd_reg_sp, opcode_add, o_neg_size, opnd_reg_sp);
    header().push_front(mi);
    mi = new_instr_dot(opcode_save, o_rp_token, o_rp);
    header().push_front(mi);
    mi = new_instr_alm(o_rp, opcode_mov_br, o_ra);
    header().push_front(mi);
    mi = new_instr_dot(opcode_save, o_pr_token, o_pr);
    header().push_front(mi);
    mi = new_instr_alm(o_pr, opcode_mov_pr);
    header().push_front(mi);

    // FIXME: the reuse of a single address opnd (o_at_ptr) instead of
    // cloning it for each occurrence is bad style.

    // If GRs will be spilled using st8.spill, then save and restore ar.unat.

    if (save_unat) {
	mi = new_instr_dot(opcode_save, o_ar_unat, o_unat);
	header().push_front(mi);			// .save ar.unat, r_unat
	mi = new_instr_alm(o_unat, opcode_mov_ar, o_ar_unat);
	header().push_front(mi);			// mov r_unat = ar.unat
    }

    // FIXME: we need .save.f and so on in front of each store that saves
    // a callee-saves register.

    if (save_area_size > 0) {
	mi = new_instr_alm(o_ptr, opcode_add, o_save, opnd_reg_sp);
	header().push_front(mi);

	NatSetIter git(saved_reg_set[CLASS_GR].iter());
	for ( ; git.is_valid(); git.next()) {
	    int r = git.current();
	    Opnd o_savee = opnd_reg(r, type_u64);
	    mi = new_instr_alm(o_at_ptr, opcode_st8_spill, o_savee, o_neg_8);
//FIXME	    set_dst(mi, 1, o_ptr);			// auto-increment dst
	    header().push_front(mi);
	}
	NatSetIter bit(saved_reg_set[CLASS_BR].iter());
	for ( ; bit.is_valid(); bit.next()) {
	    int r = bit.current();
	    Opnd o_savee = opnd_reg(r, type_p64);
	    mi = new_instr_alm(o_temp, opcode_mov_br, o_savee);
	    header().push_front(mi);
	    mi = new_instr_alm(o_at_ptr, opcode_st8_spill, o_temp, o_neg_8);
//FIXME	    set_dst(mi, 1, o_ptr);			// auto-increment dst
	    header().push_front(mi);
	}
	NatSetIter fit(saved_reg_set[CLASS_FR].iter());
	for ( ; fit.is_valid(); fit.next()) {
	    int r = fit.current();
	    Opnd o_savee = opnd_reg(r, type_f128);
	    mi = new_instr_alm(o_at_ptr, opcode_stf_spill, o_savee, o_neg_16);
//FIXME	    set_dst(mi, 1, o_ptr);			// auto-increment dst
	    header().push_front(mi);
	}
    }

    // If there are unnamed arg registers to spill, emit a sequence that begins
    //
    //   add r2 = frame_size+8, sp
    //   .mem.offset -8, 0
    //   st8.spill [r2] = r39, -8			// highest GR arg reg
    //
    // and continues with other .mem.offset/st8.spill pairs, increasing the
    // offset literal by 8 and decreasing the arg-register number by 1 for each.
    // (The value of the offset operand to .mem.offset is probably arbitrary, as
    // long as it differs for different spills.)

    if (va_area_size > 0) {
	Opnd o_scratch = opnd_immed(frame_size + 8, type_s64);

	mi = new_instr_alm(o_ptr, opcode_add, o_scratch, opnd_reg_sp);
	header().push_front(mi);			// add r2 = ...+8, sp

	int va_reg = GR_LAST_ARG;			// vararg reg to spill
	int d_psp = -8;					// psp - spill address

	do {
	    Opnd o_psp = opnd_immed(d_psp, type_s64);
	    mi = new_instr_dot(opcode_memoffset, o_psp, o_0);
	    header().push_front(mi);			// .mem.offset offset, 0

	    Opnd o_va_reg = opnd_reg(va_reg, type_u64);
	    mi = new_instr_alm(o_at_ptr, opcode_st8_spill, o_va_reg, o_neg_8);
//FIXME	    set_dst(mi, 1, o_ptr);			// auto-increment dst
	    header().push_front(mi);			// st8.spill [r2] = r,-8

	    --va_reg;
	    d_psp += 8;
	} while (d_psp <= va_area_size - 16);
    }

    mi = new_instr_dot(opcode_body);
    header().push_front(mi);

    // Create trailer instructions for procedure exit.	Restore saved regs.

    clear(trailer());

    if (save_unat) {
	mi = new_instr_alm(o_ar_unat, opcode_mov_ar, o_unat);
	trailer().push_back(mi);			// mov ar.unat = r_unat
    }

    if (save_area_size > 0) {
	mi = new_instr_alm(o_ptr, opcode_add, o_save, opnd_reg_sp);
	trailer().push_back(mi);

	NatSetIter git(saved_reg_set[CLASS_GR].iter());
	for ( ; git.is_valid(); git.next()) {
	    int r = git.current();
	    Opnd o_savee = opnd_reg(r, type_u64);
	    mi = new_instr_alm(o_savee, opcode_ld8_fill, o_at_ptr, o_neg_8);
//FIXME	    set_dst(mi, 1, o_ptr);			// auto-increment dst
	    trailer().push_back(mi);
	}
	NatSetIter bit(saved_reg_set[CLASS_BR].iter());
	for ( ; bit.is_valid(); bit.next()) {
	    int r = bit.current();
	    mi = new_instr_alm(o_temp, opcode_ld8_fill, o_at_ptr, o_neg_8);
//FIXME	    set_dst(mi, 1, o_ptr);			// auto-increment dst
	    trailer().push_back(mi);
	    Opnd o_savee = opnd_reg(r, type_p64);
	    mi = new_instr_alm(o_savee, opcode_mov_br, o_temp);
	    trailer().push_back(mi);
	}
	NatSetIter fit(saved_reg_set[CLASS_FR].iter());
	for ( ; fit.is_valid(); fit.next()) {
	    int r = fit.current();
	    Opnd o_savee = opnd_reg(r, type_f128);
	    mi = new_instr_alm(o_savee, opcode_ldf_fill, o_at_ptr, o_neg_16);
//FIXME	    set_dst(mi, 1, o_ptr);			// auto-increment dst
	    trailer().push_back(mi);
	}
    }

    mi = new_instr_alm(o_ar_pfs, opcode_mov_ar, o_pfs);
    trailer().push_back(mi);
    mi = new_instr_alm(o_ra, opcode_mov_br, o_rp);
    trailer().push_back(mi);
    mi = new_instr_alm(opcode_mov_pr, o_pr, o_neg_1);
    trailer().push_back(mi);
    if (save_unat) {
	mi = new_instr_alm(o_ar_unat, opcode_mov_ar, o_unat);
	trailer().push_back(mi);			// mov ar.unat = r_unat
    }
    mi = new_instr_dot(opcode_restore, o_sp_token);
    trailer().push_back(mi);
    mi = new_instr_alm(opnd_reg_sp, opcode_add, o_size, opnd_reg_sp);
    trailer().push_back(mi);
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
 * Helper that expands an add-immediate instruction if the immediate
 * value lies outside the range [-8192, 8191].
 */
#ifdef EGCS_BUG_DOESNT_MATTER
template <class Container>
#endif
void
maybe_expand_addi(Container *container, InstrHandle h)
{
    claim(is_kind_of<InstrList>(container) || is_kind_of<CfgNode>(container));

    Instr *instr = *h;
    int opcode = get_opcode(instr);
    int stem = get_stem(opcode);


    if (stem == ADD) {
	Opnd src0 = get_src(instr, 0);
	if (is_immed_integer(src0)) {
	    Integer value = get_immed_integer(src0);
	    if (value > 8191 || value < -8192) {
		Opnd r11 = opnd_reg(GR_LAST_RET, type_s64);
		int opcode = make_opcode(MOVL, EMPTY,EMPTY,EMPTY);
		Instr *movl = new_instr_alm(r11, opcode, src0);

#ifdef EGCS_BUG_DOESNT_MATTER
		insert_before(container, h, movl);
#else
		if (is_kind_of<InstrList>(container))
		    insert_before(to<InstrList>(container), h, movl);
		else
		    insert_before(to<CfgNode>(container), h, movl);
#endif
		set_src(instr, 0, r11);
	    }
	}
    }
}

/*
 * Run a postpass to expand add-immediate instructions whose immediate
 * value is too large for a 14-bit immediate field.
 */
void
CodeFinIa64::finalize()
{
    AnyBody *body = get_body(cur_unit);
    if (is_kind_of<InstrList>(body))
    {
	InstrList *cur_il = static_cast<InstrList*>(body);

#ifdef EGCS_BUG_DOESNT_MATTER
	map_instrs(cur_il, maybe_expand_addi<InstrList>);
#else
	map_instrs(cur_il, maybe_expand_addi);
#endif
    }
    else if (is_kind_of<Cfg>(body))
    {
	Cfg *cur_cfg = static_cast<Cfg*>(body);

#ifdef EGCS_BUG_DOESNT_MATTER
	map_instrs(cur_cfg, maybe_expand_addi<CfgNode>);
#else
	map_instrs(cur_cfg, maybe_expand_addi);
#endif
    }
    else
    {
	claim(false, "Body is neither an InstrList nor a CFG");
    }
}
