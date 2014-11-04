/* file "alpha/translate_call.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#include <machine/machine.h>
#include <suifvm/suifvm.h>

#include <alpha/code_gen.h>
#include <alpha/opcodes.h>
#include <alpha/reg_info.h>
#include <alpha/init.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

using namespace alpha;

/*
 * This module contains the routines to translate SUIF call and return
 * instructions (CALL and RET) into the appropriate Alpha assembly
 * language instructions and assembler pseudo-ops.
 *
 * The stack in an ALPHA machine grows down, all references into the stack
 * frame are with positive offsets from the stack pointer which is decremented
 * upon entry into the procedure.  The stack frames must be aligned on a
 * 16-byte boundary.  The variable framesize includes all space in stack
 * frame for this procedure: local & temps, saved registers, and maximum
 * argument space.  The variable frameoffset is a negative number that
 * indicates the distance in bytes from the virtual frame pointer to
 * the point where the registers are saved.
 *
 * The stack frame contains the following:
 *
 *   0xf..f
 *
 *		parameter home area for this procedure's reg-passed parameters
 *		local variables
 *		saved registers
 *		    saved fp in order
 *		    saved int high-number
 *		    saved int low-number
 *		    $26 (aka $ra)
 *   		arguments passed in memory to called proc (nth downto 7th)
 *		    nth arg
 *		    ...
 *   $sp -->	    7th arg
 *
 *   0x0
 *
 *
 * The variable max_arg_area specifies what I currently think the largest
 * argument area is.  In ALPHA, the space on the stack taken by the argument
 * area is (max_arg_area - (8 bytes * 6)), i.e. it can be 0 bytes long if
 * all parameters are passed in registers (unlike MIPS, no spill space need
 * be allocated by the caller for the parameter registers).  If the procedure
 * returns a structure, max_arg_area increases by 8 bytes to include first
 * parameter containing return structure address.
 *
 * The code generator also assumes that all data items smaller than a
 * quadword (8 bytes) are converted to quadword size objects before being
 * used as parameters in a call.
 *
 * A structure move is handled in a special way by the code generator.  It
 * first collects the definition and use of a virtual register containing a
 * structure and generates a special cg_lrec and cg_srec instruction.  These
 * instructions are ALWAYS contiguous so that the register allocator can
 * lookahead only a single instruction to do the expansion.  Notice that
 * I generate a cg_srec into parameter space for a cg_lrec that is used
 * by a call instruction.
 *
 * Currently, agen only recognizes a subset of the possible arithmetic
 * exceptions.  Specifically, the integer overflow and integer divide-by-0
 * exceptions; handling of the IEEE exceptions is an unfinished project.
 * The handling of integer overflow is taken care of by agen.  The default
 * is to ignore overflow and trap on divide-by-0.
 *
 * Varargs procedures can be handled by agen.  It assumes that any
 * procedure containing the builtin call __builtin_va_start is a
 * varargs (or stdarg) procedure.  In this case, all parameter registers
 * from the point after the last formal parameter are spilled to the
 * stack.  The second parameter to __builtin_va_start must also be marked
 * as ``addr_taken'' since the expansion of __builtin_va_start takes
 * the address of this parameter.  The spilling is done in Pass2().
 */

/*
 * PROCEDURE:	Translate_call
 *
 * DESCRIPTION:
 *
 * Translates a suifvm call instruction into the corresponding
 * Compaq/Digital Alpha assembly instructions.  This routine first
 * generates Alpha assembly instructions to setup the parameters to be
 * passed, both in the parameter registers and on the stack.  It then
 * generates the call instruction, and finally moves the return value
 * (if any) to the correct abstract register.
 *
 * This routine also makes sure that is_leaf is false and expands the
 * procedure's argument area if necessary.  Note that if the procedure
 * has_parameters and is_leaf is false or is_varargs is true, then the
 * parameter home area for this procedure must include space for a minimum
 * of 6 64-bit longwords.
 *
 * This routine also assumes that the parameters are listed in the order 0
 * to N-1.  It does NOT check for this fact.
 *
 * Parameter passing registers are:
 *	$16-$21		general
 *      $f16-$f21	floating-point
 */
void
CodeGenAlpha::translate_cal(Instr *cal)
{
    Instr *mi;

    int argi;			// argument index
    int num_args = srcs_size(cal) - 1;

    // The number of argument regs is the same for both GPR and FPR banks.
    int num_arg_regs = LAST_ARG_GPR - ARG0_GPR + 1;

    // Stack argument area needed by this call.  Starts negative to
    // reflect the fact that the first 6 arguments are passed in
    // registers and saved in callee's stack frame space.
    int arg_area = -num_arg_regs * 8;

    // Abstract numbers of next free FPR and GPR arg registers, resp.
    int next_freeAR_fpr = ARG0_FPR,
	next_freeAR_gpr = ARG0_GPR;

    // Set of registers possibly defined at the point of call.  To be
    // attached to the call as a regs_defd note for use by DFA.  Killed
    // registers include all those in the caller-saved convention plus the
    // assembler temporaries.
    static NatSetDense regs_killed;
    if (regs_killed.size() == 0) {
	regs_killed = *reg_caller_saves();
        regs_killed.insert(GP);
	for (int i = ASM_TMP0; i <= LAST_ASM_TMP; ++i)
	    regs_killed.insert(i);
    }

    // Set of registers implicitly occupied by actual args at the point of
    // call.  To be added to the call as a regs_used note for use by DFA.
    // The set is filled in by the loop below.
    NatSetDense regs_used;

    // If we ever generate a hold list, we would dump its contents now.
    // The single exception is held load-struct operations (which need
    // to be adjacent to the corresponding store-struct operations).
    // We need to dump the held instructions because it is possible
    // that one of these instructions could get expanded into a set of
    // instructions *including* a call, e.g. as happens with a divide
    // instruction.  Having an instruction expand into a call in the
    // middle of another calls argument build is a bad, bad idea.
    //
    // ... no hold list so no code here ...

    // Plow through the parameters, creating argument build
    // instructions (or information) as we go.
    for (argi = 1; argi <= num_args; argi++) {
	Opnd arg_opnd = get_src(cal, argi);
	TypeId arg_type = get_type(arg_opnd);
	claim(!is_addr(arg_opnd), "Call argument can't be an address operand");

	// Perform offset and argument area size calculations -- arg_area
	// should always remain aligned on a 64-bit boundary.
	claim((arg_area & 7) == 0);		// verify word aligned
	int arg_size = get_bit_size(arg_type) >> 3;
	int arg_offset = arg_area;
	arg_area += arg_size;

	// Pad out arg_area to 64-bit boundary
	int bytes_over_boundary;
	if ((bytes_over_boundary = (arg_area & 7)))
	    arg_area += (8 - bytes_over_boundary);

	// Parameter build instructions -- 4 possible scenarios
	//  (1)	structure (or union) operand;
	//  (2) FP operand and parameter registers available;
	//  (3) integer operand and parameter registers available;
	//  (4) no parameter registers available.

	// case 1 -- struct move with/without parameter registers
	if (is_record(arg_type)) {
	    // find or generate corresponding structure load
	    Instr *mi_ld = NULL;

	    for (InstrHandle h = struct_lds.begin(); h != struct_lds.end(); ++h)
		if (get_dst(*h) == arg_opnd) {
		    mi_ld = *h;
		    struct_lds.erase(h);
		    break;
		}

	    if (mi_ld == NULL) {
		// need to create structure load
		VarSym *v = get_var(arg_opnd);
		arg_opnd = opnd_reg(get_type(arg_opnd));
		mi_ld = new_instr_alm(arg_opnd, suifvm::LOD, opnd_addr_sym(v));
	    }

	    // generate the struct store into parameter space
	    Opnd argo_opnd = opnd_immed(arg_offset, type_s64);
	    Instr *mi_st = new_instr_alm(BaseDispOpnd(opnd_reg_sp, argo_opnd),
					 suifvm::STR, arg_opnd);

	    if ((next_freeAR_gpr - ARG0_GPR) >= num_arg_regs)
		expand_struct_move(mi_ld, mi_st);
	    else {
		// Struct move requiring parameter registers.  Here we
		// don't use the helper (expand_struct_move) used by
		// translate_str and translate_memcpy, but do our own
		// version of it.  Too hard to redefine the structure move.
		int delta = expand_struct_move2(mi_ld, mi_st,
						next_freeAR_gpr, &regs_used);
		next_freeAR_fpr += delta;
		next_freeAR_gpr += delta;
	    }

	// case 2
	} else if (is_floating_point(arg_type)
	&& ((next_freeAR_fpr - ARG0_FPR) < num_arg_regs)) {
	    // move parameter value into FP parameter register
	    Opnd areg_opnd = opnd_reg(next_freeAR_fpr, type_f64);
	    mi = new_instr_alm(areg_opnd, FMOV, arg_opnd);
	    emit(mi);

	    // update regs_used set for annotation
	    regs_used.insert(next_freeAR_fpr);

	    // update next_freeAR
	    next_freeAR_fpr += 1;
	    next_freeAR_gpr += 1;

	// case 3
	} else if (!is_floating_point(arg_type)
	&& ((next_freeAR_gpr - ARG0_GPR) < num_arg_regs)) {
	    // move parameter value into GP parameter register
	    Opnd areg_opnd = opnd_reg(next_freeAR_gpr, type_s64);
	    mi = new_instr_alm(areg_opnd, alpha::MOV, arg_opnd);
	    emit(mi);

	    // update regs_used set for annotation
	    regs_used.insert(next_freeAR_gpr);

	    // update next_freeAR
	    next_freeAR_fpr += 1;
	    next_freeAR_gpr += 1;

	} else {		// case 4 -- put parameter on stack
	    int s_opcode = opcode_store(arg_type);
	    claim((next_freeAR_gpr - ARG0_GPR) >= num_arg_regs);
	    Opnd argo_opnd = opnd_immed(arg_offset, type_s64);
	    mi = new_instr_alm(BaseDispOpnd(opnd_reg_sp, argo_opnd, arg_type),
			       s_opcode, arg_opnd);
	    emit(mi);
	}
    }

    // Generate the actual call instruction -- call through calculated
    // register.  We need callee address to be in $27 so $gp can be setup
    // in called proc.  This can either be the address of target symbol of
    // the call or the value of the first source.  But one of those two
    // must be null.
    //
    // We make the dst of the jsr ($ra) explicit to help out with
    // instruction scheduling.

    Sym *target = get_target(cal);
    claim((target == NULL) || (srcs_size(cal) > 0 && is_null(get_src(cal, 0))),
	  "Ambiguous called procedure");

    if (target == NULL)
	mi = new_instr_alm(opnd_reg_pv, alpha::MOV, get_src(cal, 0));
    else {
	mi = new_instr_alm(opnd_reg_pv, alpha::LDA, opnd_addr_sym(target));
	maybe_use_reg_gp(mi);			// add regs_used note for $gp
    }
    emit(mi);
    mi = new_instr_cti(opnd_reg_ra, JSR, NULL, opnd_reg_pv);

    copy_notes(cal, mi);
    set_note(mi, k_regs_used, NatSetNote(&regs_used));
    set_note(mi, k_call_target, OneNote<IrObject*>(target));
    emit(mi);

    // Generate re-load of $gp
    VarSym* gp_home = lookup_local_var(k_gp_home);
    if (gp_home == NULL)
	gp_home = new_named_var(type_ptr, k_gp_home);
    Instr *mj = new_instr_alm(opnd_reg_gp, LDQ, opnd_addr_sym(gp_home));
    emit(mj);				// ldq $gp,__gp_home__


    NatSetDense regs_defd = regs_killed; // for k_regs_defd annotation

    // Generate copy of return value, if necessary
    if (dsts_size(cal)) {
	Opnd d = get_dst(cal);
	TypeId d_type = get_type(d);
	claim(!is_record(d_type));	// should'a been fixed in gen

	int opcode, reg;
	if (is_floating_point(d_type)) {
	    opcode = FMOV;
	    reg = RET0_FPR;
	} else {
	    opcode = alpha::MOV;
	    reg = RET_GPR;
	}
	emit(new_instr_alm(d, opcode, opnd_reg(reg, d_type)));

	if (is_floating_point(d_type))
	    regs_defd.insert(RET0_FPR);
	else
	    regs_defd.insert(RET_GPR);
    }
    set_note(mi, k_regs_defd, NatSetNote(&regs_defd));

    // Update per-procedure variables
    claim((arg_area & 7) == 0);		// better be aligned
    is_leaf = false;
    if (max_arg_area < arg_area)
	max_arg_area = arg_area;
    delete cal;
}


/*
 * PROCEDURE:	Translate_return()
 *
 * DESCRIPTION:
 *
 * Translates RET suifvm instructions.  The finishing pass does the
 * actual appending of instructions that restore registers.  The printing
 * routines insert the hint bits.
 */
void
CodeGenAlpha::translate_ret(Instr *ret)
{
    TypeId r_type = NULL;

    // Translate RET, if return value then we need to get it in
    // correct hard return register.
    if (srcs_size(ret) > 0) {
	Opnd r_opnd = get_src(ret, 0);
	r_type = get_type(r_opnd);

	claim(srcs_size(ret) == 1);

	if (is_floating_point(r_type))
	    emit(new_instr_alm(opnd_reg(RET0_FPR, r_type), FMOV, r_opnd));
	else if (is_void(r_type))
	    r_type = NULL;
	else
	    emit(new_instr_alm(opnd_reg(RET_GPR, r_type), alpha::MOV, r_opnd));
    }

    // make return into an Alpha return
    set_opcode(ret, alpha::RET);
    set_src(ret, 0, opnd_reg_ra);

    // remember that this is a return (and not a switch or something else)
    ListNote<IrObject*> note;
    if (r_type)
	note.set_value(0, r_type);
    set_note(ret, k_instr_ret, note);

    // If a value is returned, note the register used as being live.
    if (r_type) {
	NatSetDense regs_used;
	if (is_floating_point(r_type))
	    regs_used.insert(RET0_FPR);
	else
	    regs_used.insert(RET_GPR);
	set_note(ret, k_regs_used, NatSetNote(&regs_used));
    }

    // remember that we need to restore the saved regs and update the
    // stack pointer during the finishing pass
    set_note(ret, k_incomplete_proc_exit, note_flag());

    emit(ret);				// ret $ra
}

/* Ugly code required to move part or all of a parameter structure
 * using the parameter registers.  This routine returns the number
 * of argument registers consumed. */
int
CodeGenAlpha::expand_struct_move2(Instr *lr, Instr *sr, int next_freeAR_gpr,
				  NatSet *regs_used)
{
    int first_AR = next_freeAR_gpr;

    // determine structure alignment
    TypeId rec_type = get_type(get_dst(lr));
    int rec_size = get_bit_size(rec_type) >> 3;	// in bytes
    int struct_alignment = get_bit_alignment(rec_type);

    if (struct_alignment > 64)
	struct_alignment = 64;
    else if (struct_alignment == 0)
	struct_alignment = 8;

    TypeId mv_type;
    int sload_op, sstore_op, increment_value;
    switch (struct_alignment) {
      case 64:
	sload_op = LDQ;
	sstore_op = STQ;
	increment_value = 8;	// in bytes
	mv_type = type_s64;
	break;
      case 32:
	sload_op = LDL;
	sstore_op = STL;
	increment_value = 4;
	mv_type = type_s32;
	break;
      case 16:
	sload_op = LDW;
	sstore_op = STW;
	increment_value = 2;
	mv_type = type_s16;
	break;
      case 8:
	sload_op = LDB;
	sstore_op = STB;
	increment_value = 1;
	mv_type = type_s8;
	break;
      default:
	claim(false);
    }

    // create placeholder for structure move annotations
    Instr *mi = new_instr_alm(opcode_null);

    ListNote<IdString> begin_note;
    begin_note.append(IdString("BEGIN structure move"));
    set_note(mi, k_comment, begin_note);

    copy_notes(lr, mi);
    copy_notes(sr, mi);
    emit(mi);

    // turn structure load into lda for structure load base register
    Opnd vrldb_opnd = opnd_reg(type_addr());
    set_opcode(lr, alpha::LDA);
    set_dst(lr, vrldb_opnd);
    maybe_use_reg_gp(lr);	// add regs_used note for $gp if needed
    emit(lr);

    // load into parameter registers while possible
    int r_i = 0;		// record index in bytes
    int num_arg_regs = LAST_ARG_GPR - ARG0_GPR + 1;
    while (((next_freeAR_gpr - ARG0_GPR) < num_arg_regs)
	   && (r_i < rec_size)) {
	Opnd areg_opnd = opnd_reg(next_freeAR_gpr, type_s64);

	if (struct_alignment == 64) {
	    // easy load
	    Opnd ae =
		BaseDispOpnd(vrldb_opnd, opnd_immed(r_i, type_s64), type_u64);
	    mi = new_instr_alm(areg_opnd, LDQ, ae);
	    emit(mi);

	} else {
	    // gotta love potentially unaligned loads
	    Opnd xvr_opnd = opnd_reg(type_s64);	// extra vr
	    Opnd ae =
		BaseDispOpnd(vrldb_opnd, opnd_immed(r_i, type_s64), type_u64);
	    mi = new_instr_alm(areg_opnd, LDQ_U, ae);
	    emit(mi);
	    ae =
		BaseDispOpnd(vrldb_opnd, opnd_immed(r_i+7, type_s64), type_u64);
	    mi = new_instr_alm(xvr_opnd, LDQ_U, ae);
	    emit(mi);
	    mi = new_instr_alm(areg_opnd, EXTQL, areg_opnd, vrldb_opnd);
	    emit(mi);
	    mi = new_instr_alm(xvr_opnd, EXTQH, xvr_opnd, vrldb_opnd);
	    emit(mi);
	    mi = new_instr_alm(areg_opnd, BIS, xvr_opnd, areg_opnd);
	    emit(mi);
	}

	// update regs_used set for annotation
	regs_used->insert(next_freeAR_gpr);

	// update indices
	next_freeAR_gpr += 1;		// update next_freeAR
	r_i += 8;			// increment in bytes
    }

    // finished with parameter register loads.  Do we have any
    // more structure to move?  If so, dump it onto the stack.
    if (r_i < rec_size) {
	// create base register for stores
	Opnd vrstb_opnd = opnd_reg(type_addr());
	mi = new_instr_alm(vrstb_opnd, alpha::LDA,
			   BaseDispOpnd(opnd_reg_sp, opnd_immed_0_u64));
	emit(mi);

	// perform load followed by store, repeatedly
	int s_i = 0;
	while (r_i < rec_size) {
	    Opnd vr_opnd = opnd_reg(mv_type);

	    // load
	    Opnd ae =
		BaseDispOpnd(vrldb_opnd, opnd_immed(r_i, type_s64), mv_type);
	    mi = new_instr_alm(vr_opnd, sload_op, ae);
	    emit(mi);

	    // store
	    ae = BaseDispOpnd(vrstb_opnd, opnd_immed(s_i, type_s64), mv_type);
	    mi = new_instr_alm(ae, sstore_op, vr_opnd);
	    emit(mi);

	    // update indices
	    r_i += increment_value;
	    s_i += increment_value;
	}
    }

    // mark end of the structure move
    mi = new_instr_alm(opcode_null);

    ListNote<IdString> end_note;
    end_note.append(IdString("END structure move"));
    set_note(mi, k_comment, end_note);

    emit(mi);

    return next_freeAR_gpr - first_AR;
}
